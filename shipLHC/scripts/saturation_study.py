import ROOT,os,subprocess
ROOT.gStyle.SetOptStat(0)
import atexit
import time
import ctypes
import math
from array import array
import numpy as np
import pandas as pd
from multiprocessing import Pool
import matplotlib.pyplot as plt
from scipy.optimize import curve_fit
import json
import uproot

# the MC data files for testbeam 2023 are located at /eos/user/e/ekhaliko/Documents/SND_Data/test_300GeV_n100k_aug2023_pi-_new/
# the data is available for the following energies (100k events): 
# Pions (pi-): 100, 140, 180, 240, 300, 500, 750, 1000, and 1500 (48k events) GeV
# Muons (mu-): 160 GeV (100k events)

def linearFunc(x,intercept,slope):
    y = intercept + slope * x
    return y
# for fixing a root bug,  will be solved in the forthcoming 6.26 release.
ROOT.gInterpreter.Declare("""
#include "MuFilterHit.h"
#include "AbsMeasurement.h"
#include "TrackPoint.h"

void fixRoot(MuFilterHit& aHit,std::vector<int>& key,std::vector<float>& value, bool mask) {
   std::map<int,float> m = aHit.GetAllSignals(false);
   std::map<int, float>::iterator it = m.begin();
   while (it != m.end())
    {
        key.push_back(it->first);
        value.push_back(it->second);
        it++;
    }
}
void fixRootT(MuFilterHit& aHit,std::vector<int>& key,std::vector<float>& value, bool mask) {
   std::map<int,float> m = aHit.GetAllTimes(false);
   std::map<int, float>::iterator it = m.begin();
   while (it != m.end())
    {
        key.push_back(it->first);
        value.push_back(it->second);
        it++;
    }
}
void fixRoot(MuFilterHit& aHit, std::vector<TString>& key,std::vector<float>& value, bool mask) {
   std::map<TString, float> m = aHit.SumOfSignals();
   std::map<TString, float>::iterator it = m.begin();
   while (it != m.end())
    {
        key.push_back(it->first);
        value.push_back(it->second);
        it++;
    }
}

void fixRoot(std::vector<genfit::TrackPoint*>& points, std::vector<int>& d,std::vector<int>& k, bool mask) {
      for(std::size_t i = 0; i < points.size(); ++i) {
        genfit::AbsMeasurement*  m = points[i]->getRawMeasurement();
        d.push_back( m->getDetId() );
        k.push_back( int(m->getHitId()/1000) );
    }
}
""")
def pyExit():
       print("Make suicide until solution found for freezing")
       os.system('kill '+str(os.getpid()))
atexit.register(pyExit)

Tkey  = ROOT.std.vector('TString')()
Ikey   = ROOT.std.vector('int')()
Value = ROOT.std.vector('float')()

def map2Dict(aHit,T='GetAllSignals',mask=False):
     if T=="SumOfSignals":
        key = Tkey
     elif T=="GetAllSignals" or T=="GetAllTimes":
        key = Ikey
     else: 
           print('use case not known',T)
           1/0
     key.clear()
     Value.clear()
     if T=="GetAllTimes": ROOT.fixRootT(aHit,key,Value,mask)
     else:                         ROOT.fixRoot(aHit,key,Value,mask)
     theDict = {}
     for k in range(key.size()):
         if T=="SumOfSignals": theDict[key[k].Data()] = Value[k]
         else: theDict[key[k]] = Value[k]
     return theDict

import rootUtils as ut
import shipunit as u
h={}
from argparse import ArgumentParser
parser = ArgumentParser()
parser.add_argument("-r", "--runNumber", dest="runNumber", help="run number", type=int,required=True)
parser.add_argument("-p", "--path", dest="path", help="run number",required=False,default="")
parser.add_argument("-f", "--inputFile", dest="fname", help="file name for MC", type=str,default=None,required=False)
parser.add_argument("-g", "--geoFile", dest="geoFile", help="geofile", required=True)
parser.add_argument("-b", "--heartBeat", dest="heartBeat", help="heart beat", default=10000,type=int)
parser.add_argument("-c", "--command", dest="command", help="command", default="")
parser.add_argument("-n", "--nEvents", dest="nEvents", help="number of events", default=-1,type=int)
parser.add_argument("-q", "--nStart", dest="Nstart", help="first event", default=0,type=int)
parser.add_argument("-t", "--trackType", dest="trackType", help="DS or Scifi", default="DS")
parser.add_argument("--remakeScifiClusters", dest="remakeScifiClusters", help="remake ScifiClusters", default=False)
options = parser.parse_args()

# runNr   = str(options.runNumber).zfill(6)
runNr = str(options.runNumber)
# /eos/experiment/sndlhc/convertedData/commissioning/TI18/
# python new_mufi_exp.py -r 6069 -p /eos/experiment/sndlhc/convertedData/physics/2023/ -g geofile_sndlhc_TI18_V2_2023.root
# runNr   = "00" + str(options.runNumber)
print(runNr)
path     = options.path
partitions = 0
if options.runNumber > 0: 
 if path.find('eos')>0:
    path     = os.environ['EOSSHIP']+options.path
    dirlist  = str( subprocess.check_output("xrdfs "+os.environ['EOSSHIP']+" ls "+options.path,shell=True) )
# single file, before Feb'22
    data = "sndsw_raw_"+runNr+".root"
    if  dirlist.find(data)<0:
# check for partitions
       dirlist  = str( subprocess.check_output("xrdfs "+os.environ['EOSSHIP']+" ls "+options.path+"run_"+runNr,shell=True) )
       while 1>0:
        data = "raw-"+ str(partitions).zfill(4)
        if dirlist.find(data)>0:
            partitions+=1
        else: break
 else:
# check for partitions
       data = "sndsw_raw_"+runNr+".root"
       dirlist = os.listdir(options.path)
       if  not data in dirlist:
          dirlist  = os.listdir(options.path+"run_"+runNr)
          for x in dirlist:
            data = "raw-"+ str(partitions).zfill(4)
            if x.find(data)>0:
               partitions+=1

import SndlhcGeo
if (options.geoFile).find('../')<0: geo = SndlhcGeo.GeoInterface(path+options.geoFile)
else:                                         geo = SndlhcGeo.GeoInterface(options.geoFile[3:])
MuFilter = geo.modules['MuFilter']
Scifi       = geo.modules['Scifi']
nav = ROOT.gGeoManager.GetCurrentNavigator()

