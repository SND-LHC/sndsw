#!/usr/bin/env python
import atexit
firstEvent = 0

def pyExit():
       "nasty hack"
       # This is needed to bypass seg violation with exiting cpp digitization
       # Most likely related to file ownership.
       os.system('kill '+str(os.getpid()))

import resource
def mem_monitor():
 # Getting virtual memory size 
    pid = os.getpid()
    with open(os.path.join("/proc", str(pid), "status")) as f:
        lines = f.readlines()
    _vmsize = [l for l in lines if l.startswith("VmSize")][0]
    vmsize = int(_vmsize.split()[1])
    #Getting physical memory size  
    pmsize = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
    print("memory: virtuell = %5.2F MB  physical = %5.2F MB"%(vmsize/1.0E3,pmsize/1.0E3))

import ROOT,os,sys
import shipRoot_conf
import shipunit as u

shipRoot_conf.configure()

from argparse import ArgumentParser
parser = ArgumentParser()
parser.add_argument("-f", "--inputFile", dest="inputFile", help="single input file", required=True)
parser.add_argument("-g", "--geoFile", dest="geoFile", help="geofile", required=True)
parser.add_argument("-n", "--nEvents", dest="nEvents",  type=int, help="number of events to process", default=100000)
parser.add_argument("-ts", "--thresholdScifi", dest="ts", type=float, help="threshold energy for Scifi [p.e.]", default=3.5)
parser.add_argument("-ss", "--saturationScifi", dest="ss", type=float, help="saturation energy for Scifi [p.e.]", default=104.)
parser.add_argument("-tML", "--thresholdMufiL", dest="tml", type=float, help="threshold energy for Mufi large [p.e.]", default=0.0)
parser.add_argument("-tMS", "--thresholdMufiS", dest="tms", type=float, help="threshold energy for Mufi small [p.e.]", default=0.0)
parser.add_argument("-no-cls", "--noClusterScifi", action='store_true', help="do not make Scifi clusters")
parser.add_argument("-cpp", "--digiCPP", action='store_true', dest="FairTask_digi", help="perform digitization using DigiTaskSND")
parser.add_argument("-d", "--Debug", dest="debug", help="debug", default=False)
parser.add_argument("--copy-emulsion-points", action='store_true', help="Copy emulsion points from input file (potentially large file size!).")

options = parser.parse_args()
# rephrase the no-cluster flag
makeClusterScifi = not options.noClusterScifi
# -----Timer-------------
timer = ROOT.TStopwatch()
timer.Start()

# outfile should be in local directory
tmp     = options.inputFile.split('/')
outFile = tmp[len(tmp)-1].replace('.root','_dig.root')
if options.inputFile.find('/eos')==0:
   if options.FairTask_digi:
       options.inputFile = os.environ['EOSSHIP']+options.inputFile
   else:   
       os.system('xrdcp '+os.environ['EOSSHIP']+options.inputFile+' '+outFile)
else:
    if not options.FairTask_digi:
       os.system('cp '+options.inputFile+' '+outFile) 
    
# -----Create geometry----------------------------------------------
import shipLHC_conf as sndDet_conf

if options.geoFile.find('/eos')==0:
       options.geoFile = os.environ['EOSSHIP']+options.geoFile
import SndlhcGeo
snd_geo = SndlhcGeo.GeoInterface(options.geoFile)

# set digitization parameters for MuFilter
lsOfGlobals  = ROOT.gROOT.GetListOfGlobals()
scifiDet     = lsOfGlobals.FindObject('Scifi')
mufiDet      = lsOfGlobals.FindObject('MuFilter')
scifiDet.SetConfPar("Scifi/nphe_min",options.ts)   # threshold
scifiDet.SetConfPar("Scifi/nphe_max",options.ss) # saturation

####
# The lines below aim to reproduce the original digitization case, but urge the user to regenerate
# the sample shall it be outdated from before the removal of the 1MeV production cut, which
# coincides with MuFi digi const update.
#
# in MC productions generated before July 2022 Scifi signal speed is missing from the geofile
better_update = False
if scifiDet.GetConfParF("Scifi/signalSpeed")==0:
  scifiDet.SetConfPar("Scifi/signalSpeed", 15*u.cm/u.nanosecond)
  better_update = True
