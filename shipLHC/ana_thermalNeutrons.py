#!/usr/bin/env python 
import ROOT
from decorators import *

import os
import shipunit as u
from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument("-r", "--run",                dest="run",       type=int,  help="production sequence number", default=0)
parser.add_argument('--material',                dest='targetMaterial',      type=str,help="target material BoronCarbide or  Boratedpolyethylene",                     default="BoronCarbide")
parser.add_argument('--thick',                       dest='targetLength',      type=float,help="target thickness",                     default=0.1)
parser.add_argument('-c', '--command',    dest='command',              help="command",default="none")
parser.add_argument("-f",        dest="inputFile",       help="Input file", required=False, default=False)
parser.add_argument("-g",       dest="geoFile",       help="geo file", required=False, default="geofile-thermNeutron_Boratedpolyethylene_coldbox.root")

options = parser.parse_args()

h={}
import rootUtils as ut
def count(hFile):
   f=ROOT.TFile(hFile)
   ut.bookHist(h,'E',';log10(Ekin [MeV])',1000,-12.,4.)
   ut.bookHist(h,'Epassed',';log10(Ekin [MeV])',1000,-12.,4.)
   ut.bookHist(h,'Esecondary',';log10(Ekin [MeV])',1000,-12.,4.)
   ut.bookHist(h,'electrons',';log10(Ekin [MeV])',1000,-12.,4.,100,-0.5,99.5)
   ut.bookHist(h,'photons',';log10(Ekin [MeV])',1000,-12.,4.,100,-0.5,99.5)
   ut.bookHist(h,'Lab','  ; logE[MeV];absorption Length  [cm]',1000,-12.,4.,100000,-0.1,100.)
   ut.bookHist(h,'Labz','; logE[MeV];absorption Length  [cm]',1000,-12.,4.,100000,-0.1,100.)
   ut.bookHist(h,'xyz','',   100,-0.1,0.1,100,-0.1,0.1,200,-1.,1.)
   ut.bookHist(h,'dxyz','',100,-0.1,0.1,100,-0.1,0.1,200,-1.,1.)
   Npassed = 0
   for sTree in f.cbmsim:
       N = sTree.MCTrack[0]
       Ekin = N.GetP()**2/(2*N.GetMass())*1000.
       logEkin = ROOT.TMath.Log10(Ekin)
       rc = h['E'].Fill(logEkin)
       Nelectrons = {11:0,22:0}
       for m in sTree.MCTrack:
# only count electrons if produced in first cm, don't want neutrons to thermalize
          if m.GetStartZ()>1: continue
          for x in Nelectrons:
              if abs(m.GetPdgCode())==x: Nelectrons[x]+=1
       rc = h['electrons'].Fill(logEkin,Nelectrons[11])
       rc = h['photons'].Fill(logEkin,Nelectrons[22])
       for p in sTree.vetoPoint:
          if not p.PdgCode()==2112:  continue
          if not p.GetTrackID()==0:  rc = h['Esecondary'].Fill(ROOT.TMath.Log10(Ekin))
          if p.GetDetectorID()==1 :
             mean = ROOT.TVector3(p.GetX(),p.GetY(),p.GetZ())
             end = p.LastPoint()
             D = 2*(end-mean)
             start = 2*mean-end
             rc = h['Lab'].Fill(logEkin,D.Mag())
             rc = h['Labz'].Fill(logEkin,D.Z())
             rc = h['xyz'].Fill(start.X(),start.Y(),start.Z())
             rc = h['dxyz'].Fill(end.X(),end.Y(),end.Z())
          else:
               Npassed+=1
               rc = h['Epassed'].Fill(ROOT.TMath.Log10(Ekin))
   h['Eff'] = ROOT.TEfficiency(h['Epassed'],h['E'])
   h['Eff'].Draw()
   print(Npassed)

def beamGas():
   f=open("/mnt/hgfs/microDisk/CERNBOX/SND@LHC/ThermalNeutrons/neutronsTI18.dat")
   variables = "Eleft:Eright:N:sig"
   fntuple = ROOT.TFile.Open('neutronsTI18.root', 'RECREATE')
   nt  = ROOT.TNtuple("nt","neutronGas",variables)
   for l in f.readlines():
          if not l.find('#')<0: continue
          p = []
          for x in l.split(' '):
             if x == '' or x == '\n': continue
             p.append(float(x))
          nt.Fill(p[0]*1000.,p[1]*1000.,p[2]/1000.*10170.,p[3])  # / 1000 for MeV
   # figure 12 of technical proposal
   fntuple.cd()
   nt.Write()
   nt.Draw('log10(N*Eleft):log10(Eleft)','','box')

def myPrint(tc,tname):
   for z in ['.png','.pdf','.root']:
      tc.Print(tname+z)