A,B,locA,locB,globA,globB    = ROOT.TVector3(),ROOT.TVector3(),ROOT.TVector3(),ROOT.TVector3(),ROOT.TVector3(),ROOT.TVector3()
latex = ROOT.TLatex()


# MuFilter mapping of planes and bars 
systemAndPlanes  = {1:2,2:5,3:7}
systemAndBars     = {1:7,2:10,3:60}
def systemAndOrientation(s,plane):
   if s==1 or s==2: return "horizontal"
   if plane%2==1 or plane == 6: return "vertical"
   return "horizontal"

systemAndChannels     = {1:[8,0],2:[6,2],3:[1,0]}
sdict                     = {1:'Veto',2:'US',3:'DS'}

freq      = 160.316E6
TDC2ns = 1E9/freq

# some helper functions

def linear_function(x, y):
    return a * x + b * y + c
 
def getAverageZpositions():
   zPos={'MuFilter':{},'Scifi':{}}
   for s in systemAndPlanes:
       for plane in range(systemAndPlanes[s]):
          bar = 4
          p = plane
          if s==3 and (plane%2==0 or plane==7): 
             bar = 90
             p = plane//2
          elif s==3 and plane%2==1:
             bar = 30
             p = plane//2
          MuFilter.GetPosition(s*10000+p*1000+bar,A,B)
          zPos['MuFilter'][s*10+plane] = (A.Z()+B.Z())/2.
   for s in range(1,6):
      mat   = 2
      sipm = 1
      channel = 64
      for o in range(2):
          Scifi.GetPosition(channel+1000*sipm+10000*mat+100000*o+1000000*s,A,B)
          zPos['Scifi'][s*10+o] = (A.Z()+B.Z())/2.
   return zPos

def smallSiPMchannel(i):
    if i==2 or i==5 or i==10 or i==13: return True
    else: return False

def largeSiPMchannel(i):
    return not smallSiPMchannel(i)

def fit_langau(hist,o,bmin,bmax):
   params = {0:'Width(scale)',1:'mostProbable',2:'norm',3:'sigma'}
   F = ROOT.TF1('langau',langaufun,0,200,4)
   for p in params: F.SetParName(p,params[p])
   rc = hist.Fit('landau','S'+o,'',bmin,bmax)
   res = rc.Get()
   if not res: return res
   F.SetParameter(2,res.Parameter(0))
   F.SetParameter(1,res.Parameter(1))
   F.SetParameter(0,res.Parameter(2))
   F.SetParameter(3,res.Parameter(2))
   F.SetParLimits(0,0,10)
   F.SetParLimits(1,0,100)
   F.SetParLimits(3,0,10)
   rc = hist.Fit(F,'S','',bmin,bmax)
   res = rc.Get().Parameter(1)
   return res

def twoLangaufun(x,par):
   N1 = langaufun(x,par)
   par2 = [par[0],par[1]*2,par[4],par[3]]
   N2 = langaufun(x,par2)
   return N1+abs(N2)

def  langaufun(x,par):
   #Fit parameters:
   #par[0]=Width (scale) parameter of Landau density
   #par[1]=Most Probable (MP, location) parameter of Landau density
   #par[2]=Total area (integral -inf to inf, normalization constant)
   #par[3]=Width (sigma) of convoluted Gaussian function
   #
   #In the Landau distribution (represented by the CERNLIB approximation),
   #the maximum is located at x=-0.22278298 with the location parameter=0.
   #This shift is corrected within this function, so that the actual
   #maximum is identical to the MP parameter.
#
      # Numeric constants
      invsq2pi = 0.3989422804014   # (2 pi)^(-1/2)
      mpshift  = -0.22278298       # Landau maximum location
#
      # Control constants
      np = 100.0      # number of convolution steps
      sc =   5.0      # convolution extends to +-sc Gaussian sigmas
#
      # Variables
      summe = 0.0
#
      # MP shift correction
      mpc = par[1] - mpshift * par[0]
#
      # Range of convolution integral
      xlow = x[0] - sc * par[3]
      xupp = x[0] + sc * par[3]
#
      step = (xupp-xlow) / np
#
      # Convolution integral of Landau and Gaussian by sum
      i=1.0
      if par[0]==0 or par[3]==0: return 9999
      while i<=np/2:
         i+=1
         xx = xlow + (i-.5) * step
         fland = ROOT.TMath.Landau(xx,mpc,par[0]) / par[0]
         summe += fland * ROOT.TMath.Gaus(x[0],xx,par[3])
#
         xx = xupp - (i-.5) * step
         fland = ROOT.TMath.Landau(xx,mpc,par[0]) / par[0]
         summe += fland * ROOT.TMath.Gaus(x[0],xx,par[3])
#
      return (par[2] * step * summe * invsq2pi / par[3])

def myPrint(tc,name,withRootFile=True):
     tc.Update()
     tc.Print(name+'-run'+str(options.runNumber)+'.png')
     tc.Print(name+'-run'+str(options.runNumber)+'.pdf')
     if withRootFile: tc.Print(name+'-run'+str(options.runNumber)+'.root')

def makeAnimation(histname,j0=1,j1=2,animated=True, findMinMax=True, lim = 50):
    ut.bookCanvas(h,'ani','',900,800,1,1)
    tc = h['ani'].cd()
    jmin,jmax = j0,j1
    if findMinMax:
       jmin,jmax = 0,0
       for j in range(j0,j1):
            tmp = histname.replace('$$$',str(j))
            if tmp in h:
                  if h[tmp].GetEntries()>lim:
                       jmin = j
                       print(j,tmp,h[tmp].GetEntries())
                       break
       for j in range(j1,j0,-1):
            tmp = histname.replace('$$$',str(j))
            if tmp in h:
                  if h[tmp].GetEntries()>lim:
                       jmax = j
                       break
    for j in range(jmin,jmax):
            tmp = histname.replace('$$$',str(j))
            h[tmp].Draw()
            tc.Update()
            stats = h[tmp].FindObject('stats')
            stats.SetOptFit(1111111)
            h[tmp].Draw()
            if animated: 
               h['ani'].Print('picAni'+str(j)+'.png')
            else:
               rc = input("hit return for next event or q for quit: ")
               if rc=='q': break
    if animated and jmax > jmin: 
           os.system("convert -delay 60 -loop 0 picAni*.png "+histname+".gif")
           os.system('rm picAni*.png')



