import os 
import ROOT
from ROOT import TObjString
import time
from argparse import ArgumentParser
import SndlhcGeo
import shipunit as u

#------------Geo file----------------
parser = ArgumentParser()
parser.add_argument("-f", "--inputFile", dest="inputFile", help="single input file", required=True)
parser.add_argument("-g", "--geoFile", dest="geoFile", help="geofile", required=False)
options = parser.parse_args()
geo = SndlhcGeo.GeoInterface(options.geoFile)
lsOfGlobals = ROOT.gROOT.GetListOfGlobals()
lsOfGlobals.Add(geo.modules['Scifi'])
scifiDet = lsOfGlobals.FindObject('Scifi')
scifiDet.SetConfPar("Scifi/signalSpeed", 15*u.cm/u.nanosecond)
lsOfGlobals.Add(geo.modules['MuFilter'])

#------------------------------------


#-----------Executioner--------------
start = time.time()
fullPath = options.inputFile
print(fullPath)
inRootTFile = ROOT.TFile(fullPath)


# Use FairRoot framework to arrange the workflow
# A FairRun is a wrapper of a collection of tasks
run = ROOT.FairRunAna()
# Input/output manager
ioman = ROOT.FairRootManager.Instance()
source = ROOT.FairFileSource(inRootTFile)
ioman.SetSource(source)
outFile = ROOT.TMemFile('dummy','CREATE') #IGNORE
sink = ROOT.FairRootFileSink(outFile)
ioman.SetSink(sink)
Outputfilename = "/eos/user/p/pbaculim/SWAN_projects/SND/neutrinos/DELETEMEweirdEvents_file109.root"
param = TObjString(Outputfilename)
ioman.RegisterInputObject("outFileName", param)
#avoiding some error messages
xrdb = ROOT.FairRuntimeDb.instance()
xrdb.getContainer("FairBaseParSet").setStatic()
xrdb.getContainer("FairGeoParSet").setStatic()

# Add all task you'd like to run
myTask = ROOT.MCEventBuilder()# XX = name of my task
run.AddTask(myTask)
# Initialize the task collection(Fair run)
run.Init()
firstEvent = 0 #example
nEvents = 0 # example
run.Run(firstEvent, nEvents)

end = time.time()

elapsed = end - start
print(f"Elapsed time: {elapsed:.2f} seconds")