materials = {'Boratedpolyethylene':ROOT.kGreen,'BoronCarbide':ROOT.kBlue,'Concrete':ROOT.kGray,'EmulsionAg109':ROOT.kOrange}

def absorptionLength(plotOnly=True):
    materials = {'Boratedpolyethylene':ROOT.kGreen,'BoronCarbide':ROOT.kBlue,'Concrete':ROOT.kGray,'EmulsionAg109':ROOT.kOrange,'H2O':ROOT.kCyan}
    B10Parameter = {}
    B10Parameter ['Boratedpolyethylene'] = 0.01*0.94 / (10.*1.672E-24)
    B10Parameter ['BoronCarbide'] = 0.125*1.360 / (10.*1.672E-24)
    Xsec = 0.61E-24
    h['Fabsorp'] = {}
    Fabsorp = h['Fabsorp']
    for m in B10Parameter:
            Fabsorp[m] = ROOT.TF1('fabs'+m,'1./([0]/sqrt(10**x)*[1])',-10.,4.)
            Fabsorp[m].SetParameter(0,Xsec)
            Fabsorp[m].SetParameter(1,B10Parameter[m])
    if not plotOnly:
     for material in materials:
           for Erange in ['-14_-12','-12_-10','-10_-8','-8_-7','-7_-6','-6_-4','-4_-2']:
               fname = "thermNeutron_"+material+"_100.0_"+Erange+"_0.root"
               if not fname in os.listdir('.'): continue
               count(fname)
               h['Esecondary'+'_'+material+Erange] = h['Esecondary'].Clone('Esecondary'+'_'+material+Erange)
               h['electrons'+'_'+material+Erange] = h['electrons'].Clone('electrons'+'_'+material+Erange)
               h['photons'+'_'+material+Erange] = h['photons'].Clone('photons'+'_'+material+Erange)
               for x in ['Lab','Labz']:
                   hname = x+'_'+material+Erange
                   h[hname] = h[x].ProfileX(hname,1,-1,'g')
                   hsum = x+'_'+material
                   if not hsum in h:
                          h[hsum] = h[hname].Clone(hsum)
                          h[hsum].SetLineColor(materials[material])
                   else: h[hsum].Add(h[hname])
     ROOT.gROOT.cd()
     ut.writeHists(h,'thermalNeutrons-histograms.root',plusCanvas=True)
    else:
     ut.readHists(h,'thermalNeutrons-histograms.root')
# make stats about secondary neutrons:
    for material in materials:
         for Erange in ['-14_-12','-12_-10','-10_-8','-8_-7','-7_-6','-6_-4','-4_-2']:
              histo = h['Esecondary'+'_'+material+Erange]
              print("secondary neutrons for %s %s %5.2F%%"%(material,Erange,100.*histo.GetEntries()/h['Labz'+'_'+material+Erange].GetEntries()))
    for X in ['electrons','photons']:
     for material in materials:
         h[X+'_'+material]=h[X].Clone(X+'_'+material)
         for Erange in ['-14_-12','-12_-10','-10_-8','-8_-7','-7_-6','-6_-4','-4_-2']:
              h[X+'_'+material].Add(h[X+'_'+material+Erange])
         norm = h[X+'_'+material].ProjectionX('norm',1,101)
         for p in [1,2,5,10]:
             h[X+str(p)+'Percent_'+material] = h[X+'_'+material].ProjectionX(X+str(p)+'Percent_'+material,1,p)
             h[X+str(p)+'Percent_'+material].Divide(norm)
     ut.bookCanvas(h,'T'+X,'',1200,800,1,1)
     h['T'+X].cd()
     material  = 'EmulsionAg109'
     h[X+'1Percent_'+material].SetLineColor(ROOT.kRed)
     h[X+'2Percent_'+material].SetLineColor(ROOT.kOrange)
     h[X+'5Percent_'+material].SetLineColor(ROOT.kBlue)
     h[X+'10Percent_'+material].SetLineColor(ROOT.kGreen)
     h[X+'1Percent_'+material].SetTitle('')
     h[X+'1Percent_'+material].GetYaxis().SetTitle('fraction of events with < N_{'+X+'}')
     h[X+'1Percent_'+material].GetXaxis().SetRangeUser(-12.,1)
     h[X+'1Percent_'+material].SetStats(0)
     h[X+'1Percent_'+material].Draw('hist')
     h['leg'+X]=ROOT.TLegend(0.63,0.25,0.88,0.40)
     for p in [1,2,5,10]:
       h[X+str(p)+'Percent_'+material].Draw('histsame')
       rc = h['leg'+X].AddEntry(h[X+str(p)+'Percent_'+material],'N_{'+X+'}<'+str(p),'PL')
     h['leg'+X].Draw('same')
     myPrint(h['T'+X],'fracEveWith'+X)