# initialize 

zPos = getAverageZpositions()

if options.runNumber>0:
              eventChain = ROOT.TChain('rawConv')
              if partitions==0:
                   eventChain.Add(path+'sndsw_raw_'+str(options.runNumber).zfill(6)+'.root')
              else:
                   for p in range(partitions):
                       eventChain.Add(path+'run_'+runNr+'/sndsw_raw-'+str(p).zfill(4)+'.root')

else:
# for MC data and other files
            #   FNAME = options.fname
              f=ROOT.TFile.Open(options.fname)
              if f.Get('rawConv'):   eventChain = f.rawConv
              else:                        eventChain = f.cbmsim
if options.remakeScifiClusters: eventChain.SetBranchStatus("Cluster_Scifi*",0)
rc = eventChain.GetEvent(0)
run      = ROOT.FairRunAna()
ioman = ROOT.FairRootManager.Instance()
ioman.SetTreeName(eventChain.GetName())
outFile = ROOT.TMemFile('dummy','CREATE')
source = ROOT.FairFileSource(eventChain.GetCurrentFile())

if partitions>0:
    for p in range(1,partitions):
                       source.AddFile(path+'run_'+runNr+'/sndsw_raw-'+str(p).zfill(4)+'.root')
run.SetSource(source)
sink = ROOT.FairRootFileSink(outFile)
run.SetSink(sink)

houghTransform = False # under construction, not yet tested
if houghTransform:
   muon_reco_task = SndlhcMuonReco.MuonReco()
   muon_reco_task.SetName("houghTransform")
   run.AddTask(muon_reco_task)
else:
   import SndlhcTracking
   trackTask = SndlhcTracking.Tracking()
   trackTask.SetName('simpleTracking')
   run.AddTask(trackTask)

#avoiding some error messages
xrdb = ROOT.FairRuntimeDb.instance()
xrdb.getContainer("FairBaseParSet").setStatic()
xrdb.getContainer("FairGeoParSet").setStatic()

run.Init()
if partitions>0:  eventTree = ioman.GetInChain()
else:                 eventTree = ioman.GetInTree()

OT = ioman.GetSink().GetOutTree()
Reco_MuonTracks = trackTask.fittedTracks
Cluster_Scifi         = trackTask.clusScifi
# wait for user action 

def help():
    print(" following methods exist")
    print("     make and plot  hitmaps, signal distributions for MuFIlter and Scifi:")
    print("              Scifi_hitMaps(Nev) and Mufi_hitMaps(Nev)     if no number of events is given, loop over all events in file.")
    print(" ")
    print("     plot time between events and time since first event")
    print("              eventTime(Nev=-1)")
    print(" ")
    print("     MuFilter residuals, efficiency estimates with DS or Scifi tracks")
    print("              Mufi_Efficiency(Nev=-1,optionTrack='DS' or 'Scifi")
    print(" ")
    print("     analyze and plot historgams made withMufi_Efficiency ")
    print("              analyze_EfficiencyAndResiduals(readHists=False), with option True, histograms are read from file")
    print(" ")
    print("     Scifi unbiased residuals for an optional set of alignment constants")
    print("              Scifi_residuals(Nev=-1,NbinsRes=100,xmin=-500.,alignPar=False), xmin = low edge of residual histos in microns")
    print(" ")
    print("     Minimization of Scifi alignment constants")
    print("              minimizeAlignScifi(first=True,level=1,minuit=False)")
    print(" ")
    print("     first attempt to look at hadronic showers ")
    print("              USshower(Nev=-1)")
    print(" ")
    print("     make histograms with QDC distributions and fit all distributions with Langau ")
    print("              mips()")
    print("     plot the above distributions directly or reading from file ")
    print("              plotMips(readhisto=True)")
    print(" ")
    print("     beam illumination ")
    print("             scifi_beamSpot() and beamSpot() for MuFilter")
    print(" ")
    print("     rough estimate of Scifi resolution by comparing slopes  build with two stations, x and y projection")
    print("             Scifi_slopes(Nev=-1)")




def beamSpot():
   trackTask.ExecuteTask()
   Xbar = -10
   Ybar = -10
   for  aTrack in Reco_MuonTracks:
         state = aTrack.getFittedState()
         pos    = state.getPos()
         rc = h['bs'].Fill(pos.x(),pos.y())
         points = aTrack.getPoints()
         keys     = ROOT.std.vector('int')()
         detIDs = ROOT.std.vector('int')()
         ROOT.fixRoot(points, detIDs,keys,True)
         for k in range(keys.size()):
             #                                     m = p.getRawMeasurement()
             detID =detIDs[k] # m.getDetId()
             key = keys[k]          # m.getHitId()//1000 # for mufi
             aHit = eventTree.Digi_MuFilterHits[key]
             if aHit.GetDetectorID() != detID: continue # not a Mufi hit
             s = detID//10000
             l  = (detID%10000)//1000  # plane number
             bar = (detID%1000)
             if s>2: 
               l=2*l
               if bar>59:
                    bar=bar-60
                    if l<6: l+=1
             if s==3 and l%2==0: Ybar=bar
             if s==3 and l%2==1: Xbar=bar
             nSiPMs = aHit.GetnSiPMs()
             nSides  = aHit.GetnSides()
             for p in range(nSides):
                 c=bar*nSiPMs*nSides + p*nSiPMs
                 for i in range(nSiPMs):
                      signal = aHit.GetSignal(i+p*nSiPMs)
                      if signal > 0:
                           rc  = h['Tsig_'+str(s)+str(l)].Fill(signal)
         mom = state.getMom()
         slopeY= mom.X()/mom.Z()
         slopeX= mom.Y()/mom.Z()
         h['slopes'].Fill(slopeX,slopeY)
         if not Ybar<0 and not Xbar<0 and abs(slopeY)<0.01: rc = h['bsDS'].Fill(Xbar,Ybar)


####### original functions below this line #######

