#!/usr/bin/env python
import ROOT,os,sys,subprocess,atexit,time
import rootUtils as ut
from XRootD import client
from XRootD.client.flags import DirListFlags, OpenFlags, MkDirFlags, QueryCode
import Monitor
import Scifi_monitoring
import Mufi_monitoring
import DAQ_monitoring
import EventDisplay_Task
import SndlhcMuonReco

def pyExit():
       print("Make suicide until solution found for freezing")
       os.system('kill '+str(os.getpid()))
atexit.register(pyExit)

ROOT.gStyle.SetTitleSize(0.045, "XYZ")
ROOT.gStyle.SetLabelSize(0.045, "XYZ")
ROOT.gROOT.ForceStyle()

from argparse import ArgumentParser

parser = ArgumentParser()
parser.add_argument("-A", "--auto", dest="auto", help="run in auto mode online monitoring",default=False,action='store_true')
parser.add_argument("--Nupdate", dest="Nupdate", help="frequence of updating online plots",default=100,type=int)
parser.add_argument("--Nlast",      dest="Nlast", help="last N events to analyze on file",default=10,type=int)
parser.add_argument("--Cosmics",      dest="cosmics", help="use default data stream if no beam",action='store_true',default=False)
parser.add_argument("--sudo", dest="sudo", help="update files on EOS",default=False,action='store_true')

parser.add_argument("-M", "--online", dest="online", help="online mode",default=False,action='store_true')
parser.add_argument("--batch", dest="batch", help="batch mode",default=False,action='store_true')
parser.add_argument("--server", dest="server", help="xrootd server",default=os.environ["EOSSHIP"])
parser.add_argument("-r", "--runNumber", dest="runNumber", help="run number", type=int,default=-1)
parser.add_argument("-p", "--path", dest="path", help="path to converted data",required=False,default="")
parser.add_argument("-praw", dest="rawDataPath", help="path to raw data",required=False,default=False)
parser.add_argument("-P", "--partition", dest="partition", help="partition of data", type=int,required=False,default=-1)
parser.add_argument("-d", "--Debug", dest="debug", help="debug", default=False)
parser.add_argument("-cpp", "--convRawCPP", action='store_true', dest="FairTask_convRaw", help="convert raw data using ConvRawData FairTask", default=False)
parser.add_argument( "--withCalibration", action='store_true', dest="makeCalibration", help="make QDC and TDC calibration, not taking from raw data", default=False)

parser.add_argument("-f", "--inputFile", dest="fname", help="file name for MC", type=str,default=None,required=False)
parser.add_argument("-g", "--geoFile", dest="geoFile", help="geofile", required=False,default=False)
parser.add_argument("-b", "--heartBeat", dest="heartBeat", help="heart beat", default=10000,type=int)
parser.add_argument("-c", "--command", dest="command", help="command", default="")
parser.add_argument("-n", "--nEvents", dest="nEvents", help="number of events", default=-1,type=int)
parser.add_argument("-s", "--nStart", dest="nStart", help="first event", default=0,type=int)
parser.add_argument("-t", "--trackType", dest="trackType", help="DS or Scifi", default="ScifiDS")

parser.add_argument("--ScifiNbinsRes", dest="ScifiNbinsRes", default=100)
parser.add_argument("--Scifixmin", dest="Scifixmin", default=-2000.)
parser.add_argument("--ScifialignPar", dest="ScifialignPar", default=False)
parser.add_argument("--ScifiResUnbiased", dest="ScifiResUnbiased", default=False)
parser.add_argument("--Mufixmin", dest="Mufixmin", default=-10.)
parser.add_argument("--ScifiStationMasked", dest="ScifiStationMasked", default="-9")

parser.add_argument("--goodEvents", dest="goodEvents", action='store_true',default=False)
parser.add_argument("--withTrack", dest="withTrack", action='store_true',default=False)
parser.add_argument("--nTracks", dest="nTracks",default=0,type=int)
parser.add_argument("--save", dest="save", action='store_true',default=False)
parser.add_argument("--sH", dest="saveHistos", action='store_true',default=False,help="save all histos not only TCanvas")
parser.add_argument("--interactive", dest="interactive", action='store_true',default=False)