#
    fntuple = ROOT.TFile.Open('neutronsTI18.root')
    nt=fntuple.nt
    ROOT.gROOT.cd()
    tcanv = 'TFig12'
    if tcanv in h: h.pop(tcanv)
    ut.bookCanvas(h,tcanv,'',1200,800,1,1)
   # figure 12 of technical proposal
    nt.Draw('log10(N*Eleft):log10(Eleft)>>rates','','box')
    h['rates']=ROOT.gROOT.FindObjectAny('rates').Clone('rates')

    for x in ['Lab','Labz']:
      tcanv = 'abs'+x
      if tcanv in h: h.pop(tcanv)
      ut.bookCanvas(h,tcanv,'',1200,800,1,1)
      h['abs'+x].cd()
      h['abs'+x].SetLogy()
      hsum = x+'_Concrete'
      h[hsum].GetXaxis().SetRangeUser(-12.,1)
      h[hsum].SetMaximum(30.)
      h[hsum].SetMinimum(0.004)
      h[hsum].GetXaxis().SetTitle('logE[MeV]')
      h[hsum].GetYaxis().SetTitle('absorption Length  [cm]')
      h[hsum].SetLineWidth(2)
      h[hsum].Draw('hist')
      h['leg'+x]=ROOT.TLegend(0.6,0.2,0.86,0.36)
      for material in materials:
           hsum = x+'_'+material
           h[hsum].SetStats(0)
           h[hsum].SetLineWidth(2)
           h[hsum].Draw('histsame')
           if material in Fabsorp:   Fabsorp[material].Draw('same')
           rc = h['leg'+x].AddEntry(h[hsum],material,'PL')
      h['leg'+x].Draw('same')
      myPrint(h['abs'+x],'AbsLength'+x)

# folding with neutron rate
    h['Fig12'] = ROOT.TGraph()
    h['dE'] = ROOT.TGraph()
    h['neutronRate'] = ROOT.TGraph()
    h['dangerZone']=ROOT.TGraph()
    h['dangerZone'].SetPoint(0,-5.6,0.)
    h['dangerZone'].SetPoint(1,-5.6,1.E7)
    h['dangerZone'].SetPoint(2,-5.1,1.E7)
    h['dangerZone'].SetPoint(3,-5.1,0.)
    h['dangerZone'].SetFillStyle(1001)
    h['dangerZone'].SetFillColor(ROOT.kYellow)

    n = 0
    RateIntegrated = 0
    RateIntegratedW = 0
    for nt in fntuple.nt:
           E = (nt.Eleft+nt.Eright)/2.
           dE = nt.Eright - nt.Eleft
           h['Fig12'].SetPoint(n,ROOT.TMath.Log10(E),ROOT.TMath.Log10(nt.N*E))
           h['neutronRate'].SetPoint(n,E,nt.N)
           h['dE'].SetPoint(n,E,dE)
           RateIntegrated+=nt.N
           RateIntegratedW+=nt.N*dE
           n+=1

    h['TFig12'].cd()
    h['Fig12'].Draw('same')
# 
    ut.bookHist(h,'Nr',';E [MeV];dn/dlogE [cm^{-2}y^{-1}] ',100,-12.,1.)
    h['Nr'].SetMaximum(2.E8)
    h['Nr'].SetMinimum(1.E-4)
    h['Nr'].SetStats(0)
    intRates = {}
    thick = {0.0:ROOT.kBlue,0.5:ROOT.kOrange,1:ROOT.kRed,2:ROOT.kRed+2,5:ROOT.kRed-7,10:ROOT.kGreen}
    for material in ['Boratedpolyethylene','BoronCarbide']:
        intRates[material]={}
        tcanv = 'ratesWith_'+material
        if tcanv in h: h.pop(tcanv)
        ut.bookCanvas(h,tcanv,material,1200,800,1,1)
        h[tcanv].SetLogy(1)
        h[tcanv].cd()
        h['ratesWith_'+material].cd()
        h['Nr'].Draw()
        h['dangerZone'].Draw('sameF')
        h['legthick'+material]=ROOT.TLegend(0.6,0.2,0.86,0.36)
        for d in thick:   # try different thicknesses
                intRates[material][d]=0
                absorbLength = h['Labz_'+material]
                gname = 'neutronRate_'+material+'_'+str(d)
                h[gname] = ROOT.TGraph()
                h[gname].SetLineWidth(2)
                h[gname].SetLineColor(thick[d])
                for n in range(h['Fig12'].GetN()):
                    logE = h['Fig12'].GetX()[n]
                    R = ROOT.TMath.Power(10.,h['Fig12'].GetY()[n])
                    dE = h['dE'].GetPointY(n)
                    E = ROOT.TMath.Power(10.,logE)
                    absorpt = 0
                    if d==0.0:
                        h[gname].SetPoint(n,logE,R)
                        intRates[material][d] += dE*R/E
                    else:
                       L = absorbLength.GetBinContent(absorbLength.FindBin(logE))
                       Rnew = 0
                       if L>0:
                         absorpt = ROOT.TMath.Exp(-d/L)
                         Rnew = R * absorpt
                       h[gname].SetPoint(n,logE,Rnew)