# this function introduces the SciFi cut that gets rid of events that don't lose energy in the first SciFi plane and draws signal distributions in SciFi
def scifi_cut(DigiHits, cut_type, hist_scifi_hit, hist_scifi_qdc, hist_us_scifi):
   N_plane_ZY = {i: 0 for i in range(1, 5)}
   N_plane_ZX = {i: 0 for i in range(1, 5)}
   scifi_num_of_hits = 0
   scifi_signal_sum = 0
   for scifiHit in DigiHits:
         scifi_num_of_hits += 1
         scifi_signal_sum += scifiHit.GetSignal(0)
         if scifiHit.isVertical(): 
               hist_scifi_qdc[scifiHit.GetStation() - 1].Fill(scifiHit.GetSignal(0))
               N_plane_ZX[scifiHit.GetStation()] += 1
         else:
               hist_scifi_qdc[scifiHit.GetStation() + 3].Fill(scifiHit.GetSignal(0))
               N_plane_ZY[scifiHit.GetStation()] += 1
   #print('number of scifi hits: ', scifi_num_of_hits, scifi_signal_sum)
   for plane in N_plane_ZY:
      if "scifiY_" + str(plane) not in hist_us_scifi:
         hist_us_scifi["scifiY_" + str(plane)] = []
      if N_plane_ZY[plane] > 0:
         hist_scifi_hit[plane - 1].Fill(N_plane_ZY[plane])
         hist_us_scifi["scifiY_" + str(plane)].append(N_plane_ZY[plane])

   for plane in N_plane_ZX:
      if "scifiX_" + str(plane) not in hist_us_scifi:
         hist_us_scifi["scifiX_" + str(plane)] = []
      if N_plane_ZX[plane] > 0:
         hist_scifi_hit[plane + 3].Fill(N_plane_ZX[plane])
         hist_us_scifi["scifiX_" + str(plane)].append(N_plane_ZX[plane])
   if cut_type == "pion":
      # print(N_plane_ZX)
      if N_plane_ZX[1] == 0:
         return False
      if N_plane_ZY[1] == 0:
         return False

   elif cut_type == "muon":
      if scifi_num_of_hits >= 25:
         return False
      if N_plane_ZX[1] == 0:
         return False
      if N_plane_ZY[1] == 0:
         return False
      return True


# this function finds the max bin
def find_max_bin(hist, range_min, range_max):
   max_bin = 0
   max_bin_content = 0
   for i in range(range_min, range_max+1):
      bin_content = hist.GetBinContent(i)
      if bin_content >= max_bin_content:
         max_bin_content = bin_content
         max_bin = hist.GetBinCenter(i)
      return max_bin

# this function draws energy loss distributions in individual US SiPMs 
def draw_qdc_sipm(hist_list_sipm, label, wall_num, write_sipm = False):
   mpv_sipms = []
   end_points = []
   sipmID = []
   k_sipm = 0
   with open(f"sipm_endpoints_{wall_num}wall", "w") as f:
      for pl in hist_list_sipm:
         for br in hist_list_sipm[pl]:
            c  = ROOT.TCanvas(f"{label}. Bar {pl*10 + br}", f"{label}. Bar {pl*10 + br}",0,0,1000,1000)
            c.Divide(4,4)
            for i, Si in enumerate(hist_list_sipm[pl][br]):
               c.cd(i+1)
               if write_sipm:
                  res = fit_langau(hist_list_sipm[pl][br][Si], str(Si), 0, 200)
                  sipmID.append(k_sipm)
                  mpv_sipms.append(res)
                  print(pl*10 + br, Si, k_sipm, res, file = f)
                  
               #fit_function = ROOT.TF1("fit_function_1n", "gaus", 100, 200)

               # # Fit the histogram with the Gaussian function
               #hist_list_sipm[pl][br][Si].Fit("fit_function_1n")
               #fit_parameters = fit_function.GetParameters()
               mean = 0
               sigma = 0
               n_init = 10
               last_nonzero_bin = hist_list_sipm[pl][br][Si].FindLastBinAbove(0) 
               #print([hist_list_sipm[pl][br][Si].GetBinContent(last_nonzero_bin-i) for i in range(n_init)])
               list_down = [hist_list_sipm[pl][br][Si].GetBinContent(last_nonzero_bin-i) for i in range(n_init)]
               # while sum(list_down) < 10 and n_init < 15:
               list_up = [hist_list_sipm[pl][br][Si].GetBinCenter(last_nonzero_bin-i)*hist_list_sipm[pl][br][Si].GetBinContent(last_nonzero_bin-i) for i in range(10)]
               try:
                  last_bin_value = sum(list_up)/sum(list_down)
                              # Set the range for the search
               except:
                  last_bin_value = -999
               max_bin = find_max_bin(hist_list_sipm[pl][br][Si], 100, 200)
               sipmID.append(k_sipm)
               end_points.append(last_bin_value)
               print(pl*10 + br, Si, k_sipm, last_bin_value, max_bin,  mean, sigma, file = f)
               hist_list_sipm[pl][br][Si].Draw()   
               k_sipm += 1       
            c.Write()


# this function draws energy loss distributions in individual US bars 
def draw_qdc_bar(hist_list_bar, label, fit_key = False):
   for pl in hist_list_bar:
      c  = ROOT.TCanvas(f"{label}. Plane {pl}", f"{label}. Bar {pl}",0,0,1000,1000)
      c.Divide(3,4)
      for i, br in enumerate(hist_list_bar[pl]):
         c.cd(i+1)
         if fit_key:
            fit_function = ROOT.TF1("fit_function", linear_function, -10, 100, -10, 100, 3)
         hist_list_bar[pl][br].Draw("COLZ")
      c.Write()

# this function draws energy loss distributions in individual US planes 
def draw_qdc_plane(hist_list, label):
    c  = ROOT.TCanvas(f"{label}",f"{label}",0,0,1000,1000)
    c.Divide(2,3)
    for i, plane in enumerate(hist_list):
      c.cd(i+1)
      hist_list[plane].Draw()
    c.Write()