# geofiles before March 2026 don't have the Veto 3 atten.length  
if mufiDet.GetConfParF("MuFilter/VTAttenuationLength")==0:
  mufiDet.SetConfPar("MuFilter/VTAttenuationLength",999*u.cm)
  better_update = True
# old digi constants from before the MuFi response update 
if mufiDet.GetConfParF("MuFilter/VandUpAttenuationLength")==999*u.cm:
  better_update = True
# in very ancient MC productions it is possible some digitization params are missing
# set them here values updated in March 2026.
if mufiDet.GetConfParF("MuFilter/DsAttenuationLength")==0 or\
     mufiDet.GetConfParF("MuFilter/VandUpPropSpeed")==0 :
  mufiDet.SetConfPar("MuFilter/DsAttenuationLength",230*u.cm)
  mufiDet.SetConfPar("MuFilter/DsTAttenuationLength",700*u.cm)
  mufiDet.SetConfPar("MuFilter/VandUpAttenuationLength",210*u.cm)
  mufiDet.SetConfPar("MuFilter/VTAttenuationLength",999*u.cm)
  mufiDet.SetConfPar("MuFilter/DsSiPMcalibration",25.*1000.)
  # 1.65 MeV = 41 qcd over 6 Large SiPMs(one side)
  mufiDet.SetConfPar("MuFilter/VandUpSiPMcalibrationL",50.*1000.)
  # no MIP signal for small SiPMs, delayed and compromised response in general
  mufiDet.SetConfPar("MuFilter/VandUpSiPMcalibrationS",0.)
  mufiDet.SetConfPar("MuFilter/VandUpPropSpeed",13.6*u.cm/u.nanosecond);
  mufiDet.SetConfPar("MuFilter/DsPropSpeed",15.1*u.cm/u.nanosecond);
  scifiDet.SetConfPar("Scifi/timeResol",150.*u.picosecond)
  mufiDet.SetConfPar("MuFilter/timeResol",150.*u.picosecond) # time resolution in ps, first guess
  better_update = True

if better_update:
  print("WARNING: Simulation file preceding the production cut change! Consider regenerating from scratch!")
####

# Fair digitization task
if options.FairTask_digi:
  run = ROOT.FairRunAna()
  ioman = ROOT.FairRootManager.Instance()
  ioman.RegisterInputObject('Scifi', snd_geo.modules['Scifi'])
  ioman.RegisterInputObject('MuFilter', snd_geo.modules['MuFilter'])
  # Don't use FairRoot's default event header settings
  run.SetEventHeaderPersistence(False)
  
  # Set input
  fileSource = ROOT.FairFileSource(options.inputFile)
  run.SetSource(fileSource)
  # Set output
  outfile = ROOT.FairRootFileSink(outFile.replace('.root','CPP.root'))
  run.SetSink(outfile)

  # Set number of events to process
  inRootFile = ROOT.TFile.Open(options.inputFile)
  inTree = inRootFile.Get('cbmsim')
  nEventsInFile = inTree.GetEntries()
  nEvents = min(nEventsInFile, options.nEvents)

  rtdb = run.GetRuntimeDb()
  DigiTask = ROOT.DigiTaskSND()
  DigiTask.withScifiClusters(makeClusterScifi)
  DigiTask.set_copy_emulsion_points(options.copy_emulsion_points)
  run.AddTask(DigiTask)
  run.Init()
  run.Run(firstEvent, nEvents)

# Digitization using python code SndlhcDigi
else:
  if options.copy_emulsion_points:
      print("ERROR: copying of emulsion points only configurable when using DigiTask")

 # import digi task
  import SndlhcDigi
  Sndlhc = SndlhcDigi.SndlhcDigi(outFile,makeClusterScifi)
  nEvents   = min(Sndlhc.sTree.GetEntries(),options.nEvents)
# main loop
  for iEvent in range(firstEvent, nEvents):
    if iEvent % 50000 == 0 or options.debug:
        print('event ', iEvent, nEvents - firstEvent)
    Sndlhc.iEvent = iEvent
    rc = Sndlhc.sTree.GetEvent(iEvent)
    Sndlhc.digitize()
    if makeClusterScifi:
       Sndlhc.clusterScifi()
 # memory monitoring
 # mem_monitor()

  # end loop over events
  Sndlhc.finish()
  
timer.Stop()
rtime = timer.RealTime()
ctime = timer.CpuTime()
print(' ') 
print("Real time ",rtime, " s, CPU time ",ctime,"s")