#            E = (nt.Eleft+nt.Eright)/2.     h['Fig12'].SetPoint(n,ROOT.TMath.Log10(E),ROOT.TMath.Log10(nt.N*E))
                       intRates[material][d] += dE * Rnew/E
                h[gname].Draw('same')
                rc = h['legthick'+material].AddEntry(h[gname],'thickness %3.1Fcm'%(d),'PL')
                reduction = intRates[material][d]/intRates[material][0]
                print('integrated rate with %s  d=%3.1Fcm: %6.4G   reduction factor=%6.2G'%(material,d,intRates[material][d],reduction) )
# make integrated rates:
                h['IUp-neutronRate_'+material+str(d)]       = ROOT.TGraph()
                h['IUp-neutronRateW_'+material+str(d)]       = ROOT.TGraph()
                h['IDown-neutronRate_'+material+str(d)] = ROOT.TGraph()
                up       = h['IUp-neutronRate_'+material+str(d)]
                down = h['IDown-neutronRate_'+material+str(d)]
                N = h[gname].GetN()
                S = 0
                for n in range(N):
                    logE = h[gname].GetX()[n]
                    dE     = h['dE'].GetPointY(n)
                    Rnew = h[gname].GetY()[n]
                    E = ROOT.TMath.Power(10.,logE)
                    S += dE * Rnew/E
                    up.SetPoint(n,logE,S)
# with damage function
                    W =  1 - h['electrons1Percent_EmulsionAg109'].GetBinContent(n)
                    h['IUp-neutronRateW_'+material+str(d)].SetPoint(n,logE,W*S)
                S = up.GetY()[N-1]
                for n in range(N):
                    logE = up.GetX()[n]
                    S -= up.GetY()[n]
                    down.SetPoint(n,logE,S)
#
                for Elimit in [100.,1.,0.1,0.01]: #MeV
                     g = h['IUp-neutronRate_'+material+str(d)]
                     gW = h['IUp-neutronRateW_'+material+str(d)]
                     N = g.GetN()
                     for n in range(N-1,1,-1):
                           if ROOT.TMath.Power(10,g.GetX()[n])<Elimit: break
                     reduction     = g.GetY()[n]/h['IUp-neutronRate_'+material+str(0.0)].GetY()[N-1]
                     reductionW = gW.GetY()[n]/h['IUp-neutronRate_'+material+str(0.0)].GetY()[N-1]
                     print('integrated rate  E < %5.3F with %s  d=%3.1Fcm: %6.4G   reduction factor=%6.2G  |     %6.4G   reduction factor=%6.2G'%(
                                 Elimit,material,d,g.GetY()[n],reduction,gW.GetY()[n],reductionW))

        h['legthick'+material].Draw('same')
        myPrint(h['ratesWith_'+material],'reducedRates_'+material)

def printReactions():
     for material in materials:
           for Erange in ['-14_-12','-12_-10','-10_-8','-8_-7','-7_-6','-6_-4','-4_-2']:
                 print('reactions for material ',material, 'in energy range ',Erange)
                 fname = "thermNeutron_"+material+"_100.0_"+Erange+"_0.root"
                 reactions(fname,1)
def reactions(hFile,distance=1E10):
   f=ROOT.TFile(hFile)
   elements = {1:'Hydrogen',2:'Helium',3:'Lithium',4:'Berylium',5:'Boron',7:'Nitrogen',6:'Carbon',
                             8:'Oxygen',11:'Sodium',12:'Magnesium ',14:'Silicon',15:'Phosphorus',16:'Sulfur',20:'Cadmium',
                           13:'Aluminum',26:'Iron',34:'Selenium',35:'Bromine',47:'Silver',53:"Iodine"}
   ut.bookHist(h,'E',';log10(Ekin [MeV])',1000,-12.,4.)
   stats = {}
   n=-1
   for sTree in f.cbmsim:
        n+=1
        daughters = []
        for m in sTree.MCTrack:
                if m.GetMotherId()==0: daughters.append(m)
        nphot = 0
        for d in daughters:
           if d.GetPdgCode()==22: nphot+=1
        isotopes =[]
        for d in daughters:
           if d.GetStartZ()>distance: continue
           pid = d.GetPdgCode()
           if pid> 1000000000:  isotopes.append(pid)
        if len(isotopes)==0: continue
        isotopes.sort()
        reaction=''
        for x in isotopes: reaction+=str(x)+','
        if  not reaction in stats:
                    sTree.MCTrack.Dump()
                    stats[reaction]={}
        if not nphot in stats[reaction]: stats[reaction][nphot]=0
        stats[reaction][nphot]+=1
        if n>50000:  break
   for reaction in stats:
      ilist = ''
      R=0
      for c in reaction.split(','):
         if len(c)<2: continue
         x = int(c)
         A = int((x/10)%1000)
         Z  = int((x/10000)%1000)
         isoname = "unknown"
         if Z in elements: isoname = elements[Z]
         ilist+= "  %s(%i,%i)"%(isoname,A,Z)
      for nphot in stats[reaction]: R+=stats[reaction][nphot]
      R = R/n
      print('nuclear reaction n + %s:  %5.3F%%'%(ilist,R*100))
   return stats