parser.add_argument("--parallel", dest="parallel",default=1,type=int)

parser.add_argument("--postScale", dest="postScale",help="post scale events, 1..10..100", default=-1,type=int)

parser.add_argument("--saveTo", dest="saveTo", help="output storage path", default="")

options = parser.parse_args()
options.slowStream = True
if options.cosmics: options.slowStream = False
options.startTime = ""
options.dashboard = "/mnt/raid10/data_online/run_status.json"
options.monitorTag = ''
if (options.auto and not options.interactive) or options.batch: ROOT.gROOT.SetBatch(True)

# if no geofile given, use defaults according to run number
if options.runNumber < 0  and not options.geoFile: 
     print('No run number given and no geoFile. Do not know what to do. Exit.')
     exit()
#RUN0: 7 Apr 2022 - 26 Jul 2022   (Run 4575 started -  test run after replacing emulsions -Ettore)
#RUN1: 26 Jul 2022 - 13 Sept 2022 (Run 4855 September 14)
#RUN2: 13 Sept 2022 - 4 Nov 2022 (Run 5172 test run after emulsion replacement)
#RUN3: 4 Nov 2022  -  

if not options.geoFile:
   if options.path.find('TI18')<0:
     if options.path.find('2022')>0 : options.geoFile =  "geofile_sndlhc_TI18_V0_2022.root"
     if options.path.find('2023')>0 : options.geoFile =  "geofile_sndlhc_TI18_V1_2023.root"
     if options.path.find('2024')>0 : options.geoFile =  "geofile_sndlhc_TI18_V0_2024.root"
   else:
     if options.runNumber < 4575:
           options.geoFile =  "geofile_sndlhc_TI18_V3_08August2022.root"
     elif options.runNumber < 4855:
          options.geoFile =  "geofile_sndlhc_TI18_V5_14August2022.root"
     elif options.runNumber < 5172:
          options.geoFile =  "geofile_sndlhc_TI18_V6_08October2022.root"
     else:
          options.geoFile =  "geofile_sndlhc_TI18_V7_22November2022.root"

# to be extended for future new alignments.

def currentRun():
      with client.File() as f:
            f.open(options.server+options.dashboard)
            status, L = f.read()
            Lcrun = L.decode().split('\n')
            f.close()
      curRun,curPart,start ="","",""
      X=eval(Lcrun[0])
      if not (X['state']=='running'):
           print("DAQ not running.",X)
      else:
           curRun = 'run_'+str(X['run_number']).zfill(6)
           curFile = X['currently_written_file']
           k = curFile.rfind('data_')+5
           curPart = int(curFile[k:k+4])
           start = X['start_time']
           # assume monitor file always present on DAQ server
           if options.slowStream: options.monitorTag = 'monitoring_'
           else:  options.monitorTag = ''
      return curRun,curPart,start

if options.auto:
   options.online = True
# search for current run
   if options.runNumber < 0:
        curRun = ""
        while curRun.find('run') < 0:
               curRun,curPart,options.startTime =  currentRun()
               if curRun.find('run') < 0:
                   print("no run info, sleep 30sec.",time.ctime())
                   time.sleep(30)
        options.runNumber = int(curRun.split('_')[1])
        lastRun = curRun
        if options.slowStream:   options.partition = 0   #   monitoring file set to data_0000.root   int(curPart.split('_')[1].split('.')[0])
        else:                             options.partition = int(curPart.split('_')[1].split('.')[0])
else:
   if options.runNumber < 0:
       print("run number required for non-auto mode")
       os._exit(1)
   if options.rawDataPath: rawDataPath = options.rawDataPath
