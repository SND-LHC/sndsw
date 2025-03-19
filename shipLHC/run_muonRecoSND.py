import ROOT
from argparse import ArgumentParser
import os

import SndlhcMuonReco

parser = ArgumentParser()
parser.add_argument("-f", "--inputFile", dest="inputFile", help="single input file", required=True)
parser.add_argument("-g", "--geoFile", dest="geoFile", help="geofile", required=False)
parser.add_argument("-o", "--withOutput", dest="withOutput", help="persistent output", action='store_true',default=False)
parser.add_argument("-s", "--saveTo", dest="outPath", help="output storage path", type=str,default="",required=False)
parser.add_argument("-par", "--parFile", dest="parFile", help="parameter file", required=False, default=os.environ['SNDSW_ROOT']+"/python/TrackingParams.xml")
parser.add_argument("-c", "--case", dest="trackingCase", help="type of tracks to build. Should match the 'tracking_case' name in parFile, use quotes", required=True)
parser.add_argument("-hf", "--HoughSpaceFormat", dest="HspaceFormat", help="Hough space representation. Should match the 'Hough_space_format' name in parFile, use quotes", required=True)
parser.add_argument("-n", "--nEvents", dest="nEvents",  type=int, help="number of events to process", default=1100000)
parser.add_argument("-i", "--firstEvent",dest="firstEvent",  help="First event of input file to use", required=False,  default=0, type=int)
parser.add_argument("-sc", "--scale",dest="scaleFactor",  help="Run reconstruction once for a randomly selected event in every [scaleFactor] events.", required=False,  default=1, type=int)

options = parser.parse_args()

x = options.inputFile
filename = x[x.rfind('/')+1:]
if x.rfind('/run_')>0:
   runN = x[x.rfind('/run_')+5:x.rfind('/run_')+11]
   path =  x[:x.rfind('/run_')+1]
else:
     runN = ""
     path =  x[:x.rfind('/')+1]
outFileName = options.outPath+filename.replace('.root','_'+runN+'_muonReco.root')

# Set the geometry file
import SndlhcGeo

if not options.geoFile:
   if runN == "" : 
      print('\033[91m'+'Error!'+'\033[0m'+' No run number detected. Must provide a geo file then!')
      exit()
   if options.inputFile.find('TI18')<0:
      if options.inputFile.find('physics/2022')>=0: options.geoFile =  path+"geofile_sndlhc_TI18_V0_2022.root"
   else:
     if int(runN) < 4575:
          options.geoFile = "geofile_sndlhc_TI18_V3_08August2022.root"
     elif int(runN) < 4855:
          options.geoFile = "geofile_sndlhc_TI18_V5_14August2022.root"
     elif int(runN) < 5172:
          options.geoFile = "geofile_sndlhc_TI18_V6_08October2022.root"
     else:
          options.geoFile = "geofile_sndlhc_TI18_V7_22November2022.root"
     # Introducing SNDLHCEventHeader in 2022 run 4791, so there will be tag!='' in the detector classes
     # and one has to use the */physics/2022 geofiles
     if int(runN) >= 4791 and int(runN) <= 5429: path = "/eos/experiment/sndlhc/convertedData/physics/2022/"
     options.geoFile = path + options.geoFile

# Check geo file is a match for the data
if (options.inputFile.find('/TI18')>=0 and options.geoFile.find('V0_2022')>=0 ) or \
   (options.inputFile.find('physics/2022')>=0 and options.geoFile.find('/TI18')>=0 ) or \
   (options.inputFile.find('/TI18')>=0 and options.geoFile.find('/TI18')>=0 and int(runN) >= 4791 and int(runN) <= 5429 ):
     print('\033[91m'+'Error!'+'\033[0m'+' Consider a different geo file for that input file! Exitting..')
     exit()

geo = SndlhcGeo.GeoInterface(options.geoFile)
lsOfGlobals = ROOT.gROOT.GetListOfGlobals()
lsOfGlobals.Add(geo.modules['Scifi'])
lsOfGlobals.Add(geo.modules['MuFilter'])

fullPath = options.inputFile
if options.inputFile.find('/eos/experiment')==0:
     fullPath = os.environ['EOSSHIP']+options.inputFile
F = ROOT.TFile.Open(fullPath)

if options.withOutput:
  outFile = ROOT.TFile(outFileName, 'RECREATE')
else:
  outFile = ROOT.TMemFile(outFileName,'CREATE')

treename = None
for test_treename in ["rawConv", "cbmsim"] :
     if hasattr(F, test_treename) :
          treename = test_treename
          break

if treename is None :
     raise RuntimeError("File {0} contains no object with a valid SND@LHC TTree name".format(fullPath))

fairRootManager = ROOT.FairRootManager.Instance()
fairRootManager.SetTreeName(treename)

run = ROOT.FairRunAna()
print("Initialized FairRunAna")

source = ROOT.FairFileSource(F)
run.SetSource(source)

sink = ROOT.FairRootFileSink(outFile)
run.SetSink(sink)
fairRootManager.InitSink()
# Don't use FairRoot's default event header settings
run.SetEventHeaderPersistence(False)

muon_reco_task = SndlhcMuonReco.MuonReco()
run.AddTask(muon_reco_task)

# Start timer
w = ROOT.TStopwatch()

# Set the parameter file - must be called before Init()
muon_reco_task.SetParFile(options.parFile)
muon_reco_task.SetTrackingCase(options.trackingCase)
muon_reco_task.SetHoughSpaceFormat(options.HspaceFormat)
# setting a flag to let the task know which manager called it
# needed as output handling differs for this manager and run_TrackSelections.
muon_reco_task.SetStandalone()
run.Init()

# Set the scale factor - must be after Init()
print("Setting a scale factor to:", options.scaleFactor)
muon_reco_task.SetScaleFactor(options.scaleFactor)

# Set number of events to process
nEvents = min( options.nEvents, source.GetEntries())

run.Run(options.firstEvent, options.firstEvent + nEvents)
w.Stop()
print('Real Time:', w.RealTime(), ' CPU time: ', w.CpuTime())
print("Done running")