def absorptionLengthOLD():
 Lfun = ROOT.TF1('Lfun','[0]*(10**x)**[1]',-9,-6)
 Lfun.SetParameter(0,6.4)
 Lfun.SetParameter(1,0.98)
 hFiles = {"thermNeutron_BoronCarbide_X.XX_-E_-E_0.root":[0.08,0.3],"thermNeutron_Boratedpolyethylene_X.XX_0.root":[1.0,100.]}
 # thermNeutron_BoronCarbide_4.0_-8_-7_0.root

 Ls = {0.01:ROOT.kRed,0.1:ROOT.kOrange,0.04:ROOT.kCyan,0.4:ROOT.kBlue,4.0:ROOT.kMagenta}

 for hFile in hFiles:
    material = hFile.split('_')[1]
    ut.bookCanvas(h,'absorb'+material,'',1200,800,1,1)
    h['absorb'+material].cd()
    for L in Ls:
          l = str(L)
          if L<3:  tmp = hFile.replace("X.XX",l).replace(" _-E_-E","")
          else:     tmp = hFile.replace("X.XX",l).replace(" _-E_-E","_-8_-7")
          count(tmp)
          h['Eff_'+l]=h['Eff'].Clone('Eff_'+l)
          h['L_'+l]=ROOT.TGraph()
          h['L_'+l].SetLineColor(Ls[L])
          h['Eff'].Draw()
          g = h['Eff'].GetPaintedGraph()
          for n in range(g.GetN()):
                 logE, p = g.GetPointX(n),g.GetPointY(n)
                 if p>0:
                     absL = -L/ROOT.TMath.Log(p) 
                 else:
                     absL = 0
                 h['L_'+l].SetPoint(n,logE,absL)
    ut.bookHist(h,'L',';logE; L [cm]',100,-9.,-6.)
    h['L'].SetMaximum(hFiles[hFile][0])
    h['L'].SetStats(0)
    h['L'].Draw()
    for L in Ls:
            if L>hFiles[hFile][1]: continue
            h['L_'+str(L)].Draw('same')
            h['L_'+str(0.1)].Fit(Lfun)
    myPrint(h['absorb'+material],'absorbLength'+material)

def flukaRateIntegrated(save=False):
   fntuple = ROOT.TFile.Open('neutronsTI18.root')
   h['Fig12'] = ROOT.TGraph()
   h['neutronRate'] = ROOT.TGraph()
   h['N'] = ROOT.TGraph()
   n = 0
   for nt in fntuple.nt:
   # nt.N  = [cm/MeV]
           E    = (nt.Eleft+nt.Eright)/2.
           dE = nt.Eright - nt.Eleft
           h['Fig12'].SetPoint(n,ROOT.TMath.Log10(E),E*nt.N)
           h['neutronRate'].SetPoint(n,ROOT.TMath.Log10(E),dE*nt.N)
           h['N'].SetPoint(n,E,dE*nt.N)
           n+=1
   S= 0
   h['neutronRate<E'] = h['neutronRate'].Clone('S')
   for n in range(h['neutronRate'].GetN()-1,-1,-1):
      S+=h['neutronRate'].GetY()[n]
      h['neutronRate<E'].SetPoint(n,h['neutronRate'].GetX()[n],S)
   h['neutronRate<E%']= h['neutronRate'].Clone('S%')
   for n in range(h['neutronRate<E'].GetN()):
      h['neutronRate<E%'].SetPoint(n,h['neutronRate<E'].GetX()[n],h['neutronRate<E'].GetY()[n]/h['neutronRate<E'].GetY()[0])
   ut.bookHist(h,'Nr',';E [MeV];dn/dlogE [cm^{-2}y^{-1}] ',100,-12.,2.)
   h['Nr'].SetMaximum(1.1)
   h['Nr'].SetMinimum(0.)
   h['Nr'].SetStats(0)
   h['Nr'].SetTitle('; log10(E [MeV]();N(E>x)/total')
   h['neutronRate<E%'].SetLineWidth(2)
   h['Nr'].Draw()
   h['neutronRate<E%'].Draw('same')
   if save: ut.writeHists(h,'flukaNeutronRates')

