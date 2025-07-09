
#!/usr/bin/env python
import ROOT,os,sys
import boardMappingParser
import csv
import time
import calendar
from rootpyPickler import Unpickler
import json

# raw data from Ettore: https://cernbox.cern.ch/index.php/s/Ten7ilKuD3qdnM2 

class ConvRawDataPY(ROOT.FairTask):
   " raw data conversion by event "
   def Init(self,options):
      self.debug        = options.debug
      self.monitoring = options.online
      self.auto = options.auto
      local = False
      if (options.path.find('eos')<0 and options.server.find('root://')<0 and not options.online) or os.path.isdir(options.path):    local = True
      if local: server = ''
      else:
           server = options.server
           from XRootD import client
           # https://xrootd.slac.stanford.edu/doc/python/xrootd-python-0.1.0/examples.html
      if options.runNumber>0: 
         runNr   = str(options.runNumber).zfill(6)
         path      = options.path+'run_'+ runNr+'/'
         if options.partition < 0:
             inFile   = 'data.root'
             self.outFile = "sndsw_raw_"+runNr+'.root'
         else:
             part = str(options.partition).zfill(4)
             inFile   = 'data_'+part+'.root'
             self.outFile = "sndsw_raw_"+runNr+'-'+part+'.root'
      if self.monitoring:  # this is an online stream
         server = options.server
         runNr   = str( abs(options.runNumber) ).zfill(6)
         path    = options.path+'run_'+ runNr+'/'
         inFile   = 'data_'+part+'.root'
         self.outFile = ROOT.TMemFile('monitorRawData', 'recreate')

# get filling scheme per run
      self.fsdict = False      
      try:
         if options.path.find('2022')!=-1: fpath = "/eos/experiment/sndlhc/convertedData/physics/2022/"
         elif options.path.find('2023')!=-1: fpath = "/eos/experiment/sndlhc/convertedData/physics/2023/"
         elif options.path.find('2024')!=-1: fpath = "/eos/experiment/sndlhc/convertedData/physics/2024/"
         else: fpath = "/eos/experiment/sndlhc/convertedData/commissioning/TI18/"
         fg = ROOT.TFile.Open(options.server+fpath+"/FSdict.root")
         pkl = Unpickler(fg)
         FSdict = pkl.load('FSdict')
         fg.Close()
         if options.runNumber in FSdict: 
             if 'B1' in FSdict[options.runNumber]: self.fsdict = FSdict[options.runNumber]
      except:
         print('continue without knowing filling scheme',options.server+options.path)
      
      # put the run's FS in format to be passed to FairTasks as input
      self.FSmap = ROOT.TMap()
      if self.fsdict:   
         # For LHC ion runs the phase shift of beam 2 could not be
         # determined and is assigned 0 or 1718(=1782-64) in the FS dict
         # For pp runs phase2 is 3564-129.
         if self.fsdict['phaseShift2'] == 0 or self.fsdict['phaseShift2'] == 1718:
             Nbunches = 1782
         else: Nbunches = 3564 # proton runs
         for bunchNumber in range (0, Nbunches):
             nb1 = (Nbunches + bunchNumber - self.fsdict['phaseShift1'])%Nbunches
             nb2 = (Nbunches + bunchNumber - self.fsdict['phaseShift1']- self.fsdict['phaseShift2'])%Nbunches
             b1 = nb1 in self.fsdict['B1']
             b2 = nb2 in self.fsdict['B2']
             IP1 = False
             IP2 = False
             if b1:
                IP1 =  self.fsdict['B1'][nb1]['IP1']
             if b2:
                IP2 =  self.fsdict['B2'][nb2]['IP2']
             self.FSmap.Add(ROOT.TObjString(str(bunchNumber)), ROOT.TObjString(str(int(IP2))+str(int(IP1))+str(int(b2))+str(int(b1))))
      else: self.FSmap.Add(ROOT.TObjString("0"), ROOT.TObjString("-1"))

      self.run     = ROOT.FairRunAna()
      self.ioman = ROOT.FairRootManager.Instance()
      ioman = self.ioman
