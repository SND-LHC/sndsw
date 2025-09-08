import os 
import ROOT
from ROOT import TObjString
import time
from argparse import ArgumentParser
import SndlhcGeo
import shipunit as u

# ------------ Argument parser ----------------
parser = ArgumentParser()
parser.add_argument("-f", "--inputFile", help="Input file")
parser.add_argument("-g", "--geoFile", help="geo file")
parser.add_argument("-o", "--outputFile", help="Output file")
parser.add_argument("-i", "--firstEvent", type=int, default=0, help="First event to process")
parser.add_argument("-n", "--nEvents", type=int, default=0, help="Number of events to process (0 = all)")
parser.add_argument("--saveFirst25nsOnly", action="store_true", help="Only store the first 25-ns chunk of each event")
options = parser.parse_args()

# ------------ Geo setup ----------------
geo = SndlhcGeo.GeoInterface(options.geoFile)
lsOfGlobals = ROOT.gROOT.GetListOfGlobals()
lsOfGlobals.Add(geo.modules['Scifi'])
scifiDet = lsOfGlobals.FindObject('Scifi')
scifiDet.SetConfPar("Scifi/signalSpeed", 15*u.cm/u.nanosecond)
lsOfGlobals.Add(geo.modules['MuFilter'])


#-----------Executioner--------------
start = time.time()
inRootTFile = ROOT.TFile(options.inputFile)
print(f"Input file: {options.inputFile}")

# Use FairRoot framework to arrange the workflow
# A FairRun is a wrapper of a collection of tasks
run = ROOT.FairRunAna()

# Input/output manager
ioman = ROOT.FairRootManager.Instance()
source = ROOT.FairFileSource(inRootTFile)
run.SetSource(source)
outFile = ROOT.TMemFile('dummy','CREATE') #IGNORE
sink = ROOT.FairRootFileSink(outFile)
run.SetSink(sink)
ioman.InitSink()
run.SetEventHeaderPersistence(False)

#Avoiding some error messages
xrdb = ROOT.FairRuntimeDb.instance()
xrdb.getContainer("FairBaseParSet").setStatic()
xrdb.getContainer("FairGeoParSet").setStatic()

# Add tasks 
eventBuilder = ROOT.MCEventBuilder(options.outputFile, options.saveFirst25nsOnly)
run.AddTask(eventBuilder)

# Initialize and run
run.Init()
run.Run(options.firstEvent, options.firstEvent+options.nEvents)

end = time.time()
elapsed = end - start
print(f"Elapsed time: {elapsed:.2f} seconds")