neutronMass = 939.565379/1000.
def coldBox(plotOnly=True):
 if not plotOnly:
   f = ROOT.TFile(options.inputFile)
   ROOT.gROOT.cd()
   ut.bookHist(h,'start','start neutron;x [cm] ;y [cm] ;z [cm]',100,-200,200,100,-200,200,100,-200,200)
   ut.bookHist(h,'startR','start neutron;R',100,0,200)
   for T in ['','cold','hot']:
      ut.bookHist(h,'entry'+T,'entry neutron;x [cm] ;y [cm] ;z [cm]',100,-100,100,100,-100,100,100,-100,100)
      ut.bookHist(h,'exit'+T+'-original','enter coldbox neutron;x [cm] ;y [cm] ;z [cm]',100,-100,100,100,-100,100,100,-100,100)
      ut.bookHist(h,'exit'+T,'enter coldbox neutron;x [cm] ;y [cm] ;z [cm]',100,-100,100,100,-100,100,100,-100,100)
   ut.bookHist(h,'EkinG','log10(Ekin)',100,-13.,0.)
   ut.bookHist(h,'EkinW','log10(Ekin)',100,-13.,0.)
   ut.bookHist(h,'EkinWlin','Ekin',100,1E-9,100*1E-9)
   ut.bookHist(h,'Ekin','log10(Ekin)',100,-13.,0,100,0.,100.)
   ut.bookHist(h,'EkinF','log10(Ekin) vs distance',100,-13.,0,100,0.,100.)
   ut.bookHist(h,'EkinF-original','log10(Ekin) vs distance',100,-13.,0,100,0.,100.)
   flukaRateIntegrated()
   Nsim = f.cbmsim.GetEntries()
   for sTree in f.cbmsim:
       neutron = sTree.MCTrack[0]
       P       = ROOT.TVector3(neutron.GetPx(),neutron.GetPy(),neutron.GetPz())
       Ekin = ROOT.TMath.Sqrt(P.Mag2()+neutronMass**2) - neutronMass
       start = ROOT.TVector3(neutron.GetStartX(),neutron.GetStartY(),neutron.GetStartZ())
       W = h['Fig12'].Eval(ROOT.TMath.Log10(Ekin*1000)) / Nsim
       rc = h['start'].Fill(start.Z(),start.X(),start.Y(),W)
       rc = h['EkinW'].Fill(ROOT.TMath.Log10(Ekin),W)
       rc = h['EkinWlin'].Fill(Ekin,W)
       rc = h['EkinG'].Fill(ROOT.TMath.Log10(Ekin))
       rc = h['startR'].Fill(start.Mag(),W)
       for p in sTree.vetoPoint:
              if p.PdgCode()!=2112: continue
              lastPoint = p.LastPoint()
              mPoint = ROOT.TVector3(p.GetX(),p.GetY(),p.GetZ())  
              firstPoint = 2*mPoint-lastPoint
# check that first point is further out.
              if firstPoint.Mag()<lastPoint.Mag(): continue
              D = lastPoint-firstPoint
              firstMom    =  ROOT.TVector3(p.GetPx(),p.GetPy(),p.GetPz())  
              Ekin_entry  = ROOT.TMath.Sqrt(firstMom.Mag2()+neutronMass**2) - neutronMass
              if p.GetDetectorID()==13: # inside coldbox
                    rc = h['exit'].Fill(firstPoint.X(),firstPoint.Y(),firstPoint.Z(),W)

                    if Ekin*1E9< 10:    rc = h['exitcold-original'].Fill(firstPoint.X(),firstPoint.Y(),firstPoint.Z(),W)     # 10eV
                    else:                            rc = h['exithot-original'].Fill(firstPoint.X(),firstPoint.Y(),firstPoint.Z(),W)

                    if Ekin_entry*1E9< 10:    rc = h['exitcold'].Fill(firstPoint.X(),firstPoint.Y(),firstPoint.Z(),W)     # 10eV
                    else:                            rc = h['exithot'].Fill(firstPoint.X(),firstPoint.Y(),firstPoint.Z(),W)
                    rc = h['EkinF'].Fill(ROOT.TMath.Log10(Ekin_entry),D.Mag(),W)
                    rc = h['EkinF-original'].Fill(ROOT.TMath.Log10(Ekin),D.Mag(),W)
              if p.GetDetectorID()==1: # inside shielding
                    rc = h['Ekin'].Fill(ROOT.TMath.Log10(Ekin),D.Mag(),W)
                    rc = h['entry'].Fill(firstPoint.X(),firstPoint.Y(),firstPoint.Z(),W)
                    if Ekin*1E9< 10:    rc = h['entrycold'].Fill(firstPoint.X(),firstPoint.Y(),firstPoint.Z(),W)     # 10eV
                    else:                            rc = h['entryhot'].Fill(firstPoint.X(),firstPoint.Y(),firstPoint.Z(),W)
   ut.writeHists(h,'histos-'+options.inputFile)
   print('finished making histograms ','histos-'+options.inputFile)
 else:
   ut.readHists(h,'histos-'+options.inputFile)
   g = ROOT.TFile(options.geoFile)
   sGeo = g.FAIRGeom
   vbox = sGeo.FindVolumeFast('vbox')
   dX,dY,dZ = vbox.GetShape().GetDX(),vbox.GetShape().GetDY(),vbox.GetShape().GetDZ()