# open input file
      if options.auto:
           mpath = options.path+options.monitorTag+'run_'+ runNr+'/'
           self.fiN=ROOT.TFile.Open(server+mpath+inFile)
      else:
           self.fiN=ROOT.TFile.Open(server+path+inFile)
      self.nEvents = 1
      self.newFormat = True
      if self.fiN.Get('event'):   self.newFormat = False # old format
      if not self.monitoring:
        if self.newFormat:
           if options.nEvents<0:  self.nEvents = self.fiN.data.GetEntries()
           else: self.nEvents = min(options.nEvents,self.fiN.data.GetEntries())
        else:
           if options.nEvents<0:  self.nEvents = self.fiN.event.GetEntries()
           else: self.nEvents = min(options.nEvents,self.fiN.event.GetEntries())
      print('converting ',self.nEvents,' events ',' of run',options.runNumber)
  # Pass input parameters to the task - runN, paths, etc.
      ioman.RegisterInputObject('runN', ROOT.TObjString(str(options.runNumber)))
      ioman.RegisterInputObject('pathCalib', ROOT.TObjString(server+path))
      ioman.RegisterInputObject('pathJSON', ROOT.TObjString(server+path))
      ioman.RegisterInputObject('nEvents', ROOT.TObjString(str(self.nEvents)))
      ioman.RegisterInputObject('nStart', ROOT.TObjString(str(options.nStart)))
      ioman.RegisterInputObject('debug', ROOT.TObjString(str(options.debug)))
      ioman.RegisterInputObject('stop', ROOT.TObjString(str(options.stop)))
      ioman.RegisterInputObject('heartBeat', ROOT.TObjString(str(options.heartBeat)))
      ioman.RegisterInputObject('makeCalibration', ROOT.TObjString(str(int(options.makeCalibration))))
      ioman.RegisterInputObject('chi2Max', ROOT.TObjString(str(options.chi2Max)))
      ioman.RegisterInputObject('saturationLimit', ROOT.TObjString(str(options.saturationLimit)))
      ioman.RegisterInputObject('local', ROOT.TObjString(str(int(local))))
      ioman.RegisterInputObject('newFormat', ROOT.TObjString(str(int(self.newFormat))))
      ioman.RegisterInputObject('FSmap', self.FSmap)
      self.options = options
      
  # Initialize logger: set severity and verbosity
      logger = ROOT.FairLogger.GetLogger()
      logger.SetColoredLog(True)
      logger.SetLogVerbosityLevel('low')
      logger.SetLogScreenLevel('error')
      logger.SetLogToScreen(True)
      if options.debug:
         logger.SetLogToFile(True)
         logger.SetLogFileName('run_'+str(options.runNumber)+'-'+part+'_CPP.log')
         logger.SetLogFileLevel('info')   
         logger.SetLogScreenLevel('info')
      
  # Pass raw data file as input object
      ioman.RegisterInputObject("rawData", self.fiN)

  # Set output
      if self.monitoring:  self.outfile = ROOT.FairRootFileSink(self.outFile)
      elif options.FairTask_convRaw: self.outfile = ROOT.FairRootFileSink(self.outFile.replace('.root','_CPP.root'))
      else:  self.outfile = ROOT.FairRootFileSink(self.outFile)
      self.run.SetSink(self.outfile)

  # Don't use FairRoot's default event header settings
      self.run.SetEventHeaderPersistence(False)
      self.xrdb = ROOT.FairRuntimeDb.instance()
      self.xrdb.getContainer("FairBaseParSet").setStatic()
      self.xrdb.getContainer("FairGeoParSet").setStatic()
# Fair convRawData task
      if options.FairTask_convRaw:
          self.run.AddTask(ROOT.ConvRawData())
          self.fSink = self.outfile
          #X = fSink.GetRootFile()
          #X.SetCompressionLevel(ROOT.CompressionSettings(ROOT.kLZMA, 5)) # it seems it has no effect
          self.run.Init()