# works only for runs on EOS
   if not options.server.find('eos')<0:
      if options.rawDataPath: rawDataPath = options.rawDataPath
      elif options.path.find('2025')>0:
          rawDataPath = "/eos/experiment/sndlhc/raw_data/physics/2025/run_251"
      elif options.path.find('2024')>0:
          rawDataPath = "/eos/experiment/sndlhc/raw_data/physics/2024/run_241"
      elif options.path.find('2023')>0:
          rawDataPath = "/eos/experiment/sndlhc/raw_data/physics/2023/"
      elif options.path.find('2022')>0:
          rawDataPath = "/eos/experiment/sndlhc/raw_data/physics/2022/"
      else:
          rawDataPath = "/eos/experiment/sndlhc/raw_data/commissioning/TI18/data/"
      options.rawDataPath = rawDataPath
      runDir = rawDataPath+"run_"+str(options.runNumber).zfill(6)
      jname = "run_timestamps.json"
      dirlist  = str( subprocess.check_output("xrdfs "+os.environ['EOSSHIP']+" ls "+runDir,shell=True) ) 
      if jname in dirlist:
         with client.File() as f:          
            f.open(options.server+runDir+"/run_timestamps.json")
            status, jsonStr = f.read()
         exec("date = "+jsonStr.decode())
         options.startTime = date['start_time'].replace('Z','')
         if 'stop_time' in date:
             options.startTime += " - "+ date['stop_time'].replace('Z','')

# prepare tasks:
FairTasks = []
houghTransform = False # under construction, not yet tested
if houghTransform:
   muon_reco_task = SndlhcMuonReco.MuonReco()
   muon_reco_task.SetName("houghTransform")
   FairTasks.append(muon_reco_task)
else:
   import SndlhcTracking
   trackTask = SndlhcTracking.Tracking() 
   trackTask.SetName('simpleTracking')
   FairTasks.append(trackTask)

M = Monitor.Monitoring(options,FairTasks)
if options.nEvents < 0 :   options.nEvents = M.GetEntries()
if options.postScale==0 and options.nEvents>100E6: options.postScale = 100
if options.postScale==0 and options.nEvents>10E6: options.postScale = 10
print('using postScale ',options.postScale,' for run ',options.runNumber)
monitorTasks = {}
if not options.fname:
   monitorTasks['daq']     = DAQ_monitoring.DAQ_boards()
   monitorTasks['rates']   = DAQ_monitoring.Time_evolution()
monitorTasks['Scifi_hitMaps']   = Scifi_monitoring.Scifi_hitMaps()
monitorTasks['Mufi_hitMaps']   = Mufi_monitoring.Mufi_hitMaps()
monitorTasks['Mufi_QDCcorellations']   = Mufi_monitoring.Mufi_largeVSsmall()
if options.postScale<2: monitorTasks['Veto_Efficiency']   = Mufi_monitoring.Veto_Efficiency()
#monitorTasks['Scifi_residuals'] = Scifi_monitoring.Scifi_residuals()   # time consuming
if options.interactive:  monitorTasks['EventDisplay']   = EventDisplay_Task.twod()

for m in monitorTasks:
    monitorTasks[m].Init(options,M)

# monitorTasks['Veto_Efficiency'].debug  = True

if not options.auto:   # default online/offline mode
 process = []
 pid = 0
 for i in range(options.parallel):
   if options.parallel==1:
     nstart,nstop = options.nStart,options.nStart+options.nEvents
   else:
     try:
       pid = os.fork()
     except OSError:
       print("Could not create a child process")
     print('pid',pid,i)
     if pid!=0:
          process.append(pid)
     else:
         dN = options.nEvents//options.parallel
         nstart = i*dN
         nstop =  nstart + dN
         if i==(options.parallel-1): nstop = options.nEvents
   if pid == 0:
    print('start ',i,nstart,nstop)
    Tcounter = {'Monitor':0}
    for m in monitorTasks:
       Tcounter[m] = 0

    for n in range(nstart,nstop):
     if options.postScale>1:
        if ROOT.gRandom.Rndm()>1./options.postScale: continue
     tic = time.perf_counter_ns()
     event = M.GetEvent(n)
     toc = time.perf_counter_ns()
     Tcounter['Monitor']+=toc-tic
     
     if not options.online:
        if n%options.heartBeat == 0:
            print("--> run/event nr: %i %i %5.2F%%"%(M.eventTree.EventHeader.GetRunId(),n,(n-nstart)/(nstop-nstart)*100))