# this function draws the "QDC vs Number of fired bars" distributions
def draw_qdc_vs_noh(h_qdc_hit, h_qdc_hit_norm):
    c  = ROOT.TCanvas("QDC vs. NOH","QDC vs. NOH",0,0,1000,1000)
    ROOT.gPad.SetLogz()
    h_qdc_hit.Draw('colz')
    c.Write()
    c  = ROOT.TCanvas("QDC/NOH vs. NOH","QDC/NOH vs. NOH",0,0,1000,1000)
    ROOT.gPad.SetLogz()
    h_qdc_hit_norm.Draw('colz')
    c.Write()

# this function draws the US shower distribution
def draw_usshower(hist_usshower):
    c  = ROOT.TCanvas("US Shower","US Shower",0,0,1000,1000)
   #  ROOT.gPad.SetLogz()
    hist_usshower.Draw('colz')
    c.Write()

# this function draws various US distributions
def draw_track_dist(File, hists):
   for hist in hists:
      c  = ROOT.TCanvas(hist.GetTitle(),hist.GetTitle(),0,0,1000,1000)
            # Set the marker color
      hist.SetMarkerSize(1.)

      # Set the error bar color
      hist.SetLineColor(ROOT.kRed)
      hist.SetLineWidth(2)
      hist.SetMarkerStyle(ROOT.kFullCircle)
      hist.Draw("PLC PMC")
      c.SetGrid(1,1)
      c.Write()

# this function draws various SciFi distributions
def draw_scifi_dist(File,hists, label):
   label = 'scifi_' + label
   c  = ROOT.TCanvas(f"{label}",f"{label}",0,0,1000,1000)
   c.Divide(4,2)
   for i, plane in enumerate(hists):
      tc = c.cd(i+1)
      tc.SetLogy(1)
      hists[plane].Draw()
   c.Write()


# this function implements the muon reconstruction
def track_reco_tool():
      optionTrack = "Scifi"
      for aTrack in Reco_MuonTracks: aTrack.Delete()
      Reco_MuonTracks.Clear()
      if optionTrack=='DS': rc = trackTask.ExecuteTask("DS")
      else                         : rc = trackTask.ExecuteTask("ScifiDS")
      if not Reco_MuonTracks.GetEntries()==1:
         return False, None
      theTrack = Reco_MuonTracks[0]
      if not theTrack.getFitStatus().isFitConverged():   # for H8 where only two planes / proj were avaiable
            return False, theTrack
      return True, theTrack
   # only take horizontal tracks
      state = theTrack.getFittedState(0)
      pos   = state.getPos()
      mom = state.getMom()
      slopeX= mom.X()/mom.Z()
      slopeY= mom.Y()/mom.Z()
      return True

from pathlib import Path


# this function implements the shower tagging mechanism
def read_good_events(run_number):
    with uproot.open("/afs/cern.ch/work/f/fmei/public/TB_showertags.root") as file:

        # Access the 'ShowerTags' tree
        tree = file["ShowerTags"]

        # Get the branches from the tree
        branch_names = tree.keys()

        # Create an empty dictionary to store branch data
        data_dict = {}

        # Access each branch and retrieve data
        for branch_name in branch_names:
            branch_data = tree[branch_name].array()
            data_dict[branch_name] = branch_data

        # Create a pandas DataFrame from the dictionary
        df = pd.DataFrame(data_dict).query(f"run_number == {run_number}")
        # print(df)
        return df
        

def extrapolate(theTrack,z_mid):
    state = theTrack.getFittedState()
    pos = state.getPos()
    mom = state.getMom()
    slope_x = mom.x()/mom.z()
    slope_y = mom.y()/mom.z()
    x=pos.x()+slope_x*(z_mid-pos.z())
    y=pos.y()+slope_y*(z_mid-pos.z())
    return x,y

def residual(theTrack, detID, MuFilter, h_xy_track):
    MuFilter.GetPosition(detID,A,B)
    Z = 0.5*(A[2]+B[2])
    Y = 0.5*(A[1]+B[1])
    X = 0.5*(A[0]+B[0])
    xEx, yEx = extrapolate(theTrack,Z)
    h_xy_track.Fill(xEx, yEx)
    resY = yEx-Y
    resX = xEx-X
    return resX, resY