#
   ROOT.gROOT.cd()
   binning = {}
   for c in ['entry','exit']:
     binning[c]={}
     for p in ['x','y','z']:
       tmp = h[c].Project3D(p)
       for imin in range(1,tmp.GetNbinsX()+1): 
             if tmp.GetBinContent(imin)>0: break
       for imax in range(tmp.GetNbinsX(),0,-1): 
             if tmp.GetBinContent(imax)>0: break
       binning[c][p]={'min':imin,'max':imax}
#
   ytop       =  {'axis':'Y','proj':'xz','entry':binning['entry']['y']['max'],'exit':binning['exit']['y']['max'],'xdist':dZ,'ydist':dX}
   ybot       =  {'axis':'Y','proj':'xz','entry':binning['entry']['y']['min'],'exit':binning['exit']['y']['min'],'xdist':dZ,'ydist':dX}
   xLeft      =  {'axis':'X','proj':'yz','entry':binning['entry']['x']['min'],'exit':binning['exit']['x']['min'],'xdist':dZ,'ydist':dY}
   xRight   =  {'axis':'X','proj':'yz','entry':binning['entry']['x']['max'],'exit':binning['exit']['x']['max'],'xdist':dZ,'ydist':dY}
   zMin       = {'axis':'Z','proj':'yx','entry':binning['entry']['z']['min'],'exit':binning['exit']['z']['min'],'xdist':dX,'ydist':dY}
   zMax      = {'axis':'Z','proj':'yx','entry':binning['entry']['z']['min'],'exit':binning['exit']['z']['max'],'xdist':dX,'ydist':dY}

   # make projections
   projections = {'Top':ytop,'Bot':ybot,'Right':xRight,'Left':xLeft,'Front':zMax,'Back':zMin}
   h['fluences'] = {}
   for T in ['','cold','hot']:
    for Z in ['entry','exit','exit-original']:
     tmp = Z.split('-')
     c = tmp[0]
     x=''
     if len(tmp)>1: x='-'+tmp[1]
     case = c+T+x
     ut.bookCanvas(h,'t'+case,'',1200,1800,2,3)
     k=1
     for p in projections:
      h['t'+case].cd(k)
      tmp = h[case]
      axis = eval('tmp.Get'+projections[p]['axis']+'axis()')
      axis.SetRange(projections[p][c]-1,projections[p][c]+1)
      h[case+p] = h[case].Project3D(projections[p]['proj'])
      h[case+p].SetName(case+p)
      axis.SetRange(0,0)
      h[case+p].SetStats(0)
      h[case+p].SetMinimum(0)
      h[case+p].SetTitle(p)
      h[case+p].Draw('colz')
# check uniformity:
      h['X-'+case+p]=h[case+p].ProjectionX('X-'+case+p)
      h['Y-'+case+p]=h[case+p].ProjectionY('Y-'+case+p)
      k+=1
      sqcm    = projections[p]['xdist']* projections[p]['ydist']
      entries = h[case+p].GetSumOfWeights()
      X = entries/sqcm
      if X > 100: txt = "%5.1F/cm^{2}"%(X)
      else: txt = "%5.2F/cm^{2}"%(X)
      h['fluences'][case+p] =X
      L = ROOT.TLatex()
      rc = L.DrawLatexNDC(0.2,0.85,txt)
      h['X-'+case+p].Scale(1./ projections[p]['ydist'])
      h['Y-'+case+p].Scale(1./ projections[p]['xdist'])
     myPrint(h['t'+case],'t'+case)

# make cross checks
   ut.bookCanvas(h,'crosschecks','cross checks',900,600,1,1)
   tc = h['crosschecks'].cd()
   tc.SetLogy(1)
   tc.SetGridx()
   tc.SetGridy()
   h['EkinW'].SetStats(0)
   h['EkinW'].SetStats(0)
   h['EkinW'].SetLineWidth(3)
   h['EkinW'].SetTitle(';log(E) [GeV];dn/dlogE [cm^{-2}y^{-1}] ')
   h['EkinW'].SetMaximum(1E8)
   h['EkinW'].SetMinimum(1E2)
   h['EkinW'].Draw()
   myPrint(h['crosschecks'],'kinEnergy')
   tc.SetLogy(0)
   k=-10
   for p in projections:
      for l in ['X-','Y-']:
        case = l+'entry'+p
        h[case].SetLineColor(ROOT.kRed+k)
        h[case].SetStats(0)
        if k<-9: 
            h[case].SetTitle('; x,y,z  [cm]; N/L [cm^{-1}]')
            tpl = ut.findMaximumAndMinimum(h[case])
            h[case].SetMaximum(tpl[1]*1.5)
            h[case].Draw()
        else: h[case].Draw('same')
        k+=1
   myPrint(h['crosschecks'],'irradiationXYZ')