#-------end of init for cpp ------------------------------------
      else:
       if options.makeCalibration:
         if  local:
           fqdc_cal = open(path+'/qdc_cal.csv')
           Lqdc = fqdc_cal.readlines()
           ftdc_cal = open(path+'/tdc_cal.csv')
           Ltdc = ftdc_cal.readlines()
         else:
           with client.File() as f:
               f.open(server+path+"/qdc_cal.csv")
               status, L = f.read()
               Lqdc = L.decode().split('\n')
               f.close()
               f.open(server+path+"/tdc_cal.csv")
               status, L = f.read()
               Ltdc = L.decode().split('\n')
               f.close()
  # calibration data
         self.qdc_cal = {}
         qdc_cal = self.qdc_cal
         L = Lqdc
         for l in range(1,len(L)):
               tmp = L[l].replace('\n','').split(',')
               if len(tmp)<10:continue
               board_id = int(tmp[0])
               if not board_id in qdc_cal: qdc_cal[board_id]={}
               fe_id = int(tmp[1])
               if not fe_id in qdc_cal[board_id]: qdc_cal[board_id][fe_id]={}
               channel = int(tmp[2])
               if not channel in qdc_cal[board_id][fe_id]: qdc_cal[board_id][fe_id][channel]={}
               tac = int(tmp[3])
               if not tac in qdc_cal[board_id][fe_id][channel]: qdc_cal[board_id][fe_id][channel][tac]={}
               X = qdc_cal[board_id][fe_id][channel][tac]
               X['a']=float(tmp[4])
               X['b']=float(tmp[5])
               X['c']=float(tmp[6])
               X['d']=float(tmp[8])
               X['e']=float(tmp[10])
               if float(tmp[9]) < 2: X['chi2Ndof'] = 999999.
               else:                  X['chi2Ndof']=float(tmp[7])/float(tmp[9])

         L=Ltdc
         for l in range(1,len(L)):
               tmp = L[l].replace('\n','').split(',')
               if len(tmp)<9:continue
               board_id = int(tmp[0])
               if not board_id in qdc_cal: qdc_cal[board_id]={}
               fe_id = int(tmp[1])
               if not fe_id in qdc_cal[board_id]: qdc_cal[board_id][fe_id]={}
               channel = int(tmp[2])
               if not channel in qdc_cal[board_id][fe_id]: qdc_cal[board_id][fe_id][channel]={}
               tac = int(tmp[3])
               if not tac in qdc_cal[board_id][fe_id][channel]: qdc_cal[board_id][fe_id][channel][tac]={}
               tdc = int(tmp[4])
               if not tdc in qdc_cal[board_id][fe_id][channel][tac]: qdc_cal[board_id][fe_id][channel][tac][tdc]={}
               X = qdc_cal[board_id][fe_id][channel][tac][tdc]
               X['a']=float(tmp[5])
               X['b']=float(tmp[6])
               X['c']=float(tmp[7])
               X['d']=float(tmp[9])
               if float(tmp[10]) < 2: X['chi2Ndof'] = 999999.
               else:                  X['chi2Ndof']=float(tmp[8])/float(tmp[10])

  # read MuFilter mappings of SiPM channel to tofpet_id and tofpet_channel
       SiPMmap={}
       self.TofpetMap = {}
       key = {'DS':2,'US':1,'Veto':0}
       sndswPath = os.environ['SNDSW_ROOT']
       for system in key:
           infile = sndswPath+"/geometry/"+system+"_SiPM_mapping.csv"
           SiPMmap[system] = {}
           if os.path.isdir(sndswPath+"/geometry"):
               with open(infile, "r") as f:
                   reader = csv.DictReader(f, delimiter=',')
                   for i, row in enumerate(reader):
                       SiPMmap[system][int(row['SiPM'])] = [int(row['CN']), int(row['pin']), int(row['TOFPET']), int(row['CHANNEL'])]
                   self.TofpetMap[key[system]] = {}
                   for channel in SiPMmap[system]:
                      row = SiPMmap[system][channel]
                      self.TofpetMap[key[system]][row[2]*1000+row[3]] = channel
       if local:
           with open(path+'board_mapping.json') as f:
               jsonStr = f.read()
       else:
           with client.File() as f:
                f.open(server+path+"/board_mapping.json")
                status, jsonStr = f.read()
  # pass the read string to getBoardMapping()
       self.boardMaps = boardMappingParser.getBoardMapping(jsonStr)

       self.slots = {0:'A',1:'A',2:'B',3:'B',4:'C',5:'C',6:'D',7:'D'}

       self.MufiSystem = {}
       for b in self.boardMaps['MuFilter']:
           board_id = int(b.split('_')[1])
           self.MufiSystem[board_id]={}
           for x in self.boardMaps['MuFilter'][b]:
               for slot in self.slots:
                  s = 0
                  tmp = self.boardMaps['MuFilter'][b][x].split('_')[0]
                  if tmp=='US': s = 1
                  elif tmp=='DS': s = 2
                  if self.slots[slot]==x: self.MufiSystem[board_id][slot]=s

       self.offMap={}
                                        # first bar, number of sipm channels / bar and direction
       for s in range(1,3):
           for o in ['Left','Right']: 
              self.offMap['Veto_'+str(s)+o] =[10000 + (s-1)*1000+ 0,8,2]    # first channel, nSiPMs, nSides, from bottom to top
       self.offMap['Veto_3Vert'] = [10000 + 2*1000+ 6,-8,1]
       for s in range(1,6):
           for o in ['Left','Right']: 
              self.offMap['US_'+str(s)+o] =[20000 + (s-1)*1000+ 9,-8,2]     # from top to bottom
       for s in range(1,5):
           for o in ['Vert']: 
              self.offMap['DS_'+str(s)+o] =[30000 + (s-1)*1000+ 119, -1,1] # direction not known
           if s>3: continue
           for o in ['Left','Right']: 
              self.offMap['DS_'+str(s)+o] =[30000 + (s-1)*1000+ 59,-1,2]   # direction not known

       self.boards = {}
       for b in self.fiN.GetListOfKeys():
          name = b.GetName()
          if name.find('board')!=0: continue
          self.boards[name]=self.fiN.Get(name)

       self.fSink  = self.outfile
       self.sTree = ROOT.TTree('rawConv','raw data converted')
       ROOT.gDirectory.pwd()
       self.header  = ROOT.SNDLHCEventHeader()
       eventSND  = self.sTree.Branch("EventHeader.",self.header,32000,-1)

       self.digiSciFi   = ROOT.TClonesArray("sndScifiHit")
       self.digiSciFiBranch   = self.sTree.Branch("Digi_ScifiHits",self.digiSciFi,32000,1)
       self.digiMuFilter   = ROOT.TClonesArray("MuFilterHit")
       self.digiMuFilterHitBranch   = self.sTree.Branch("Digi_MuFilterHits",self.digiMuFilter,32000,1)
       self.geoVersion = 0

       B = ROOT.TList()
       B.SetName('BranchList')
       B.Add(ROOT.TObjString('sndScifiHit'))
       B.Add(ROOT.TObjString('MuFilterHit'))
       B.Add(ROOT.TObjString('SNDLHCEventHeader'))
       self.fSink.WriteObject(B,"BranchList", ROOT.TObject.kSingleKey)
       self.fSink.SetRunId(options.runNumber)
       self.fSink.SetOutTree(self.sTree)

       if self.newFormat: self.run_startUTC = self.getStartTime()

