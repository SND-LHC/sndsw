#!/usr/bin/env python
import ROOT,os,sys,subprocess,atexit,time
import Monitor
import SndlhcMuonReco
import SndlhcTracking

def pyExit():
       print("Make suicide until solution found for freezing")
       os.system('kill '+str(os.getpid()))
atexit.register(pyExit)

from argparse import ArgumentParser
parser = ArgumentParser()
parser.add_argument("-o", "--outFile", dest="oname", help="output file name", type=str,default=None,required=True)
parser.add_argument("-f", "--inputFile", dest="fname", help="file name for MC", type=str,default=None,required=False)
parser.add_argument("--server", dest="server", help="xrootd server",default=os.environ["EOSSHIP"])
parser.add_argument("-r", "--runNumber", dest="runNumber", help="run number", type=int,required=False,default=-1)
parser.add_argument("-p", "--path", dest="path", help="path to file",required=False,default="")
parser.add_argument("-P", "--partition", dest="partition", help="partition of data", type=int,required=False,default=-1)
parser.add_argument("-g", "--geoFile", dest="geoFile", help="geofile", required=True)
parser.add_argument("-b", "--heartBeat", dest="heartBeat", help="heart beat", default=10000,type=int)
parser.add_argument("-c", "--command", dest="command", help="command", default="")
parser.add_argument("-n", "--nEvents", dest="nEvents", help="number of events", default=-1,type=int)
parser.add_argument("-s", "--nStart", dest="nStart", help="first event", default=0,type=int)
parser.add_argument("-st", "--simpleTracking", dest="simpleTracking", action='store_true', default=False)
parser.add_argument("-genfitFormat", "--genfitFormat", dest='genfitFormat', action='store_true', help="output track format for simple tracking when only it is run", default=False)
parser.add_argument("-t", "--trackType", dest="trackType", help="DS, Scifi, ScifiDS", default="ScifiDS")

parser.add_argument("--ScifiNbinsRes", dest="ScifiNbinsRes", default=100)
parser.add_argument("--Scifixmin", dest="Scifixmin", default=-2000.)
parser.add_argument("--ScifialignPar", dest="ScifialignPar", default=False)

parser.add_argument("--goodEvents", dest="goodEvents", action='store_true',default=False)
parser.add_argument("--withTrack", dest="withTrack", action='store_true',default=False)
parser.add_argument("--nTracks", dest="nTracks",default=0,type=int)
parser.add_argument("--save", dest="save", action='store_true',default=False)

parser.add_argument("-ht", "--HoughTracking", dest="HoughTracking", action='store_true', default=False)
parser.add_argument("-par", "--parFile", dest="parFile", help="parameter file", default=os.environ['SNDSW_ROOT']+"/python/TrackingParams.yml")
parser.add_argument("-hf", "--HoughSpaceFormat", dest="HspaceFormat", help="Hough space representation. Should match the 'Hough_space_format' name in parFile, use quotes", default='linearSlopeIntercept')

parser.add_argument("-sc", "--scale",dest="scaleFactor",  help="Randomly run reconstruction.", required=False,  default=1, type=int)

options = parser.parse_args()

# prepare tasks:
options.FairTasks = {}
options.genfitTrack = False
HT_tasks = []
if options.HoughTracking:
   if options.trackType == 'Scifi' or options.trackType == 'ScifiDS':
      muon_reco_task_Sf = SndlhcMuonReco.MuonReco()
      muon_reco_task_Sf.SetTrackingCase('passing_mu_Sf')
      muon_reco_task_Sf.SetName("houghTransform_Sf")
      options.FairTasks["houghTransform_Sf"] = muon_reco_task_Sf
      HT_tasks.append(muon_reco_task_Sf)
   if options.trackType == 'DS' or options.trackType == 'ScifiDS':
      muon_reco_task_DS = SndlhcMuonReco.MuonReco()
      muon_reco_task_DS.SetTrackingCase('passing_mu_DS')
      muon_reco_task_DS.SetName("houghTransform_DS")
      options.FairTasks["houghTransform_DS"] = muon_reco_task_DS
      HT_tasks.append(muon_reco_task_DS)
   for ht_task in HT_tasks:
       ht_task.SetParFile(options.parFile)
       ht_task.SetHoughSpaceFormat(options.HspaceFormat)
if options.simpleTracking:
   trackTask = SndlhcTracking.Tracking()
   trackTask.SetName("simpleTracking")
   # If HT task is also used, pass the track format from its input yaml file
   # else consult with the command line genfitFormat option
   if options.HoughTracking: pass
   else: options.genfitTrack = options.genfitFormat
   options.FairTasks["simpleTracking"] = trackTask
M = Monitor.TrackSelector(options)
if options.nEvents < 0 : 
    options.nEvents = M.eventTree.GetEntries()

M.Execute()
M.Finalize()
