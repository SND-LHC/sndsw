import ROOT
import os,sys,subprocess,atexit
from array import array
import shipunit as u
import SndlhcMuonReco
import rootUtils as ut
from decorators import *
from rootpyPickler import Unpickler
import time
from XRootD import client

from datetime import datetime
from pathlib import Path

def pyExit():
       "unfortunately need as bypassing an issue related to use xrootd"
       os.system('kill '+str(os.getpid()))
atexit.register(pyExit)


A,B = ROOT.TVector3(),ROOT.TVector3()
freq      =  160.316E6

eventComment = {}   # possibility to add an event comment before moving to next event

h={}
from argparse import ArgumentParser
parser = ArgumentParser()
parser.add_argument("-r", "--runNumber", dest="runNumber", help="run number", type=int,required=False)
parser.add_argument("-p", "--path", dest="path", help="path to data file",required=False,default=os.environ["EOSSHIP"]+"/eos/experiment/sndlhc/convertedData/physics/2022/")
parser.add_argument("-praw", "--pathRaw", dest="pathRaw", help="path to raw data file",required=False,default="/eos/experiment/sndlhc/raw_data/physics/2022/")
parser.add_argument("-f", "--inputFile", dest="inputFile", help="input file data and MC",default="",required=False)
parser.add_argument("-g", "--geoFile", dest="geoFile", help="geofile", default=os.environ["EOSSHIP"]+"/eos/experiment/sndlhc/convertedData/physics/2022/geofile_sndlhc_TI18_V0_2022.root")
parser.add_argument("-P", "--partition", dest="partition", help="partition of data", type=int,required=False,default=-1)
parser.add_argument("--server", dest="server", help="xrootd server",default=os.environ["EOSSHIP"])
parser.add_argument("-X", dest="extraInfo", help="print extra event info",default=True)

parser.add_argument("-par", "--parFile", dest="parFile", help="parameter file", default=os.environ['SNDSW_ROOT']+"/python/TrackingParams.xml")
parser.add_argument("-hf", "--HoughSpaceFormat", dest="HspaceFormat", help="Hough space representation. Should match the 'Hough_space_format' name in parFile, use quotes", default='linearSlopeIntercept')

options = parser.parse_args()
options.storePic = ''
trans2local = False
runInfo = False
try:
   fg  = ROOT.TFile.Open(options.server+options.p+"RunInfodict.root")
   pkl = Unpickler(fg)
   runInfo = pkl.load('runInfo')
   fg.Close()
except: pass

import SndlhcGeo
geo = SndlhcGeo.GeoInterface(options.geoFile)

lsOfGlobals = ROOT.gROOT.GetListOfGlobals()
lsOfGlobals.Add(geo.modules['Scifi'])
lsOfGlobals.Add(geo.modules['MuFilter'])

detSize = {}
si = geo.snd_geo.Scifi
detSize[0] =[si.channel_width, si.channel_width, si.scifimat_z ]
mi = geo.snd_geo.MuFilter
detSize[1] =[mi.VetoBarX/2,                   mi.VetoBarY/2,            mi.VetoBarZ/2]
detSize[2] =[mi.UpstreamBarX/2,           mi.UpstreamBarY/2,    mi.UpstreamBarZ/2]
detSize[3] =[mi.DownstreamBarX_ver/2,mi.DownstreamBarY/2,mi.DownstreamBarZ/2]
withDetector = True  # False is useful when using zoom
with2Points = False  # plot start and end point of straw/bar
mc = False

firstScifi_z = 300 * u.cm
TDC2ns = 1E9 / 160.316E6

# Initialize FairLogger: set severity and verbosity
logger = ROOT.FairLogger.GetLogger()
logger.SetColoredLog(True)
logger.SetLogVerbosityLevel('low')
logger.SetLogScreenLevel('WARNING')
logger.SetLogToScreen(True)

run      = ROOT.FairRunAna()
ioman = ROOT.FairRootManager.Instance()

if options.inputFile=="":
  f=ROOT.TFile.Open(options.path+'sndsw_raw_'+str(options.runNumber).zfill(6)+'.root')
else:
  f=ROOT.TFile.Open(options.path+options.inputFile)

if f.FindKey('cbmsim'):
        eventTree = f.cbmsim
        runId = 'sim'
        if eventTree.GetBranch('ScifiPoint'): mc = True
else:   
        eventTree = f.rawConv
        ioman.SetTreeName('rawConv')

outFile = ROOT.TMemFile('dummy','CREATE')
source = ROOT.FairFileSource(f)
run.SetSource(source)
sink = ROOT.FairRootFileSink(outFile)
run.SetSink(sink)

HT_tasks = {'muon_reco_task_Sf':SndlhcMuonReco.MuonReco(),
            'muon_reco_task_DS':SndlhcMuonReco.MuonReco(),
            'muon_reco_task_nuInt':SndlhcMuonReco.MuonReco()}
for ht_task in HT_tasks.values():
    run.AddTask(ht_task)

import SndlhcTracking
trackTask = SndlhcTracking.Tracking() 
trackTask.SetName('simpleTracking')
run.AddTask(trackTask)

#avoiding some error messages
xrdb = ROOT.FairRuntimeDb.instance()
xrdb.getContainer("FairBaseParSet").setStatic()
xrdb.getContainer("FairGeoParSet").setStatic()

for ht_task in HT_tasks.values():
    ht_task.SetParFile(options.parFile)
    ht_task.SetHoughSpaceFormat(options.HspaceFormat)
    # force the output of reco task to genfit::Track
    # as the display code looks for such output
    ht_task.ForceGenfitTrackFormat()
HT_tasks['muon_reco_task_Sf'].SetTrackingCase('passing_mu_Sf')
HT_tasks['muon_reco_task_DS'].SetTrackingCase('passing_mu_DS')
HT_tasks['muon_reco_task_nuInt'].SetTrackingCase('nu_interaction_products')