#-------end of init for py ------------------------------------

   def channel(self,tofpet_id,tofpet_channel,position):
      return (64 * tofpet_id + 63 - tofpet_channel + 512*position) # 512 channels per mat, 1536 channels per plane
                                                                                                                                     # one channel covers all 6 layers of fibres

  # following procedure in https://gitlab.cern.ch/scifi-telescope/scifi-rec/-/blob/ettore-tofpet/lib/processors/applycalibration.cpp
   def qdc_chi2(self,board_id,tofpet_id,channel,tac,TDC=0):
      qdc_cal = self.qdc_cal
      par    = qdc_cal[board_id][tofpet_id][channel][tac]
      parT = qdc_cal[board_id][tofpet_id][channel][tac][TDC]
      return max(par['chi2Ndof'],parT['chi2Ndof'])

   def qdc_sat(self,board_id,tofpet_id,channel,tac,v_fine):
      qdc_cal = self.qdc_cal
      par = qdc_cal[board_id][tofpet_id][channel][tac]
      return v_fine/par['d']
    
   def comb_calibration(self,board_id,tofpet_id,channel,tac,v_coarse,v_fine,t_coarse,t_fine,GQDC = 1.0, TDC=0): # max gain QDC = 3.6
      qdc_cal = self.qdc_cal
      par  = qdc_cal[board_id][tofpet_id][channel][tac]
      parT = par[TDC]
      x    = t_fine
      ftdc = (-parT['b']-ROOT.TMath.Sqrt(parT['b']**2-4*parT['a']*(parT['c']-x)))/(2*parT['a']) #   Ettore 28/01/2022 +parT['d']
      timestamp = t_coarse + ftdc
      tf = timestamp - t_coarse
      x = v_coarse - tf
      fqdc = -par['c']*ROOT.TMath.Log(1+ROOT.TMath.Exp( par['a']*(x-par['e'])**2-par['b']*(x-par['e']) )) + par['d']
      value = (v_fine-fqdc)/GQDC
      return timestamp,value,max(par['chi2Ndof'],parT['chi2Ndof']),v_fine/par['d']

   def calibrationReport(self):
      qdc_cal = self.qdc_cal
      report = {}
      TDC = 0
      for b in qdc_cal:
           for t in  qdc_cal[b]:
                for c in qdc_cal[b][t]:
                     for tac in qdc_cal[b][t][c]:
                               par    = qdc_cal[b][t][c][tac]
                               if 'chi2Ndof' in par:                             chi2 = par['chi2Ndof']
                               else: chi2=-1
                               parT = qdc_cal[b][t][c][tac][TDC]
                               if 'chi2Ndof' in parT:   chi2T = parT['chi2Ndof']
                               else: chi2T=-1
                               key = tac +10*c + t*10*100 + b*10*100*100
                               if not key in report: report[key]=[chi2,chi2T]
      return report

   def debugMapping(self,board,tofpet_id,tofpet_channel):
                  key = (tofpet_id%2)*1000 + tofpet_channel
                  tmp = boardMaps['MuFilter'][board][self.slots[tofpet_id]]
                  sipmChannel = TofpetMap[system][key]-1
                  nSiPMs =   abs(offMap[tmp][1])
                  nSides =   abs(offMap[tmp][2])
                  direction            = int(offMap[tmp][1]/nSiPMs)
                  detID                   = offMap[tmp][0] + direction*(sipmChannel//(nSiPMs))
                  sipm_number = sipmChannel%(nSiPMs)
                  print(sipmChannel,nSiPMs,nSides,detID,sipm_number)

   def printChannelInfo(self):
         runNr   = str( abs(self.options.runNumber) ).zfill(6)
         path    = self.options.path+'run_'+ runNr+'/'
         from XRootD import client
         with client.File() as f:
               f.open(self.options.server+path+"/channels.csv")
               status, L = f.read()
               self.Lchannel = L.decode().split('\n')
               f.close()
         for l in self.Lchannel: print(l)

   def printCurrentRun(self):
         filename = "currently_processed_file.txt"
         path    = self.options.path
         from XRootD import client
         with client.File() as f:
               f.open(self.options.server+path+filename)
               status, L = f.read()
               self.Lcrun = L.decode().split('\n')
               f.close()
         for l in self.Lcrun: print(l)

   def getStartTime(self):
      runNr = str( abs(self.options.runNumber) ).zfill(6)
      path  = self.options.path+'run_'+ runNr+'/'
      jname = "run_timestamps.json"
      from XRootD import client
      with client.File() as f:
               f.open(self.options.server+path+jname)
               status, jsonStr = f.read()
               f.close()
      date = json.loads(jsonStr)
      time_str = date['start_time'].replace('Z','')
      time_obj = time.strptime(time_str, '%Y-%m-%dT%H:%M:%S')
      startTimeOfRun = calendar.timegm(time_obj)
      return startTimeOfRun

  # reading hits and converting to event information

  # https://gitlab.cern.ch/snd-scifi/software/-/wikis/Raw-data-format 
  # tac: 0-3, identifies the Time-To-Analogue converter used for this hit (each channel has four of them and they require separate calibration).
  # t_coarse: Coarse timestamp of the hit, running on a 4 times the LHC clock
  # t_fine: 0-1023, fine timestamp of the hit. It contains the raw value of the charge digitized by the TDC and requires calibration.
  # v_coarse: 0-1023, QDC mode: it represents the number of clock cycles the charge integration lasted.
  # v_fine = 0-1023, QDC mode: represents the charge measured. Requires calibration.

   def Run(self):
      if self.options.FairTask_convRaw:
          self.run.Run(self.options.nStart, self.nEvents)
      else:
          for eventNumber in range(self.options.nStart,self.nEvents):
             self.executeEvent(eventNumber)
          # fill TTree
             self.sTree.Fill()
   def executeEvent(self,eventNumber):
       if self.newFormat: self.executeEvent1(eventNumber)
       else:              self.executeEvent0(eventNumber)
   def executeEvent1(self,eventNumber):
     if self.options.FairTask_convRaw:
          # update source and run conversion starting from a custom eventN 
          if self.auto:              
             self.run.GetTask("ConvRawData").UpdateInput(eventNumber)
          self.run.Run(self.options.nStart, self.nEvents)
          Fout = self.outfile.GetRootFile()
          self.sTree = Fout.Get('cbmsim')
          return   
     if eventNumber%self.options.heartBeat==0 or self.debug:
               print('run ',self.options.runNumber, ' event',eventNumber," ",time.ctime())

     event = self.fiN.data
     event.GetEvent(eventNumber)
     self.header.SetEventTime(event.evt_timestamp)
     self.header.SetUTCtimestamp(int(event.evt_timestamp*6.23768*1e-9 + self.run_startUTC))
     self.header.SetEventNumber(event.evt_number) #   for new event header
     self.header.SetFlags(event.evt_flags)
     self.header.SetRunId( self.options.runNumber )
     if self.FSmap.GetEntries()>1:
          if self.header.GetAccMode()== 12: # ion runs
            self.header.SetBunchType(int(str(self.FSmap.GetValue(str(int((event.evt_timestamp%(4*3564))/8))))))
          else: # proton runs
            self.header.SetBunchType(int(str(self.FSmap.GetValue(str(int((event.evt_timestamp%(4*3564))/4))))))
     else:
          self.header.SetBunchType(int(str(self.FSmap.GetValue("0"))))

     indexSciFi=0
     self.digiSciFi.Delete()
     digiSciFiStore = {}
     indexMuFilter = 0
     self.digiMuFilter.Delete()
     digiMuFilterStore = {}

     for n in range(event.n_hits):
           board_id = event.boardId[n]
           board = 'board_'+str(board_id)
           scifi = True
           if board in self.boardMaps['Scifi']:
              station,mat = self.boardMaps['Scifi'][board]
           elif board in self.boardMaps['MuFilter']:
              scifi = False
           else: 
             print(board+' not known. Serious error, stop')
             1/0
           mask = False
           tofpet_id = event.tofpetId[n]
           tofpet_channel = event.tofpetChannel[n]
           if self.options.makeCalibration:
               if self.options.debug:
                    tac = event.tac[n]
                    print(scifi,board_id,tofpet_id,tofpet_channel,tac,event.tCoarse[n],event.tFine[n],event.vCoarse[n],event.vFine[n])
               TDC,QDC,Chi2ndof,satur = self.comb_calibration(board_id,tofpet_id,tofpet_channel,tac,event.vCoarse[n],event.vFine[n],event.tCoarse[n],event.tFine[n])
           else:
               TDC = event.timestamp[n]
               QDC = event.value[n]
               QDCchi2N = event.valueCalChi2[n]/event.valueCalDof[n]
               TDCchi2N = event.timestampCalChi2[n]/event.timestampCalDof[n]
               Chi2ndof = max( QDCchi2N,TDCchi2N )
               satur = event.value_saturation[n]
           if Chi2ndof > self.options.chi2Max:
                         if QDC>1E20:    QDC = 997.   # checking for inf
                         if QDC != QDC:  QDC = 998.   # checking for nan
                         if QDC>0: QDC = -QDC
                         mask = True
           elif satur  > self.options.saturationLimit or QDC>1E20 or QDC != QDC:
                         if QDC>1E20:    QDC = 987.   # checking for inf
                         if self.options.debug: 
                                print('inf',board_id,tofpet_id,tofpet_channel,event.tac[n],event.vCoarse[n],event.vFine[n],TDC-event.tCoarse[n],eventNumber,Chi2ndof)
                         if QDC != QDC:  QDC = 988.   # checking for nan
                         if self.options.debug: 
                                 print('nan',board_id,tofpet_id,tofpet_channel,event.tac[n],event.vCoarse[n],event.vFine[n],TDC-event.tCoarse[n],eventNumber,Chi2ndof)
                         A = int(min( QDC,1000))
                         B = min(satur,999)/1000.
                         QDC = A+B
                         mask = True
           elif Chi2ndof > self.options.chi2Max:
                         if QDC>0: QDC = -QDC
                         mask = True
           if self.options.debug:
                    print('calibrated: tdc = ',TDC,'  qdc = ',QDC)  # TDC clock cycle = 160 MHz or 6.25ns

           if not scifi:
  # mufi encoding
                  system = self.MufiSystem[board_id][tofpet_id]
                  key = (tofpet_id%2)*1000 + tofpet_channel
                  tmp = self.boardMaps['MuFilter'][board][self.slots[tofpet_id]]
                  if self.options.debug or not tmp.find('not')<0: print('debug',tmp,system,key,board,tofpet_id,tofpet_id%2,tofpet_channel)
                  sipmChannel = 99
                  if not key in self.TofpetMap[system]:
                          print('key does not exist',key)
                          print(system, key,board,tofpet_id, self.TofpetMap[system])
                  else:
                         sipmChannel = self.TofpetMap[system][key]-1
                  nSiPMs = abs(self.offMap[tmp][1])
                  nSides =   abs(self.offMap[tmp][2])
                  direction            = int(self.offMap[tmp][1]/nSiPMs)
                  detID                   = self.offMap[tmp][0] + direction*(sipmChannel//(nSiPMs))
                  sipm_number = sipmChannel%(nSiPMs)
                  if tmp.find('Right')>0:
                         sipm_number += nSiPMs
                  if not detID in digiMuFilterStore:
                          digiMuFilterStore[detID] =  ROOT.MuFilterHit(detID,nSiPMs,nSides)
                  test = digiMuFilterStore[detID].GetSignal(sipm_number)
                  digiMuFilterStore[detID].SetDigi(QDC,TDC,sipm_number)
                  digiMuFilterStore[detID].SetDaqID(sipm_number,n,board_id, tofpet_id, tofpet_channel)
                  if mask: digiMuFilterStore[detID].SetMasked(sipm_number)

                  if self.options.debug:
                      print('create mu hit: ',detID,tmp,system,tofpet_id,self.offMap[tmp],sipmChannel,nSiPMs,nSides,test)
                      print('                ',detID,sipm_number,QDC,TDC)
                  if test>0 or detID%1000>200 or sipm_number>15:
                      print('what goes wrong?',detID,sipm_number,system,key,board,tofpet_id,tofpet_channel,test)

           else:
  # scifi encoding
                  chan = self.channel(tofpet_id,tofpet_channel,mat)
                  orientation = 1
                  if station[2]=="Y": orientation = 0
                  sipmLocal = (chan - mat*512)
                  sipmID = 1000000*int(station[1]) + 100000*orientation + 10000*mat + 1000*(sipmLocal//128) + chan%128
                  if not sipmID in digiSciFiStore: 
                       digiSciFiStore[sipmID] =  ROOT.sndScifiHit(sipmID)
                  digiSciFiStore[sipmID].SetDigi(QDC,TDC)
                  digiSciFiStore[sipmID].SetDaqID(0,n, board_id, tofpet_id, tofpet_channel)
                  if mask: digiSciFiStore[sipmID].setInvalid()
                  if self.options.debug:
                      print('create scifi hit: tdc = ',board,sipmID,QDC,TDC)
                      print('tofpet:', tofpet_id,tofpet_channel,mat,chan)
                      print(station[1],station[2],mat,chan,int(chan/128)%4,chan%128)

# copy hits to detector branches
     for sipmID in digiSciFiStore:
               if self.digiSciFi.GetSize() == indexSciFi: self.digiSciFi.Expand(indexSciFi+100)
               self.digiSciFi[indexSciFi]=digiSciFiStore[sipmID]
               indexSciFi+=1
     for detID in digiMuFilterStore:
               if self.digiMuFilter.GetSize() == indexMuFilter: self.digiMuFilter.Expand(indexMuFilter+100)
               self.digiMuFilter[indexMuFilter]=digiMuFilterStore[detID]
               indexMuFilter+=1
   def executeEvent0(self,eventNumber):
     if self.options.FairTask_convRaw:
          # update source and run conversion starting from a custom eventN 
          if self.auto:              
             self.run.GetTask("ConvRawData").UpdateInput(eventNumber)
          self.run.Run(self.options.nStart, self.nEvents)
          Fout = self.outfile.GetRootFile()
          self.sTree = Fout.Get('cbmsim')
          return   
     if eventNumber%self.options.heartBeat==0 or self.debug:
               print('run ',self.options.runNumber, ' event',eventNumber," ",time.ctime())
     event = self.fiN.event
     rc = event.GetEvent(eventNumber)
     self.header.SetEventTime(event.timestamp)
     self.header.SetRunId( self.options.runNumber )
     self.header.SetInputFileId(self.geoVersion)

     indexSciFi=0
     self.digiSciFi.Delete()
     digiSciFiStore = {}
     indexMuFilter = 0
     self.digiMuFilter.Delete()
     digiMuFilterStore = {}

     for board in self.boards:
        board_id = int(board.split('_')[1])
        scifi = True
        if board in self.boardMaps['Scifi']:
            station,mat = self.boardMaps['Scifi'][board]
        elif board in self.boardMaps['MuFilter']:
            scifi = False
        else: 
             print(board+' not known. Serious error, stop')
             1/0
        bt = self.boards[board]
        rc  = bt.GetEvent(eventNumber)
        for n in range(bt.n_hits):
           mask = False
           tofpet_id = bt.tofpet_id[n]
           tofpet_channel = bt.tofpet_channel[n]
           if self.options.makeCalibration:
               if self.options.debug:
                    print(scifi,board_id,bt.tofpet_id[n],bt.tofpet_channel[n],bt.tac[n],bt.t_coarse[n],bt.t_fine[n],bt.v_coarse[n],bt.v_fine[n])
               tac = bt.tac[n]
               TDC,QDC,Chi2ndof,satur = self.comb_calibration(board_id,tofpet_id,tofpet_channel,tac,bt.v_coarse[n],bt.v_fine[n],bt.t_coarse[n],bt.t_fine[n])
           else:
               TDC = bt.timestamp[n]
               QDC = bt.value[n]
               Chi2ndof = 1
               satur = 0.
           if Chi2ndof > self.options.chi2Max:
                         if QDC>1E20:    QDC = 997.   # checking for inf
                         if QDC != QDC:  QDC = 998.   # checking for nan
                         if QDC>0: QDC = -QDC
                         mask = True
           elif satur  > self.options.saturationLimit or QDC>1E20 or QDC != QDC:
                         if QDC>1E20:    QDC = 987.   # checking for inf
                         if self.options.debug: 
                                print('inf',board_id,bt.tofpet_id[n],bt.tofpet_channel[n],bt.tac[n],bt.v_coarse[n],bt.v_fine[n],TDC-bt.t_coarse[n],eventNumber,Chi2ndof)
                         if QDC != QDC:  QDC = 988.   # checking for nan
                         if self.options.debug: 
                                 print('nan',board_id,bt.tofpet_id[n],bt.tofpet_channel[n],bt.tac[n],bt.v_coarse[n],bt.v_fine[n],\
                                          TDC-bt.t_coarse[n],TDC,bt.t_coarse[n],eventNumber,Chi2ndof)
                         A = int(min( QDC,1000))
                         B = min(satur,999)/1000.
                         QDC = A+B
                         mask = True
           elif Chi2ndof > self.options.chi2Max:
                         if QDC>0: QDC = -QDC
                         mask = True
           if self.options.debug:
                    print('calibrated: tdc = ',TDC,'  qdc = ',QDC)  # TDC clock cycle = 160 MHz or 6.25ns

           if not scifi:
  # mufi encoding
                  system = self.MufiSystem[board_id][tofpet_id]
                  key = (tofpet_id%2)*1000 + tofpet_channel
                  tmp = self.boardMaps['MuFilter'][board][self.slots[tofpet_id]]
                  if self.options.debug or not tmp.find('not')<0: print('debug',tmp,system,key,board,tofpet_id,tofpet_id%2,tofpet_channel)
                  sipmChannel = 99
                  if not key in self.TofpetMap[system]:
                          print('key does not exist',key)
                          print(system, key,board,tofpet_id, self.TofpetMap[system])
                  else:
                         sipmChannel = self.TofpetMap[system][key]-1
                  nSiPMs = abs(self.offMap[tmp][1])
                  nSides =   abs(self.offMap[tmp][2])
                  direction            = int(self.offMap[tmp][1]/nSiPMs)
                  detID                   = self.offMap[tmp][0] + direction*(sipmChannel//(nSiPMs))
                  sipm_number = sipmChannel%(nSiPMs)
                  if tmp.find('Right')>0:
                         sipm_number += nSiPMs
                  if not detID in digiMuFilterStore:
                          digiMuFilterStore[detID] =  ROOT.MuFilterHit(detID,nSiPMs,nSides)
                  test = digiMuFilterStore[detID].GetSignal(sipm_number)
                  digiMuFilterStore[detID].SetDigi(QDC,TDC,sipm_number)
                  digiMuFilterStore[detID].SetDaqID(sipm_number, board_id, tofpet_id, tofpet_channel)
                  if mask: digiMuFilterStore[detID].SetMasked(sipm_number)

                  if self.options.debug:
                      print('create mu hit: ',detID,tmp,system,tofpet_id,self.offMap[tmp],sipmChannel,nSiPMs,nSides,test)
                      print('                ',detID,sipm_number,QDC,TDC)
                  if test>0 or detID%1000>200 or sipm_number>15:
                      print('what goes wrong?',detID,sipm_number,system,key,board,tofpet_id,tofpet_channel,test)

           else:
  # scifi encoding
                  chan = self.channel(tofpet_id,tofpet_channel,mat)
                  orientation = 1
                  if station[2]=="Y": orientation = 0
                  sipmLocal = (chan - mat*512)
                  sipmID = 1000000*int(station[1]) + 100000*orientation + 10000*mat + 1000*(sipmLocal//128) + chan%128
                  if not sipmID in digiSciFiStore: 
                       digiSciFiStore[sipmID] =  ROOT.sndScifiHit(sipmID)
                  digiSciFiStore[sipmID].SetDigi(QDC,TDC)
                  digiSciFiStore[sipmID].SetDaqID(0, board_id, tofpet_id, tofpet_channel)
                  if mask: digiSciFiStore[sipmID].setInvalid()
                  if self.options.debug:
                      print('create scifi hit: tdc = ',board,sipmID,QDC,TDC)
                      print('tofpet:', tofpet_id,tofpet_channel,mat,chan)
                      print(station[1],station[2],mat,chan,int(chan/128)%4,chan%128)

# copy hits to detector branches
     for sipmID in digiSciFiStore:
               if self.digiSciFi.GetSize() == indexSciFi: self.digiSciFi.Expand(indexSciFi+100)
               self.digiSciFi[indexSciFi]=digiSciFiStore[sipmID]
               indexSciFi+=1
     for detID in digiMuFilterStore:
               if self.digiMuFilter.GetSize() == indexMuFilter: self.digiMuFilter.Expand(indexMuFilter+100)
               self.digiMuFilter[indexMuFilter]=digiMuFilterStore[detID]
               indexMuFilter+=1


   def Finalize(self):
      if self.options.FairTask_convRaw:
  # overwrite cbmsim 
          F = self.outfile.GetRootFile()
          T = F.Get("cbmsim")
          T.SetName('rawConv')
          T.Write()
          self.fiN.Close()
          self.outfile.Close()
      else:
         if self.options.debug:
             print('number of events processed',self.sTree.GetEntries(),self.fiN.event.GetEntries())
         self.sTree.Write()
         self.fiN.Close()
         self.fSink.Close()
      print("File closed ",self.fiN.GetName())