#
   tc.SetLogy(1)
   h['rej'] = h['EkinF'].ProjectionX('rej')
   h['rej'].Divide(h['Ekin'].ProjectionX('Ekinxxx'))
   h['rejo'] = h['EkinF-original'].ProjectionX('rejo')
   h['rejo'].Divide(h['Ekin'].ProjectionX('Ekinxxx'))
   h['rej'].SetStats(0)
   h['rejo'].SetStats(0)
   h['rej'].SetTitle(';log10(Ekin) GeV; rejection')
   h['rej'].SetLineColor(ROOT.kBlue)
   h['rej'].SetLineWidth(2)
   h['rejo'].SetLineWidth(2)
   h['rejo'].SetLineColor(ROOT.kGreen)
   h['rej'].GetXaxis().SetRangeUser(-13.,-1.)
   h['rej'].SetMaximum(1.2)
   h['rej'].SetMinimum(1.E-4)
   h['rej'].Draw('hist')
   h['rejo'].Draw('histsame')
   h['legR']=ROOT.TLegend(0.15,0.88,0.72,0.97)
   rc = h['legR'].AddEntry(h['rejo'],'reduction as function of original E_{kin} when entering shield','PL')
   rc = h['legR'].AddEntry(h['rej'],'reduction as function of E_{kin} when leaving shield','PL')
   h['legR'].Draw('same')
   myPrint(h['crosschecks'],'rejections')

# statistics:
   for o in ['','-original']:
    for T in ['cold','hot']:
       norm = 0
       exit = 0
       concrete = 0
       holes = 0
       for p in projections:
          if p!='Bot' and norm!='Front':
                norm+=h['fluences']['entry'+T+p]
                exit+=h['fluences']['exit'+T+o+p]
          if p=='Bot':     concrete = h['fluences']['exit'+T+o+p]
          elif p=='Front': holes = h['fluences']['exit'+T+o+p]
       norm = norm/float(4)
       exit     = exit/float(4)
#! do not need average, need total. 
# incoming = 6 * average ! 
       print('%s  %s region:  %5.2F  concrete: %5.2F    holes:   %5.2F x permille     total:   %5.2F x permille'%(o,T,exit/norm*1000,concrete/norm*1000,holes/norm*1000,(4*exit+concrete+holes)/(6*norm)*1000))

def drawNeutronGen(hname='start'):
    ROOT.gStyle.SetPalette(ROOT.kRainBow)
    pal = ROOT.TColor.GetPalette()
    geoManager = ROOT.gGeoManager
    geoManager.SetNsegments(12)
    h['material'] = ROOT.TGeoMaterial("dummy")
    h['medium'] = ROOT.TGeoMedium('dummy',1,h['material'])
    h['top'] = geoManager.GetTopVolume()
    top = h['top']
    floor=geoManager.FindVolumeFast('vfloor')
    floor.SetLineColor(ROOT.kGray)
    box = geoManager.FindVolumeFast('vbox')
    box.SetLineColor(ROOT.kBlue-9)
    top.Draw('ogl')
#
    nx,ny,nz = h[hname].GetXaxis().GetNbins(),h[hname].GetYaxis().GetNbins(),h[hname].GetZaxis().GetNbins()
    hmax = pal.GetSize()/h[hname].GetMaximum()
    h['x'] = geoManager.MakeBox('x',h['medium'] ,0.5,0.5,0.5)
    h['x'].SetTransparency(50)
    h['x'].SetLineColor(ROOT.kMagenta-6)
    h['A']  = ROOT.TGeoVolumeAssembly("A")
    n=0
    h['T'] = []
    for ix in range(1,nx+1,2):
        for iy in range(1,ny+1,2):
           for iz in range(1,nz+1,2):
                C = h[hname].GetBinContent(ix,iy,iz)
                if not C>0: continue
                n+=1
                x,y,z  = h[hname].GetXaxis().GetBinCenter(ix),h[hname].GetYaxis().GetBinCenter(iy),h[hname].GetZaxis().GetBinCenter(iz)
                # kc = int(pal.GetAt(int(C*hmax)))
                # print("color",int(C*hmax),kc)
                h['T'].append(ROOT.TGeoTranslation(x,y, z))
                h['A'].AddNode(h['x'], ix+1000*iy+1000000*iz ,h['T'][len(h['T'])-1])
                s = str(ix+1000*iy+1000000*iz)
    h['A'].AddNode(top,0)
    h['A'].Draw('ogl')

if options.command=='coldBox':
    coldBox(plotOnly=False)