# assume for the moment file does not contain fitted tracks
     for m in monitorTasks:
        tic = time.perf_counter_ns()
        monitorTasks[m].ExecuteEvent(M.eventTree)
        toc = time.perf_counter_ns()
        Tcounter[m]+=toc-tic
    for m in monitorTasks:
          monitorTasks[m].Plot()
    txt = ''
    for x in Tcounter: txt+=x+':%5.1Fs '%(Tcounter[x]/1E9)
    print('timing performance:',txt)
    if options.parallel>1: # save partitions
           ut.writeHists(M.h,'tmp'+str(options.runNumber)+'p'+str(i))
           exit(0)
 if options.parallel>1: 
     while process:
          pid,exit_code = os.wait()
          if pid == 0: time.sleep(100)
          else: 
                print('child process has finished',len(process)-1,pid,exit_code)
                process.remove(pid)
     for i in range(options.parallel):
        tmp = 'tmp'+str(options.runNumber)+'p'+str(i)
        if tmp in os.listdir('.'):         ut.readHists(M.h,tmp)
        else: print('file missing ',tmp)
     M.presenterFile.Close()
     if options.saveTo=="":
       name = 'run'+self.runNr+'.root'
     else:
       name = options.saveTo+'run'+M.runNr+'_'+str(options.nStart//1000000)+'.root'
     M.presenterFile = ROOT.TFile(name,'update')

     for m in monitorTasks:
          monitorTasks[m].Plot()
     # check if all events had been processed
     if 'Etime' in M.h:
       if not M.h['Etime'].GetEntries() == options.nEvents:
         print('event count failed! Processed:',M.h['Etime'].GetEntries(),' total number of events:',options.nEvents)
       else:
         print('i am finished, all events processed')
 if options.saveHistos: ut.writeHists(M.h,'allHistos-run'+M.runNr+'.root')

 M.publishRootFile()
 if options.sudo:
     print(options.runNumber,options.startTime)
     options.startTime += " #events="+str(options.nEvents)
     M.updateHtml()
else: 
   """ auto mode
       check for open data file on the online machine
       analyze N events 
       re-open file and check for new events, 
       if none available, 
         check every 5 seconds: if no new file re-open again
         if new file, finish run, publish histograms, and restart with new file
   """
   N0 = 0
   if options.slowStream: lastPart = 0   #   reading from second low speed DAQ stream    tmp[len(tmp)-1]
   nLast = options.nEvents
   nStart = nLast-options.Nlast
   if not options.interactive and options.sudo: M.updateHtml()
   lastFile = M.converter.fiN.GetName()
   if not options.slowStream:
     tmp = lastFile.split('/')
     lastRun  = tmp[len(tmp)-2].replace('monitoring_','')
     lastPart = tmp[len(tmp)-1]

   M.updateSource(lastFile)
   while 1>0:
      for n in range(nStart,nLast):
        event = M.GetEvent(n)
        N0+=1
        # trackTask.event = event
        for m in monitorTasks:
           monitorTasks[m].ExecuteEvent(M.eventTree)
# update plots
        if N0%options.Nupdate==0 or N0==int(options.Nupdate/10):
           for m in monitorTasks:
               monitorTasks[m].Plot()
           if options.sudo: M.publishRootFile()

      M.updateSource(lastFile)
      newEntries = M.GetEntries()
      if newEntries>nLast:
         nStart = max(nLast,newEntries-options.Nlast)
         nLast = newEntries
      else:  
      # check if file has changed
         curRun,curPart,options.startTime  =  currentRun()
         while curRun.find('run') < 0:
               curRun,curPart,options.startTime  =  currentRun()
               if curRun.find('run') < 0:  
                   print("sleep 300sec.",curRun,time.ctime())
                   time.sleep(300)
         if not curRun == lastRun:
            for m in monitorTasks:
               if not options.interactive:  monitorTasks[m].Plot()
            print("run ",lastRun," has finished.")
            quit()  # reinitialize everything with new run number
         if not curPart == lastPart and not options.slowStream:
            lastPart = curPart
            lastFile = options.server+options.path+options.monitorTag+lastRun+"/"+ lastPart
            M.converter.fiN = ROOT.TFile.Open(lastFile)
         else:
            time.sleep(10) # sleep 10 seconds and check for new events
            print('DAQ inactive for 10sec. Last event = ',M.GetEntries(), curRun,curPart,N0)
            nStart = nLast