run.Init()
OT = sink.GetOutTree()
eventTree = ioman.GetInTree()
eventTree.GetEvent(0)
if eventTree.EventHeader.ClassName() == 'SNDLHCEventHeader':
   geo.modules['Scifi'].InitEvent(eventTree.EventHeader)
   geo.modules['MuFilter'].InitEvent(eventTree.EventHeader)
# if faireventheader, rely on user to select correct geofile.

if eventTree.GetBranch('Digi_MuFilterHit'):
# backward compatbility for early converted events
  eventTree.GetEvent(0)
  eventTree.Digi_MuFilterHits = eventTree.Digi_MuFilterHit

nav = ROOT.gGeoManager.GetCurrentNavigator()

# get filling scheme
try:
           runNumber = eventTree.EventHeader.GetRunId()
           fg  = ROOT.TFile.Open(os.environ['EOSSHIP']+options.p+'FSdict.root')
           pkl = Unpickler(fg)
           FSdict = pkl.load('FSdict')
           fg.Close()
           if runNumber in FSdict: fsdict = FSdict[runNumber]
           else:  fsdict = False
except:
           print('continue without knowing filling scheme')
           fsdict = False

Nlimit = 4
onlyScifi = False

def goodEvent(event):
# can be replaced by any user selection
           stations = {'Scifi':{},'Mufi':{}}
           if event.Digi_ScifiHits.GetEntries()>25: return False
           for d in event.Digi_ScifiHits:
               stations['Scifi'][d.GetDetectorID()//1000000] = 1
           for d in event.Digi_MuFilterHits:
               plane = d.GetDetectorID()//1000
               stations['Mufi'][plane] = 1
           totalN = len(stations['Mufi'])+len(stations['Scifi'])
           if len(stations['Scifi'])>4 and len(stations['Mufi'])>6: return True
           else: False
           if onlyScifi and len(stations['Scifi'])>Nlimit: return True
           elif not onlyScifi  and totalN >  Nlimit: return True
           else: return False

def userProcessing(event):
    '''User hook to add action after event is plotted.

    Useful for adding special objects to the display for example.
    An example for display of 3-track events with external reco:

    ```python
    trackTask.multipleTrackCandidates(
        nMaxCl=8, dGap=0.2, dMax=0.8, dMax3=0.8, ovMax=1, doublet=True, debug=False
    )
    n3D = [0, 0]
    for p in range(2):
        tc = h['simpleDisplay'].cd(-p + 2)
        for trackId in trackTask.multipleTrackStore['trackCand'][p]:
            if trackId < 100000 and not trackTask.multipleTrackStore['doublet']:
                continue
            if trackId in trackTask.multipleTrackStore['cloneCand'][p]:
                continue
            n3D[p] += 1
            rc = trackTask.multipleTrackStore['trackCand'][p][trackId].Fit('pol1', 'SQ')
            trackTask.multipleTrackStore['trackCand'][p][trackId].Draw('same')
        tc.Update()
    print('Number of full tracks', n3D)
    return True
    ```
    '''
    return

def bunchXtype():
# check for b1,b2,IP1,IP2
        xing = {'all':True,'B1only':False,'B2noB1':False,'noBeam':False}
        if fsdict:
             T   = eventTree.EventHeader.GetEventTime()
             bunchNumber = int(T%(4*3564)/4+0.5)
             nb1 = (3564 + bunchNumber - fsdict['phaseShift1'])%3564
             nb2 = (3564 + bunchNumber - fsdict['phaseShift1']- fsdict['phaseShift2'])%3564
             b1 = nb1 in fsdict['B1']
             b2 = nb2 in fsdict['B2']
             IP1 = False
             IP2 = False
             if b1:
                IP1 =  fsdict['B1'][nb1]['IP1']
             if b2:
                IP2 =  fsdict['B2'][nb2]['IP2']
             if b2 and not b1:
                xing['B2noB1'] = True
             if b1 and not b2 and not IP1:
                xing['B1only'] = True
             if not b1 and not b2: xing['noBeam'] = True
        return xing

def loopEvents(start=0,save=False,goodEvents=False,withTrack=-1,withHoughTrack=-1,nTracks=0,minSipmMult=1,withTiming=False, option=None,Setup='',verbose=0,auto=False):
 if 'simpleDisplay' not in h: ut.bookCanvas(h,key='simpleDisplay',title='simple event display',nx=1200,ny=1600,cx=1,cy=2)
 h['simpleDisplay'].cd(1)
 zStart = 250. # TI18 coordinate system
 if Setup == 'H6': zStart = 60.
 if Setup == 'TP': zStart = -50. # old coordinate system with origin in middle of target
 if 'xz' in h: 
    h.pop('xz').Delete()
    h.pop('yz').Delete()
 else:
    h['xmin'],h['xmax'] = -100.,10.
    h['ymin'],h['ymax'] = -30.,80.
    h['zmin'],h['zmax'] = zStart,zStart+350.
    for d in ['xmin','xmax','ymin','ymax','zmin','zmax']: h['c'+d]=h[d]
 ut.bookHist(h,'xz','; z [cm]; x [cm]',500,h['czmin'],h['czmax'],100,h['cxmin'],h['cxmax'])
 ut.bookHist(h,'yz','; z [cm]; y [cm]',500,h['czmin'],h['czmax'],100,h['cymin'],h['cymax'])

 proj = {1:'xz',2:'yz'}
 h['xz'].SetStats(0)
 h['yz'].SetStats(0)

 N = -1
 Tprev = -1
 A,B = ROOT.TVector3(),ROOT.TVector3()
 ptext={0:'   Y projection',1:'   X projection'}
 text = ROOT.TLatex()
 event = eventTree
 OT = sink.GetOutTree()
 if withTrack==0 or withHoughTrack==0: OT = eventTree
 if type(start) == type(1):
    s = start
    e = event.GetEntries()
 else:
    s = 0
    e = len(start)
 for N in range(s,e):
    if type(start) == type(1): rc = event.GetEvent(N)
    else: rc = event.GetEvent(start[N])
    if goodEvents and not goodEvent(event): continue
    nHoughtracks = 0
    OT.Reco_MuonTracks = ROOT.TObjArray(10)
    if withHoughTrack > 0:
       rc = source.GetInTree().GetEvent(N)
       # Delete SndlhcMuonReco kalman tracks container
       for ht_task in HT_tasks.values():
           ht_task.kalman_tracks.Delete()
       if withHoughTrack==1:
            HT_tasks['muon_reco_task_Sf'].Exec(0)
            HT_tasks['muon_reco_task_DS'].Exec(0)
       elif withHoughTrack==2:
            HT_tasks['muon_reco_task_Sf'].Exec(0)
       elif withHoughTrack==3:
            HT_tasks['muon_reco_task_DS'].Exec(0)
       elif withHoughTrack==4:
            HT_tasks['muon_reco_task_nuInt'].Exec(0)
       # Save the tracks in OT.Reco_MuonTracks object
       for ht_task in HT_tasks.values():
           for trk in ht_task.kalman_tracks:
               OT.Reco_MuonTracks.Add(trk)
       nHoughtracks = OT.Reco_MuonTracks.GetEntries()
       if nHoughtracks>0: print('number of tracks by HT:', nHoughtracks)

    if withTrack > 0:
          # Delete SndlhcTracking fitted tracks container
          trackTask.fittedTracks.Delete()
          if withTrack==1:
              trackTask.ExecuteTask("ScifiDS")
          elif withTrack==2:
              trackTask.ExecuteTask("Scifi")
          elif withTrack==3:
              trackTask.ExecuteTask("DS")
          # Save found tracks
          for trk in trackTask.fittedTracks:
              OT.Reco_MuonTracks.Add(trk)
          ntracks = len(OT.Reco_MuonTracks) - nHoughtracks
          if ntracks>0: print('number of tracks by ST:', ntracks)
    nAlltracks = len(OT.Reco_MuonTracks)
    if nAlltracks<nTracks: continue

    if verbose>0:
       for aTrack in OT.Reco_MuonTracks:
           print(aTrack.__repr__())
           mom    = aTrack.getFittedState().getMom()
           pos      = aTrack.getFittedState().getPos()
           mom.Print()
           pos.Print()
    T,dT = 0,0
    T = event.EventHeader.GetEventTime()
    runId = eventTree.EventHeader.GetRunId()
    if Tprev >0: dT = T-Tprev
    Tprev = T
    if nAlltracks > 0: print('total number of tracks: ', nAlltracks)

    digis = []
    if event.FindBranch("Digi_ScifiHits"): digis.append(event.Digi_ScifiHits)
    if event.FindBranch("Digi_MuFilterHits"): digis.append(event.Digi_MuFilterHits)
    if event.FindBranch("Digi_MuFilterHit"): digis.append(event.Digi_MuFilterHit)
    empty = True
    for x in digis:
       if x.GetEntries()>0:
         if empty: print( "event -> %i"%N)
         empty = False
    if empty: continue
    h['hitCollectionX']= {'Scifi':[0,ROOT.TGraphErrors()],'DS':[0,ROOT.TGraphErrors()]}
    h['hitCollectionY']= {'Veto':[0,ROOT.TGraphErrors()],'Scifi':[0,ROOT.TGraphErrors()],'US':[0,ROOT.TGraphErrors()],'DS':[0,ROOT.TGraphErrors()]}
    h['firedChannelsX']= {'Scifi':[0,0,0],'DS':[0,0,0]}
    h['firedChannelsY']= {'Veto':[0,0,0,0],'Scifi':[0,0,0],'US':[0,0,0,0],'DS':[0,0,0,0]}
    systems = {1:'Veto',2:'US',3:'DS',0:'Scifi'}
    for collection in ['hitCollectionX','hitCollectionY']:
       for c in h[collection]:
          rc=h[collection][c][1].SetName(c)
          rc=h[collection][c][1].Set(0)

    dTs = "%5.2Fns"%(dT/freq*1E9)
    # find detector which triggered
    minT = firstTimeStamp(event)
    if minT[0] < 1000000000:
        dTs+= "    " + str(minT[1].GetDetectorID())
    for p in proj:
       rc = h[ 'simpleDisplay'].cd(p)
       h[proj[p]].Draw('b')

    if withDetector:
      drawDetectors()
    for D in digis:
      for digi in D:
         detID = digi.GetDetectorID()
         sipmMult = 1
         if digi.GetName()  == 'MuFilterHit':
            system = digi.GetSystem()
            geo.modules['MuFilter'].GetPosition(detID,A,B)
            sipmMult = len(digi.GetAllSignals(False,False))
            if sipmMult<minSipmMult and (system==1 or system==2): continue
         else:
            geo.modules['Scifi'].GetSiPMPosition(detID,A,B)
            system = 0
         curPath = nav.GetPath()
         tmp = curPath.rfind('/')
         nav.cd(curPath[:tmp])
         first = True
         for X in [A, B]:
             if not first and not with2Points:
                 continue
             first = False
             globA, locA = array('d', [X[0], X[1], X[2]]), array('d', [X[0], X[1], X[2]])
             if trans2local:
                 nav.MasterToLocal(globA, locA)
             Z = X[2]
             if digi.isVertical():
                   collection = 'hitCollectionX'
                   Y = locA[0]
                   sY = detSize[system][0]
             else:
                   collection = 'hitCollectionY'
                   Y = locA[1]
                   sY = detSize[system][1]
             c = h[collection][systems[system]]
             rc = c[1].SetPoint(c[0], Z, Y)
             rc = c[1].SetPointError(c[0], detSize[system][2], sY)
             c[0] += 1

         fillNode(curPath)

         if digi.isVertical():  F = 'firedChannelsX'
         else:                     F = 'firedChannelsY'
         ns = max(1,digi.GetnSides())
         for side in range(ns):
             for m in  range(digi.GetnSiPMs()):
                   qdc = digi.GetSignal(m+side*digi.GetnSiPMs())
                   if qdc < 0 and qdc > -900:  h[F][systems[system]][1]+=1
                   elif not qdc<0:   
                       h[F][systems[system]][0]+=1
                       if len(h[F][systems[system]]) < 2+side: continue
                       h[F][systems[system]][2+side]+=qdc
    h['hitCollectionY']['Scifi'][1].SetMarkerColor(ROOT.kBlue+2)
    h['hitCollectionX']['Scifi'][1].SetMarkerColor(ROOT.kBlue+2)
    k = 1
    moreEventInfo = []
    for collection in ['hitCollectionX','hitCollectionY']:
       h['simpleDisplay'].cd(k)
       drawInfo(h['simpleDisplay'], k, runId, N, T)
       k+=1
       for c in h[collection]:
          F = collection.replace('hitCollection','firedChannels')
          pj = collection.split('ion')[1]
          if pj =="X" or c=="Scifi":
              atext = "%1s %5s %3i  +:%3i -:%3i qdc :%5.1F"%(pj,c,h[collection][c][1].GetN(),h[F][c][0],h[F][c][1],h[F][c][2])
          else:
              atext = "%1s %5s %3i  +:%3i -:%3i qdcL:%5.1F qdcR:%5.1F"%(pj,c,h[collection][c][1].GetN(),h[F][c][0],h[F][c][1],h[F][c][2],h[F][c][3])
          moreEventInfo.append(atext)
          print(atext)
          if h[collection][c][1].GetN()<1: continue
          if c=='Scifi':
            h[collection][c][1].SetMarkerStyle(20)
            h[collection][c][1].SetMarkerSize(1.5)
            rc=h[collection][c][1].Draw('sameP')
            h['display:'+c]=h[collection][c][1]
    T0 = eventTree.EventHeader.GetEventTime()
    if type(start) == type(1): rc = event.GetEvent(N-1)
    else: rc = event.GetEvent(start[N]-1)
    delTM1 = eventTree.EventHeader.GetEventTime() - T0
    if type(start) == type(1): rc = event.GetEvent(N+1)
    else: rc = event.GetEvent(start[N]+1)
    delTP1 = eventTree.EventHeader.GetEventTime() - T0
    atext = "timing info, prev event: %6i cc  next event: %6i cc"%(delTM1,delTP1)
    moreEventInfo.append(atext)
    if type(start) == type(1): rc = event.GetEvent(N)
    else: rc = event.GetEvent(start[N])

    k = 1
    for collection in ['hitCollectionX','hitCollectionY']:
       h['simpleDisplay'].cd(k)
       drawInfo(h['simpleDisplay'], k, runId, N, T,moreEventInfo)
       k+=1
            
    h['simpleDisplay'].Update()
    if withTiming: timingOfEvent()
    addTrack(OT)

    if option == "2tracks": 
          rc = twoTrackEvent(sMin=10,dClMin=7,minDistance=0.5,sepDistance=0.5)
          if not rc: rc = twoTrackEvent(sMin=10,dClMin=7,minDistance=0.5,sepDistance=0.75)
          if not rc: rc = twoTrackEvent(sMin=10,dClMin=7,minDistance=0.5,sepDistance=1.0)
          if not rc: rc = twoTrackEvent(sMin=10,dClMin=7,minDistance=0.5,sepDistance=1.75)
          if not rc: rc = twoTrackEvent(sMin=10,dClMin=7,minDistance=0.5,sepDistance=2.5)
          if not rc: rc = twoTrackEvent(sMin=10,dClMin=7,minDistance=0.5,sepDistance=3.0)

    if verbose>0: dumpChannels()
    userProcessing(event)

    if save: h['simpleDisplay'].Print('{:0>2d}-event_{:04d}'.format(runId,N)+'.png')
    if auto:
        h['simpleDisplay'].Print(options.storePic+str(runId)+'-event_'+str(event.EventHeader.GetEventNumber())+'.png')
    if not auto:
       rc = input("hit return for next event or p for print or q for quit: ")
       if rc=='p': 
             h['simpleDisplay'].Print(options.storePic+str(runId)+'-event_'+str(event.EventHeader.GetEventNumber())+'.png')
       elif rc == 'q':
          break
       else:
          eventComment[f"{runId}-event_{event.EventHeader.GetEventNumber()}"] = rc
 if save: os.system("convert -delay 60 -loop 0 event*.png animated.gif")

def addTrack(OT,scifi=False):
   xax = h['xz'].GetXaxis()
   nTrack = 0
   for   aTrack in OT.Reco_MuonTracks:
      trackColor = ROOT.kRed
      if aTrack.GetUniqueID()==1:
          trackColor = ROOT.kBlue+2
          flightDir = trackTask.trackDir(aTrack)
          print('flight direction: %5.3F  significance: %5.3F'%(flightDir[0],flightDir[1]))
      if aTrack.GetUniqueID()==3: trackColor = ROOT.kBlack
      if aTrack.GetUniqueID()==11: trackColor = ROOT.kAzure-2 # HT scifi track
      if aTrack.GetUniqueID()==13: trackColor = ROOT.kGray+2 # HT ds track
      # HT cross-system track fit
      if aTrack.GetUniqueID()==15: trackColor = ROOT.kOrange+7
      S = aTrack.getFitStatus()
      if not S.isFitConverged() and (scifi or (aTrack.GetUniqueID()==1 or aTrack.GetUniqueID()==11) ):# scifi trk object ids are 1 or 11(Hough tracking)
         print('not converge')
         continue
      for p in [0,1]:
          h['aLine'+str(nTrack*10+p)] = ROOT.TGraph()

      zEx = xax.GetBinCenter(1)
      mom    = aTrack.getFittedState().getMom()
      pos      = aTrack.getFittedState().getPos()
      lam      = (zEx-pos.z())/mom.z()
      Ex        = [pos.x()+lam*mom.x(),pos.y()+lam*mom.y()]
      for p in [0,1]:   h['aLine'+str(nTrack*10+p)].SetPoint(0,zEx,Ex[p])

      for i in range(aTrack.getNumPointsWithMeasurement()):
         state = aTrack.getFittedState(i)
         pos    = state.getPos()
         for p in [0,1]:
             h['aLine'+str(nTrack*10+p)].SetPoint(i+1,pos[2],pos[p])

      zEx = xax.GetBinCenter(xax.GetLast())
      mom    = aTrack.getFittedState().getMom()
      pos      = aTrack.getFittedState().getPos()
      lam      = (zEx-pos.z())/mom.z()
      Ex        = [pos.x()+lam*mom.x(),pos.y()+lam*mom.y()]
      for p in [0,1]:   h['aLine'+str(nTrack*10+p)].SetPoint(i+2,zEx,Ex[p])

      for p in [0,1]:
             tc = h[ 'simpleDisplay'].cd(p+1)
             h['aLine'+str(nTrack*10+p)].SetLineColor(trackColor)
             h['aLine'+str(nTrack*10+p)].SetLineWidth(2)
             h['aLine'+str(nTrack*10+p)].Draw('same')
             tc.Update()
             h[ 'simpleDisplay'].Update()
      nTrack+=1

def twoTrackEvent(sMin=10,dClMin=7,minDistance=1.5,sepDistance=0.5):
        trackTask.clusScifi.Clear()
        trackTask.scifiCluster()
        clusters = trackTask.clusScifi
        sortedClusters={}
        for aCl in clusters:
           so = aCl.GetFirst()//100000
           if not so in sortedClusters: sortedClusters[so]=[]
           sortedClusters[so].append(aCl)
        if len(sortedClusters)<sMin: return False
        M=0
        for x in sortedClusters:
           if len(sortedClusters[x]) == 2:  M+=1
        if M < dClMin: return False
        seeds = {}
        S = [-1,-1]
        for o in range(0,2):
# same procedure for both projections
# take seeds from from first station with 2 clusters
             for s in range(1,6):
                 x = 10*s+o
                 if x in sortedClusters:
                    if len(sortedClusters[x])==2:
                       sortedClusters[x][0].GetPosition(A,B)
                       if o%2==1: pos0 = (A[0]+B[0])/2
                       else: pos0 = (A[1]+B[1])/2
                       sortedClusters[x][1].GetPosition(A,B)
                       if o%2==1: pos1 = (A[0]+B[0])/2
                       else: pos1 = (A[1]+B[1])/2
                       if abs(pos0-pos1) > minDistance:
                         S[o] = s
                         break
             if S[o]<0: break  # no seed found
             seeds[o]={}
             k = -1
             for c in sortedClusters[S[o]*10+o]:
                 k += 1
                 c.GetPosition(A,B)
                 if o%2==1: pos = (A[0]+B[0])/2
                 else: pos = (A[1]+B[1])/2
                 seeds[o][k] = [[c,pos]]
             if k!=1: continue
             if abs(seeds[o][0][0][1] - seeds[o][1][0][1]) < sepDistance: continue
             for s in range(1,6):
               if s==S[o]: continue
               for c in sortedClusters[s*10+o]:
                   c.GetPosition(A,B)
                   if o%2==1: pos = (A[0]+B[0])/2
                   else: pos = (A[1]+B[1])/2
                   for k in range(2):
                        if  abs(seeds[o][k][0][1] - pos) < sepDistance:
                           seeds[o][k].append([c,pos])
        if S[0]<0 or S[1]<0:
            passed = False
        else:
           passed = True
           for o in range(0,2):
              for k in range(2):
                  if len(seeds[o][k])<3:
                      passed = False
                      break
        print(passed)
        if passed:
           tracks = []
           for k in range(2):
             # arbitrarly combine X and Y of combination 0
               n = 0
               hitlist = {}
               for o in range(0,2):
                   for X in seeds[o][k]:
                      hitlist[n] = X[0]
                      n+=1
               theTrack = trackTask.fitTrack(hitlist)
               if not hasattr(theTrack,"getFittedState"):
                    validTrack = False
                    continue
               fitStatus = theTrack.getFitStatus()
               if not fitStatus.isFitConverged():
                    theTrack.Delete()
               else: 
                    tracks.append(theTrack)
           if len(tracks)==2:
                 OT = sink.GetOutTree()
                 OT.Reco_MuonTracks = tracks
                 addTrack(OT,True) 
        return passed

def drawDetectors():
   nodes = {'volMuFilter_1/volFeBlockEnd_1':ROOT.kGreen-6}
   for i in range(2):
      nodes['volVeto_1/volVetoPlane_{}_{}'.format(i, i)]=ROOT.kRed
      for j in range(7):
         nodes['volVeto_1/volVetoPlane_{}_{}/volVetoBar_1{}{:0>3d}'.format(i, i, i, j)]=ROOT.kRed
      nodes['volVeto_1/subVetoBox_{}'.format(i)]=ROOT.kGray+1
   for i in range(4):
      nodes['volMuFilter_1/volMuDownstreamDet_{}_{}'.format(i, i+7)]=ROOT.kBlue+1
      for j in range(60):
         nodes['volMuFilter_1/volMuDownstreamDet_{}_{}/volMuDownstreamBar_ver_3{}{:0>3d}'.format(i, i+7, i, j+60)]=ROOT.kBlue+1
         if i < 3:
            nodes['volMuFilter_1/volMuDownstreamDet_{}_{}/volMuDownstreamBar_hor_3{}{:0>3d}'.format(i, i+7, i, j)]=ROOT.kBlue+1
   for i in range(4):
      nodes['volMuFilter_1/subDSBox_{}'.format(i+7)]=ROOT.kGray+1
   for i in range(5):
      nodes['volTarget_1/ScifiVolume{}_{}000000'.format(i+1, i+1)]=ROOT.kBlue+1
      nodes['volTarget_1/volWallborder_{}'.format(i)]=ROOT.kGray
      nodes['volMuFilter_1/subUSBox_{}'.format(i+2)]=ROOT.kGray+1
      nodes['volMuFilter_1/volMuUpstreamDet_{}_{}'.format(i, i+2)]=ROOT.kBlue+1
      for j in range(10):
         nodes['volMuFilter_1/volMuUpstreamDet_{}_{}/volMuUpstreamBar_2{}00{}'.format(i, i+2, i, j)]=ROOT.kBlue+1
      nodes['volMuFilter_1/volFeBlock_{}'.format(i)]=ROOT.kGreen-6
   for i in range(7,10):
      nodes['volMuFilter_1/volFeBlock_{}'.format(i)]=ROOT.kGreen-6
   passNodes = {'Block', 'Wall'}
   xNodes = {'UpstreamBar', 'VetoBar', 'hor'}
   proj = {'X':0,'Y':1}
   for node_ in nodes:
      node = '/cave_1/Detector_0/'+node_
      for p in proj:
         if node+p in h and any(passNode in node for passNode in passNodes):
            X = h[node+p]
            c = proj[p]
            h['simpleDisplay'].cd(c+1)
            X.Draw('f&&same')
            X.Draw('same')
         else:
            nav.cd(node)
            N = nav.GetCurrentNode()
            S = N.GetVolume().GetShape()
            dx,dy,dz = S.GetDX(),S.GetDY(),S.GetDZ()
            ox,oy,oz = S.GetOrigin()[0],S.GetOrigin()[1],S.GetOrigin()[2]
            P = {}
            M = {}
            if p=='X' and not any(xNode in node for xNode in xNodes):
               P['LeftBottom'] = array('d',[-dx+ox,oy,-dz+oz])
               P['LeftTop'] = array('d',[dx+ox,oy,-dz+oz])
               P['RightBottom'] = array('d',[-dx+ox,oy,dz+oz])
               P['RightTop'] = array('d',[dx+ox,oy,dz+oz])
            elif p=='Y' and 'ver' not in node:
               P['LeftBottom'] = array('d',[ox,-dy+oy,-dz+oz])
               P['LeftTop'] = array('d',[ox,dy+oy,-dz+oz])
               P['RightBottom'] = array('d',[ox,-dy+oy,dz+oz])
               P['RightTop'] = array('d',[ox,dy+oy,dz+oz])
            else: continue
            for C in P:
               M[C] = array('d',[0,0,0])
               nav.LocalToMaster(P[C],M[C])
            h[node+p] = ROOT.TPolyLine()
            X = h[node+p]
            c = proj[p]
            X.SetPoint(0,M['LeftBottom'][2],M['LeftBottom'][c])
            X.SetPoint(1,M['LeftTop'][2],M['LeftTop'][c])
            X.SetPoint(2,M['RightTop'][2],M['RightTop'][c])
            X.SetPoint(3,M['RightBottom'][2],M['RightBottom'][c])
            X.SetPoint(4,M['LeftBottom'][2],M['LeftBottom'][c])
            X.SetLineColor(nodes[node_])
            X.SetLineWidth(1)
            h['simpleDisplay'].cd(c+1)
            if any(passNode in node for passNode in passNodes):
               X.SetFillColorAlpha(nodes[node_], 0.5)
               X.Draw('f&&same')
            X.Draw('same')
def zoom(xmin=None,xmax=None,ymin=None,ymax=None,zmin=None,zmax=None):
# zoom() will reset to default setting
  for d in ['xmin','xmax','ymin','ymax','zmin','zmax']:
     if eval(d): h['c'+d]=eval(d)
     else: h['c'+d]=h[d]
  h['xz'].GetXaxis().SetRangeUser(h['czmin'],h['czmax'])
  h['yz'].GetXaxis().SetRangeUser(h['czmin'],h['czmax'])
  h['xz'].GetYaxis().SetRangeUser(h['cxmin'],h['cxmax'])
  h['yz'].GetYaxis().SetRangeUser(h['cymin'],h['cymax'])
  tc = h['simpleDisplay'].cd(1)
  tc.Update()
  tc = h['simpleDisplay'].cd(2)
  tc.Update()
  h['simpleDisplay'].Update()

def dumpVeto():
    muHits = {10:[],11:[]}
    for aHit in eventTree.Digi_MuFilterHits:
         if not aHit.isValid(): continue
         s = aHit.GetDetectorID()//10000
         if s>1: continue
         p = (aHit.GetDetectorID()//1000)%10
         bar = (aHit.GetDetectorID()%1000)%60
         plane = s*10+p
         muHits[plane].append(aHit)
    for plane in [10,11]:
        for aHit in muHits[plane]:
          S =aHit.GetAllSignals(False,False)
          txt = ""
          for x in S:
              if x[1]>0: txt+=str(x[1])+" "
          print(plane, (aHit.GetDetectorID()%1000)%60, txt)

# decode MuFilter detID
def MuFilter_PlaneBars(detID):
         s = detID//10000
         l  = (detID%10000)//1000  # plane number
         bar = (detID%1000)
         if s>2:
             l=2*l
             if bar>59:
                  bar=bar-60
                  if l<6: l+=1
         return {'station':s,'plane':l,'bar':bar}

def checkOtherTriggers(event,deadTime = 100,debug=False):
      T0 = event.EventHeader.GetEventTime()
      N = event.EventHeader.GetEventNumber()
      Nprev = 1
      rc = event.GetEvent(N-Nprev)
      dt = T0 - event.EventHeader.GetEventTime()
      otherFastTrigger = False
      otherAdvTrigger = False
      tightNoiseFilter = False
      while dt < deadTime:
         otherFastTrigger = False
         for x in event.EventHeader.GetFastNoiseFilters():
             if debug: print('fast:', x.first, x.second )
             if x.second and not x.first == 'Veto_Total': otherFastTrigger = True
         otherAdvTrigger = False
         for x in event.EventHeader.GetAdvNoiseFilters():
             if debug: print('adv:', x.first, x.second )
             if x.second and not x.first == 'VETO_Planes': otherAdvTrigger = True
         if debug: print('pre event ',Nprev,dt,otherFastTrigger,otherAdvTrigger)
         if otherFastTrigger and otherAdvTrigger:
             rc = event.GetEvent(N)
             return otherFastTrigger, otherAdvTrigger, tightNoiseFilter, Nprev, dt
         Nprev+=1
         rc = event.GetEvent(N-Nprev)
         dt = T0 - event.EventHeader.GetEventTime()
      Nprev = 1
      rc = event.GetEvent(N-Nprev)
      dt = T0 - event.EventHeader.GetEventTime()
      while dt < deadTime:
         hits = {1:0,0:0}
         for aHit in event.Digi_MuFilterHits:
            Minfo = MuFilter_PlaneBars(aHit.GetDetectorID())
            s,l,bar = Minfo['station'],Minfo['plane'],Minfo['bar']
            if s>1: continue
            allChannels = aHit.GetAllSignals(False,False)
            hits[l]+=len(allChannels)
         noiseFilter0 = (hits[0]+hits[1])>4.5
         noiseFilter1 = hits[0]>0 and hits[1]>0
         if debug: print('veto hits:',hits)
         if noiseFilter0 and noiseFilter1: 
            tightNoiseFilter = True
            rc = event.GetEvent(N)
            return otherFastTrigger, otherAdvTrigger, tightNoiseFilter, Nprev-1, dt
         Nprev+=1
         rc = event.GetEvent(N-Nprev)
         dt = T0 - event.EventHeader.GetEventTime()
      if Nprev>1: 
            rc = event.GetEvent(N-Nprev+1)
            dt = T0 - event.EventHeader.GetEventTime()
      rc = event.GetEvent(N)
      return otherFastTrigger, otherAdvTrigger, tightNoiseFilter, Nprev-1, dt
      
def cleanTracks():
    OT = sink.GetOutTree()
    listOfDetIDs = {}
    n = 0
    for aTrack in OT.Reco_MuonTracks:
        listOfDetIDs[n] = []
        for i in range(aTrack.getNumPointsWithMeasurement()):
           M =  aTrack.getPointWithMeasurement(i)
           R =  M.getRawMeasurement()
           listOfDetIDs[n].append(R.getDetId())
           if R.getDetId()>0: listOfDetIDs[n].append(R.getDetId()-1)
           listOfDetIDs[n].append(R.getDetId()+1)
        n+=1
    uniqueTracks = []
    for n1 in range( len(listOfDetIDs) ):
       unique = True
       for n2 in range( len(listOfDetIDs) ):
             if n1==n2: continue
             I = set(listOfDetIDs[n1]).intersection(listOfDetIDs[n2])
             if len(I)>0:  unique = False
       if unique: uniqueTracks.append(n1)
    if len(uniqueTracks)>1: 
         for n1 in range( len(listOfDetIDs) ): print(listOfDetIDs[n1])
    return uniqueTracks

def timingOfEvent(makeCluster=False,debug=False):
   ut.bookHist(h,'evTimeDS','cor time of hits;[ns]',70,-5.,30)
   ut.bookHist(h,'evTimeScifi','cor time of hits blue DS red Scifi;[ns]',70,-5.,30)
   ut.bookCanvas(h,'tevTime','cor time of hits',1024,768,1,1)
   h['evTimeScifi'].SetLineColor(ROOT.kRed)
   h['evTimeDS'].SetLineColor(ROOT.kBlue)
   h['evTimeScifi'].SetStats(0)
   h['evTimeDS'].SetStats(0)
   h['evTimeScifi'].SetLineWidth(2)
   h['evTimeDS'].SetLineWidth(2)
   if makeCluster: trackTask.scifiCluster()
   meanXY = {}
   for siCl in trackTask.clusScifi:
       detID = siCl.GetFirst()
       s = detID//1000000
       isVertical = detID%1000000//100000
       siCl.GetPosition(A,B)
       z=(A[2]+B[2])/2.
       pos = (A[1]+B[1])/2.
       L = abs(A[0]-B[0])/2.
       if isVertical:
          pos = (A[0]+B[0])/2.
          L = abs(A[1]-B[1])/2.
       corTime = geo.modules['Scifi'].GetCorrectedTime(detID, siCl.GetTime(), 0) - (z-firstScifi_z)/u.speedOfLight
       h['evTimeScifi'].Fill(corTime)
       if debug: print(detID,corTime,pos)
   for aHit in eventTree.Digi_MuFilterHits:
       detID = aHit.GetDetectorID()
       if not detID//10000==3: continue
       if aHit.isVertical(): nmax = 1
       else: nmax=2
       geo.modules['MuFilter'].GetPosition(detID,A,B)
       z=(A[2]+B[2])/2.
       pos = (A[1]+B[1])/2.
       L = abs(A[0]-B[0])/2.
       if isVertical: 
          pos = (A[0]+B[0])/2.
          L = abs(A[1]-B[1])/2.
       for i in range(nmax):
            corTime = geo.modules['MuFilter'].GetCorrectedTime(detID, i, aHit.GetTime(i)*TDC2ns, 0)- (z-firstScifi_z)/u.speedOfLight
            h['evTimeDS'].Fill(corTime)
            if debug: print(detID,i,corTime,pos)
   tc=h['tevTime'].cd()
   h['evTimeScifi'].Draw()
   h['evTimeDS'].Draw('same')
   tc.Update()
def mufiNoise():
  for s in range(1,4): 
    ut.bookHist(h,'mult'+str(s),'hit mult for system '+str(s),100,-0.5,99.5)
    ut.bookHist(h,'multb'+str(s),'hit mult per bar for system '+str(s),20,-0.5,19.5)
    ut.bookHist(h,'res'+str(s),'residual system '+str(s),20,-10.,10.)
  OT = sink.GetOutTree()
  N=0
  for event in eventTree:
       N+=1
       if N%1000==0: print(N)
       OT.Reco_MuonTracks.Delete()
       rc = trackTask.ExecuteTask("Scifi")
       for aTrack in OT.Reco_MuonTracks:
           mom    = aTrack.getFittedState().getMom()
           pos      = aTrack.getFittedState().getPos()
           if not aTrack.getFitStatus().isFitConverged(): continue
           mult = {1:0,2:0,3:0}
           for aHit in eventTree.Digi_MuFilterHits:
              if not aHit.isValid(): continue
              s = aHit.GetDetectorID()//10000
              S = aHit.GetAllSignals(False,False)
              rc = h['multb'+str(s)].Fill(len(S))
              mult[s]+=len(S)
              if s==2 or s==1:
                 geo.modules['MuFilter'].GetPosition(aHit.GetDetectorID(),A,B)
                 y = (A[1]+B[1])/2.
                 zEx = (A[2]+B[2])/2.
                 lam      = (zEx-pos.z())/mom.z()
                 Ey        = pos.y()+lam*mom.y()
                 rc = h['res'+str(s)].Fill(Ey-y)
           for s in mult: rc = h['mult'+str(s)].Fill(mult[s])
  ut.bookCanvas(h,'noise','',1200,1200,2,3)
  for s in range(1,4):
   tc = h['noise'].cd(s*2-1)
   tc.SetLogy(1)
   h['mult'+str(s)].Draw()
   h['noise'].cd(s*2)
   h['multb'+str(s)].Draw()
  ut.bookCanvas(h,'res','',600,1200,1,3)
  for s in range(1,4):
   tc = h['res'].cd(s)
   h['res'+str(s)].Draw()

def firstTimeStamp(event):
        tmin = [1E9,'']
        digis = [event.Digi_MuFilterHits,event.Digi_ScifiHits]
        for digi in event.Digi_ScifiHits:
               dt = digi.GetTime()
               if dt<tmin[0]:
                    tmin[0]=dt
                    tmin[1]=digi
        for digi in event.Digi_MuFilterHits:
           for t in digi.GetAllTimes():    # will not give time if QDC<0!
               dt = t.second
               if dt<tmin[0]:
                    tmin[0]=dt
                    tmin[1]=digi
        return tmin

def dumpChannels(D='Digi_MuFilterHits'):
     X = eval("eventTree."+D)
     text = {}
     for aHit in X:
         side = 'L'
         txt = "%8i"%(aHit.GetDetectorID())
         for k in range(aHit.GetnSiPMs()*aHit.GetnSides()):
              qdc = aHit.GetSignal(k)
              if qdc < -900: continue
              i = k
              if not k<aHit.GetnSiPMs():
                   i = k-aHit.GetnSiPMs()
                   if side == 'L':
                        txt += " | "
                        side = 'R'
              txt+= "  %2i:%4.1F  "%(i,qdc)
         text[aHit.GetDetectorID()] = txt
     keys = list(text.keys())
     keys.sort()
     for k in keys: print(text[k])

def fillNode(node):
   xNodes = {'UpstreamBar', 'VetoBar', 'hor'}
   proj = {'X':0,'Y':1}
   color = ROOT.kBlack
   thick = 5
   for p in proj:
      if node+p in h:
         X = h[node+p]
         if 'Veto' in node:
            color = ROOT.kRed+1
         if 'Downstream' in node:
            thick = 5
         c = proj[p]
         h[ 'simpleDisplay'].cd(c+1)
         X.SetFillColor(color)
         X.SetLineColor(color)
         X.SetLineWidth(thick)
         X.Draw('f&&same')
         X.Draw('same')   

def drawInfo(pad, k, run, event, timestamp,moreEventInfo=[]):
   drawLogo = True
   drawText = True
   if drawLogo:
      padLogo = ROOT.TPad("logo","logo",0.1,0.1,0.2,0.3)
      padLogo.SetFillStyle(4000)
      padLogo.SetFillColorAlpha(0, 0)
      padLogo.Draw()
      logo = ROOT.TImage.Open('$SNDSW_ROOT/shipLHC/Large__SND_Logo_black_cut.png')
      logo.SetConstRatio(True)
      logo.DrawText(0, 0, 'SND', 98)
      padLogo.cd()
      logo.Draw()
      pad.cd(k)

   if drawText:
    if k==1 or len(moreEventInfo)<5:
      runNumber = eventTree.EventHeader.GetRunId()
      timestamp_print = False
      if not mc and hasattr(eventTree.EventHeader, "GetUTCtimestamp"):
        timestamp_print = True
        time_event= datetime.utcfromtimestamp(eventTree.EventHeader.GetUTCtimestamp())
      padText = ROOT.TPad("info","info",0.19,0.1,0.6,0.3)
      padText.SetFillStyle(4000)
      padText.Draw()
      padText.cd()
      textInfo = ROOT.TLatex()
      textInfo.SetTextAlign(11)
      textInfo.SetTextFont(42)
      textInfo.SetTextSize(.15)
      textInfo.DrawLatex(0, 0.6, 'SND@LHC Experiment, CERN')
      if hasattr(eventTree.EventHeader,'GetEventNumber'): N = eventTree.EventHeader.GetEventNumber()
      else: N = event
      textInfo.DrawLatex(0, 0.4, 'Run / Event: '+str(run)+' / '+str(N))
      if timestamp_print:
           textInfo.DrawLatex(0, 0.2, 'Time (GMT): {}'.format(time_event))
      pad.cd(k)
    elif options.extraInfo:
      padText = ROOT.TPad("info","info",0.29,0.12,0.9,0.35)
      padText.SetFillStyle(4000)
      padText.Draw()
      padText.cd()
      textInfo = ROOT.TLatex()
      textInfo.SetTextAlign(11)
      textInfo.SetTextFont(42)
      textInfo.SetTextSize(.1)
      textInfo.SetTextColor(ROOT.kMagenta+2)
      dely = 0.12
      ystart = 0.85
      for i in range(7):
        textInfo.DrawLatex(0.4, 0.9-dely*i, moreEventInfo[i])
      pad.cd(k)