def MIP_study(Nev_st = 0, Nev_en = 1, list_of_events_key = False, title = "default", label_0 = "def", conv_mech = "dir", side_bar_key = False, mc = False, muon = True, small_count = False, large_count = False, offset = 0., write_data = False):

 # veto system 2 layers with 7 bars and 8 sipm channels on both ends
 # US system 5 layers with 10 bars and 8 sipm channels on both ends
 # DS system horizontal(3) planes, 60 bars, readout on both sides, single channel
 #                         vertical(4) planes, 60 bar, readout on top, single channel
 

 bin_min = -10
 bin_max = 250
 label = "[#sqrt{QDC_L #times QDC_R}]"
 label_sipm = "[QDC]"
 hist_list = {l: ROOT.TH1I("plane" + f"_{l}", "plane" + f"_{l}; {label};", 200, bin_min, bin_max) for l in range(20, 25)}
 hist_list_bar = {l: {bar: ROOT.TH1I("plane" + f"_{l}_{bar}", "plane" + f"_{l}_{bar}; {label};", 200, bin_min, bin_max) for bar in range(10)} for l in range(20, 25)}
 hist_list_sipm = {l: {bar: {sipm: ROOT.TH1I("plane" + f"_{l}_{bar}_{sipm}", "plane" + f"_{l}_{bar}_{sipm}; {label_sipm};", 200, bin_min, bin_max) for sipm in range(16)} for bar in range(10)} for l in range(20, 25)}
 h_qdc_hit = ROOT.TH2I("qdc_vs_hit","QDC vs. Number of fired bars;Number of fired bars;QDC", 100,0,50.,100,0,1500)
 h_qdc_hit_norm = ROOT.TH2I("qdc_vs_hit_norm","#sum QDC/Ntot vs. Number of fired bars;Number of fired bars; #sum QDC/Ntot", 100,0,50.,100,0,40)
 hist_usshower = ROOT.TH2I("US Shower", "US Shower; Plane; Bar", 5, -0.5, 4.5, 10, -0.5, 9.5)
 hist_usshower_fraction = ROOT.TH2I("US_Shower_fraction", "US Shower; Plane; Bar", 5, -0.5, 4.5, 10, -0.5, 9.5)
 l_vs_s_hist = {l: {bar: ROOT.TH2I(f"l_vs_s_{l}_{bar}", f"Large_vs_small. Plane_{l}_{bar}; Average signal of Lsipm/hit [QDC]; Average signal of Ssipm/hit [QDC]", 200, -10, 200, 200, -10, 50) for bar in range(10)} for l in range(20, 25)}
 l_vs_s_hist_left = {l: {bar: ROOT.TH2I(f"l_vs_s_{l}_{bar}", f"Large_vs_small. Left. Plane_{l}_{bar}; Average signal of Lsipm/hit [QDC]; Average signal of Ssipm/hit [QDC]", 200, -10, 200, 200, -10, 50) for bar in range(10)} for l in range(20, 25)}
 l_vs_s_hist_right = {l: {bar: ROOT.TH2I(f"l_vs_s_{l}_{bar}", f"Large_vs_small. Right. Plane_{l}_{bar}; Average signal of Lsipm/hit [QDC]; Average signal of Ssipm/hit [QDC]", 200, -10, 200, 200, -10, 50) for bar in range(10)} for l in range(20, 25)}
 l_vs_r_hist ={l: {bar: ROOT.TH2I(f"l_vs_r_{l}_{bar}", f"Left_vs_right. Plane_{l}_{bar}; Average signal of Left sipm/hit [QDC]; Average signal of Right sipm/hit [QDC]", 200, -10, 200, 200, -10, 200) for bar in range(10)} for l in range(20, 25)}
 l_vs_r_hist_large ={l: {bar: ROOT.TH2I(f"l_vs_r_{l}_{bar}_large", f"Left_vs_right. Large. Plane_{l}_{bar}; Average signal of Right sipm/hit [QDC]; Average signal of Left sipm/hit [QDC]", 200, -10, 200, 200, -10, 200) for bar in range(10)} for l in range(20, 25)}
 l_vs_r_hist_small ={l: {bar: ROOT.TH2I(f"l_vs_r_{l}_{bar}_small", f"Left_vs_right. Small. Plane_{l}_{bar}; Average signal of Left sipm/hit [QDC]; Average signal of Right sipm/hit [QDC]", 200, -10, 200, 200, -10, 200) for bar in range(10)} for l in range(20, 25)}
 h_xy_track =  ROOT.TH2I("track_xy","track_xy;x;y", 100,-100,100.,100,-100,100.)
 h_xy_track_res =  ROOT.TH2I("track_xy_res","track_xy_res;x;y", 100,-100,100.,100,-100,100.)
 hist_scifi_hit = {l: ROOT.TH1I(f"scifi_plane_{l}", f"scifi_plane_{'vertical' if l < 4 else 'horizontal'} {l+1 if l < 4 else l - 3}; Number of hits;", 100, 0, 750) for l in range(8)}
 hist_scifi_qdc = {l: ROOT.TH1I(f"scifi_plane_{l}", f"scifi_plane_{'vertical' if l < 4 else 'horizontal'} {l+1 if l < 4 else l - 3}; QDC;", 200, 0, 100) for l in range(8)}
 h_long = ROOT.TH1I("long_shower", f"long_shower; plane;{label}", 5, -0.5, 4.5,)
 h_small_sipm = ROOT.TH1I("small_signals", f"Small_sipms signals; Signal;", 200, -10, 30)
 h_signal_per_event = ROOT.TH1I("signal_per_event", f"Large SiPM signal per event; QDC per event;", 200, -10, 2000)
 event_freq_label = "Signal_small" if small_count else "Signal"
 event_freq = ROOT.TH1I("event_freq", f"Hit number. {event_freq_label} > 0; Event number [k]; Number of hits", Nev_en // 10000, 0., (Nev_en // 10000.)*10)
#  h_long_us_scifi = ROOT.TH1I("long_shower_us_scifi", f"long_shower; plane;{label}", 5, -0.5, 4.5,)
 l_vs_r_hist_large_all =  ROOT.TH2I("LRALL","LR;L;R", 100,0,200.,100,0,200.)
 N=-1
 if Nev_en < 0 : Nev_en = eventTree.GetEntries()
#  eventTree.GetEvent(0)

 sss = 0
 wall_num = -999
 if list_of_events_key:
   wall_num = 1
   #df_events = pd.read_csv(f"/eos/user/e/ekhaliko/Documents/shower_tagging/merged_100_100k.csv")
   #df_events = pd.read_csv(f"/eos/user/e/ekhaliko/Documents/shower_tagging/merged_180_100k.csv")
   df_events = pd.read_csv(f"/eos/user/e/ekhaliko/Documents/shower_tagging/merged_300_100k.csv")
   if not mc:
      df_events = read_good_events(options.runNumber)
   print(df_events)
   event_list = df_events.query(f"wall == {wall_num}")["event_number"].to_numpy()
 else:
   event_list = range(Nev_st, Nev_en)

 us_scifi = {}
 us_qdc_list = []
 label = ""
 negative_signal = 0
 calib = 1.0
 non_saturated_bars = [0, 1, 2, 3, 6, 7, 8, 9,]
 if muon:
     saturation = False
     optionTrack = "Scifi"
     track_reco = True
     time_cut = True
     sci_fi_cut = True
 else: 
     saturation = False
     optionTrack = "ScifiDS"
     track_reco = False
     time_cut = True
     sci_fi_cut = False
 if mc:
   saturation = True
   track_reco = False
   time_cut = False
   offset = 0.
   sci_fi_cut = False

 sipm_cut = "large_11"
 sipm_cut_type, num_of_sipms = sipm_cut.split("_") 
 sqrt_on = False

 if sci_fi_cut:
    label += f"_{sci_fi_cut}_"
 if track_reco:
  label += "_with-reco_check-track-pos_slope_res-cut-strict"
 label += f"_{optionTrack}"
 label+= f"_{sipm_cut_type}-sipm-cut-{num_of_sipms}"
 if time_cut:
  label+= "_time-cut"
 label+= f"_{conv_mech}"
 label+= f"_sqrt-{sqrt_on}"
 label+= f"_{side_bar_key}"
 
 if write_data:
    fout = open(str(options.runNumber), "w")

 for N in map(int, event_list): 
    eventTree.GetEvent(N)
    sss += 1
    if sss%5000 == 0: print('event ',sss,N,' ',time.ctime())

    if sci_fi_cut:
      if not scifi_cut(eventTree.Digi_ScifiHits, "pion" if not muon else "muon", hist_scifi_hit, hist_scifi_qdc, us_scifi): continue
   #  if not ana.OneHitPerUS(eventTree.Digi_MuFilterHits): continue

    if muon:   
      if track_reco:
         track_des, theTrack = track_reco_tool()
         if not track_des:
            continue
      
    else:
      if track_reco:
         track_des, theTrack = track_reco_tool()
         if track_des:
            continue
      # if pos[0] > -37. or pos[0] < -40.:
      #    continue
      # # h_slope.Fill(slopeX, slopeY)
      # if abs(slopeX)>0.25: continue   # 4cm distance, 250mrad = 1cm
      # if abs(slopeY)>0.1: continue
      
      # h_xy_track_slope_abs.Fill(mom.x()/mom.Mag(),mom.y()/mom.Mag())

    number_of_hits = 0
    qdc_per_event = 0

    all_signal_num = {}
    negative_signal_num = {}
    us_shower = {}
    for hit_number, aHit in enumerate(eventTree.Digi_MuFilterHits):
      #   if not aHit.isValid(): continue
        detID = aHit.GetDetectorID()

        s = detID//10000 # det number (s = 1 -- veto, s = 2 -- US, s = 3 -- DS)
        l  = (detID%10000)//1000  # plane number
        bar = (detID%1000)

        if s != 2 : continue
        if muon:
         if track_reco:
            
            resx, resy = residual(theTrack, detID, MuFilter, h_xy_track)
            h_xy_track_res.Fill(resx, resy)
            #res = np.sqrt(resx**2 + resy**2)
            res = np.abs(resy)
            # cut based on residuals
            if res >= 2.95:
               continue
        else:
         if track_reco and track_des:   
            resx, resy = residual(theTrack, detID, MuFilter, h_xy_track)
            h_xy_track_res.Fill(resx, resy)
            #res = np.sqrt(resx**2 + resy**2)
            res = np.abs(resy)
            # cut based on residuals
            if res < 2.95:
               continue           

      #   timing cut - 4 clock cycles or less
        if time_cut:
           if aHit.GetTime() >= 4. or aHit.GetTime() <= 0.:
              continue

        nSiPMs = aHit.GetnSiPMs()
        nSides  = aHit.GetnSides()

        if side_bar_key: # this chooses signals in non-saturated bars only
         if bar not in non_saturated_bars: continue

        if conv_mech == "dir":   # this chooses a method of reading data
          allChannels = {i: aHit.GetSignal(i) + offset if smallSiPMchannel(i) else aHit.GetSignal(i) for i in range(16)}
          #allChannels = {i: aHit.GetSignal(i) + offset for i in range(16)}
        else:
          allChannels = map2Dict(aHit,'GetAllSignals')
        allTimes = {i: aHit.GetTime(i) for i in range(16)}
 
        small_list = [] #
        large_list = [] # 

        l_vs_r_list_right = [] #
        l_vs_r_list_left = [] #
        l_vs_r_hist_right_small_list = [] #
        l_vs_r_hist_left_small_list = [] #
        l_vs_r_hist_right_large_list = [] # 
        l_vs_r_hist_left_large_list = [] #
      
        large_firing = 0
        for Si in allChannels:
            qdc_1 = allChannels[Si]
            if qdc_1 == -999:
               continue
            if not smallSiPMchannel(Si):
               large_firing += 1
         # cut on the number of fired large SiPMs in a bar - 11 or more
        if large_firing < 11: continue

        if l not in all_signal_num:
           all_signal_num[l] = {}
           negative_signal_num[l] = {}
        if bar not in all_signal_num[l]:
           all_signal_num[l][bar] = 0
           negative_signal_num[l][bar] = 0
        for Si in allChannels:
            qdc_1 = allChannels[Si]
            if qdc_1 == -999:
               continue
               # this part introduces the saturation formula for mc data
            if saturation and not smallSiPMchannel(Si):
               qdcmax, alpha = find_parameters(l,bar,Si)
               qdc_1 = qdc_1*calib
               qdc_1 = aHit.SaturationMC(qdc_1, qdcmax, alpha)
               #qdc_1 = (1 - math.exp(-alpha*qdc_1/qdcmax))*qdcmax
            if smallSiPMchannel(Si):
               h_small_sipm.Fill(allChannels[Si])
            if small_count:
               if smallSiPMchannel(Si):
                  all_signal_num[l][bar] += 1 
                  if qdc_1 < 0:
                     negative_signal_num[l][bar] += 1
            else: 
               all_signal_num[l][bar] += 1 
               if qdc_1 < 0:
                     negative_signal_num[l][bar] += 1
            if smallSiPMchannel(Si):
               small_list.append(qdc_1)
               if Si >= 8:
                  l_vs_r_hist_right_small_list.append(qdc_1)
                  l_vs_r_list_right.append(qdc_1)
               else:
                  l_vs_r_hist_left_small_list.append(qdc_1)
                  l_vs_r_list_left.append(qdc_1)
            else:
               large_list.append(qdc_1)
               if Si >= 8:
                  l_vs_r_hist_right_large_list.append(qdc_1)
                  l_vs_r_list_right.append(qdc_1)
               else:
                  l_vs_r_hist_left_large_list.append(qdc_1)
                  l_vs_r_list_left.append(qdc_1)
            hist_list_sipm[s*10 + l][bar][Si].Fill(qdc_1)
        l_vs_s_hist[s*10 + l][bar].Fill(np.array(large_list).mean(), np.array(small_list).mean())
        l_vs_r_hist[s*10 + l][bar].Fill(np.array(l_vs_r_list_right).mean(), np.array(l_vs_r_list_left).mean())
        l_vs_r_hist_large[s*10 + l][bar].Fill(np.array(l_vs_r_hist_right_large_list).mean(), np.array(l_vs_r_hist_left_large_list).mean())
        l_vs_r_hist_small[s*10 + l][bar].Fill(np.array(l_vs_r_hist_right_small_list).mean(), np.array(l_vs_r_hist_left_small_list).mean())
        l_vs_s_hist_left[s*10 + l][bar].Fill(np.array(l_vs_r_hist_left_large_list).mean(), np.array(l_vs_r_hist_left_small_list).mean())
        l_vs_s_hist_right[s*10 + l][bar].Fill(np.array(l_vs_r_hist_right_large_list).mean(), np.array(l_vs_r_hist_right_small_list).mean())
        l_vs_r_hist_large_all.Fill(np.array(l_vs_r_hist_left_large_list).mean(), np.array(l_vs_r_hist_right_large_list).mean())
        if write_data:
                  meta_data = [N, hit_number, detID]
                  string_output = ""
                  string_output += "\t".join(list(map(str, meta_data)))
                  string_output += "\t"
                  string_output += "\t".join(list(map(str, list(allChannels.values()))))
                  string_output += "\t"
                  string_output += "\t".join(list(map(str, list(allTimes.values()))))
                  string_output += "\n"
                  fout.write(string_output)

        qdc_value = np.sqrt(np.array(l_vs_r_hist_left_large_list).mean()*np.array(l_vs_r_hist_right_large_list).mean())


        hist_list[s*10 + l].Fill(qdc_value)
        hist_list_bar[s*10 + l][bar].Fill(qdc_value)

        if l not in us_shower:
            us_shower[l] = {}
      
        if bar not in us_shower[l]:
            us_shower[l][bar] = 0

        qdc_per_event += qdc_value
        number_of_hits += 1
        us_shower[l][bar] += qdc_value
      
    if number_of_hits > 0:
      h_qdc_hit.Fill(number_of_hits, qdc_per_event)
      h_qdc_hit_norm.Fill(number_of_hits, qdc_per_event/(number_of_hits))
      us_qdc_list.append(qdc_per_event)
      h_signal_per_event.Fill(qdc_per_event)
      

    event_freq.Fill((N // 10000)*10, number_of_hits)
    for plane in us_shower:
      sum_us = sum([us_shower[plane][i] for i in us_shower[plane]])
      h_long.Fill(plane, sum([us_shower[plane][i] for i in us_shower[plane]])) 
      if "us_" + str(plane + 1) not in us_scifi:
         us_scifi["us_" + str(plane + 1)] = []
      us_scifi["us_" + str(plane + 1)].append(sum_us) 
      for bar in us_shower[plane]:
         hist_usshower.Fill(plane, bar, us_shower[plane][bar])
         if all_signal_num[plane][bar]:
            hist_usshower_fraction.Fill(plane, bar, negative_signal_num[plane][bar]/all_signal_num[plane][bar])
 
 fig, ax = plt.subplots(dpi = 200)
 ax.hist(us_qdc_list, bins = 200)
 plt.savefig(f"hist_{options.runNumber}.pdf")
 print(f"{options.runNumber} run", f": {np.array(us_qdc_list).mean()}", np.array(us_qdc_list).std())
 
 
 File = ROOT.TFile.Open(f"{'muon' if muon else 'pion'}_{title}_{options.runNumber}{label}.root", "RECREATE")
 draw_qdc_sipm(hist_list_sipm, "US QDC distribution", wall_num)
 draw_qdc_bar(hist_list_bar, "US QDC distribution")
 draw_qdc_bar(l_vs_s_hist, "Large vs Small")
 draw_qdc_bar(l_vs_s_hist_right, "Large vs Small. Right")
 draw_qdc_bar(l_vs_s_hist_left, "Large vs Small. Large")
 draw_qdc_bar(l_vs_r_hist, "Left vs Right")
 draw_qdc_bar(l_vs_r_hist_small, "Left vs Right. Small")
 draw_qdc_bar(l_vs_r_hist_large, "Left vs Right. Large")
 draw_qdc_plane(hist_list, "US QDC distribution")
 draw_qdc_vs_noh(h_qdc_hit, h_qdc_hit_norm)
 draw_usshower(hist_usshower)
 draw_usshower(hist_usshower_fraction)
 draw_track_dist(File, [l_vs_r_hist_large_all])
 draw_track_dist(File, [h_xy_track, h_xy_track_res])
 draw_track_dist(File, [h_long])
 draw_track_dist(File, [h_small_sipm])
 draw_track_dist(File, [h_signal_per_event])
 draw_track_dist(File, [event_freq])
 draw_scifi_dist(File, hist_scifi_hit, "hit_distribution")
 draw_scifi_dist(File, hist_scifi_qdc, "qdc_distribution")
 File.Close()
#  File = ROOT.TFile.Open(f"{options.runNumber}_sipm_hists.root", "RECREATE") 
#  draw_scifi_dist(File, hist_scifi_qdc, "qdc_distribution")
 print(f"Data of {options.runNumber} run has been stored in {'muon' if muon else 'pion'}_{title}_{options.runNumber}{label}.root")
 print(f"Negative singals: {negative_signal}")

def run_energy(run):
   if run == 100631 or run == 100677:
      return 100
   if run == 100633 or run == 100673:
      return 140
   if run == 100648 or run == 100646:
      return 240
   if run == 100636:
      return 180
   if run == 100643 or run == 100645:
      return 300
import pickle

def find_parameters(plane, bar, sipm):
    with open('/eos/user/e/ekhaliko/Documents/saturation_code/param', 'r') as file:
        for line in file:
            columns = line.strip().split(' ')
            if len(columns) >= 5 and int(columns[0]) == plane and int(columns[1]) == bar and int(columns[2]) == sipm:
                return float(columns[3]),float(columns[4])
    return 1.,0.
    

#label = "muon 100 GeV"
#title = "august"
label = "pion 300 GeV"
#title = "test_100k_muons"
title = "test_tagwall1_10k_300GeV_calib1.0"
MIP_study(Nev_st = options.Nstart,
   Nev_en = options.Nstart + options.nEvents, 
   list_of_events_key = True, #this key activated shower origin tagging by selecting only the events tagged in a specific wall
   title = title,
   label_0 = label,
   conv_mech = "dir", # this key chooses a method of reading data, direct or using map2Dict
   offset = 0., # this key establishes the offset value
   side_bar_key = False, # if this key is activated only the unsaturated bars are taken into account
   mc = True, # turn on this key to analyze Monte-Carlo data
   muon = False, # turn on this key to analyze muon data
   small_count = True, # this key turns on the account of small SiPMs
   large_count = True, # this key turns on the account of large SiPMs
   write_data = False) # this key writes down the full signal info