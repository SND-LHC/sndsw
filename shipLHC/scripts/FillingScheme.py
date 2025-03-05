import ROOT,os
import rootUtils as ut
from urllib.request import urlopen
import numpy
import time,calendar
import subprocess
from XRootD import client
import pickle
from rootpyPickler import Pickler
from rootpyPickler import Unpickler
import atexit
import requests
import json
import re

def pyExit():
       print("Make suicide until solution found for freezing")
       os.system('kill '+str(os.getpid()))
atexit.register(pyExit)

fromElog = {4361:7902,4362: 7921, 4363: 7921, 4364: 7921, 4365: 7921, 4366: 7922, 4367: 7923, 4368: 7923, 4369: 7923, 4370: 7924, 4371: 7924, 4372: 7925, 4373: 7926, 4374: 7927, 4375: 7928, 4376: 7929, 4377: 7930, 4378: 7931, 4379: 7931, 4380: 7932, 4381: 7933, 4382: 7934, 4383: 7935, 4384: 7936, 4385: 7937, 4386: 7938, 4387: 7939, 4388: 7940, 4389: 7941, 4390: 7942, 4391: 7943, 4392: 7944, 4393: 7945, 4394: 7946, 4395: 7947, 4396: 7948, 4397: 7949, 4398: 7950, 4399: 7951, 4400: 7952, 4401: 7953, 4402: 7954, 4403: 7955, 4404: 7956, 4405: 7957, 4406: 7958, 4407: 7959, 4408: 7959, 4409: 7960, 4410: 7960, 4411: 7961, 4412: 7961, 4413: 7962, 4414: 7963, 4415: 7963, 4416: 7964, 4418: 7964, 4419: 7965, 4420: 7966, 4421: 7966, 4422: 7967, 4423: 7967, 4424: 7967, 4425: 7967, 4426: 7968, 4427: 7969, 4428: 7969, 4429: 7970, 4430: 7971, 4431: 7971, 4432: 7972, 4433: 7973, 4434: 7974, 4435: 7974, 4436: 7975, 4437: 7976, 4438: 7976, 4440: 7977, 4441: 7977, 4443: 7977, 4446: 7977, 4447: 7977, 4448: 7977, 4449: 7978, 4450: 7979, 4451: 7979, 4452: 7979, 4461: 7979, 4462: 7982, 4463: 7983, 4464: 7984, 4465: 7985, 4466: 7986, 4467: 7987, 4468: 7988, 4469: 7989, 4470: 7990, 4471: 7991, 4472: 7992, 4473: 7993, 4474: 7994, 4475: 7995, 4476: 7996, 4477: 7997, 4478: 7998, 4479: 7999, 4480: 7999, 4481: 8000, 4482: 8001, 4483: 8002, 4484: 8003, 4485: 8004, 4486: 8005, 4487: 8006, 4488: 8007, 4489: 8008, 4493: 8008, 4494: 8008, 4495: 8009, 4496: 8010, 4497: 8010, 4498: 8011, 4499: 8012, 4500: 8013, 4501: 8014, 4503: 8016, 4504: 8017, 4505: 8018, 4506: 8018, 4507: 8019, 4508: 8019, 4509: 8020, 4510: 8021, 4511: 8021, 4512: 8022, 4513: 8022, 4514: 8023, 4515: 8023, 4516: 8024, 4523: 8025, 4524: 8025, 4525: 8025, 4526: 8026, 4527: 8027, 4528: 8028, 4529: 8028, 4530: 8029, 4531: 8029, 4532: 8030, 4533: 8031, 4534: 8032, 4535: 8032, 4536: 8033, 4537: 8033, 4539: 8033, 4540: 8033, 4541: 8033, 4542: 8034, 4543: 8034, 4544: 8034, 4557: 8035, 4558: 8035, 4559: 8036, 4561: 8038, 4562: 8039, 4563: 8040, 4564: 8041, 4568: 8043, 4569: 8044, 4570: 8044, 4571: 8045, 4572: 8046, 4573: 8047, 4574: 8047, 4575: 8047, 4578: 8047, 4579: 8047, 4580: 8050, 4581: 8051, 4582: 8052, 4583: 8052, 4585: 8053, 4586: 8053, 4587: 8054, 4588: 8055, 4589: 8056, 4590: 8056, 4591: 8057, 4592: 8058, 4593: 8058, 4594: 8059, 4595: 8059, 4598: 8061, 4601: 8061, 4602: 8061, 4603: 8061, 4604: 8062, 4606: 8063, 4612: 8063, 4613: 8064, 4614: 8064, 4615: 8065, 4616: 8066, 4617: 8067, 4619: 8068, 4620: 8069, 4621: 8069, 4622: 8070, 4623: 8071, 4624: 8071, 4625: 8072, 4626: 8072, 4627: 8073, 4628: 8073, 4629: 8073, 4630: 8073, 4631: 8073, 4632: 8073, 4635: 8073, 4636: 8074, 4637: 8075, 4638: 8076, 4639: 8076, 4641: 8076, 4642: 8076, 4643: 8076, 4644: 8076, 4645: 8076, 4646: 8076, 4647: 8076, 4648: 8077, 4649: 8078, 4650: 8079, 4653: 8080, 4654: 8081, 4657: 8082, 4658: 8082, 4659: 8082, 4660: 8082, 4662: 8083, 4665: 8083, 4666: 8083, 4667: 8083, 4669: 8084, 4670: 8084, 4672: 8084, 4673: 8084, 4675: 8084, 4676: 8084, 4677: 8084, 4678: 8084, 4679: 8084, 4680: 8084, 4681: 8084, 4682: 8084, 4684: 8084, 4686: 8084, 4687: 8084, 4688: 8084, 4689: 8084, 4690: 8084, 4693: 8084, 4560: 8037, 4661: 8083}

# other remarks
# run 4986 fill 8234 Acc Message :  - 300b fill for VELO insertion - issue with MKI - B2, we ramp with 218b 


emulsionReplacements = {0:1,4573:2,4859:3,5172:4,5485:5,6446:6}   # first runs with new emulsion

class fillingScheme():

   def Init(self,options):
     self.options = options
     self.h={}
     self.path = options.path
     self.content = ''
     self.phaseShift1 =  3564-404  # IP1
     self.phaseShift2 =  129          # IP2
     self.lpcFillingscheme = False
     self.FSdict = {}
     self.LumiInt = {}
     self.runInfo = {}
     self.beamCurrent = {}
     if "FSdict.root" in os.listdir(options.path):
         fg  = ROOT.TFile.Open(options.path+'FSdict.root')
         pkl = Unpickler(fg)
         self.FSdict = pkl.load('FSdict')
         fg.Close()
     if "RunInfodict.root" in os.listdir(options.path):
         fg  = ROOT.TFile.Open(options.path+"RunInfodict.root")
         pkl = Unpickler(fg)
         self.runInfo = pkl.load('runInfo')
         fg.Close()
     if "Lumidict.root" in os.listdir(options.path):
         fg  = ROOT.TFile.Open(options.path+"Lumidict.root")
         pkl = Unpickler(fg)
         self.LumiInt = pkl.load('LumiInt')
         fg.Close()

     self.startTimes = { }
     self.date = {}

   def myPrint(self,tname,oname):
          for t in ['.root','.pdf','.png']:
               self.h[tname].Print(self.options.path+oname+t)

   def readStartTimes(self):
       months = {'Jan.':'01-','Feb.':'02-','Mar.':'03-','Apr.':'04-','May.':'05-','Jun.':'06-','Jul.':'07-','Aug.':'08-','Sep-':'09-','Oct.':'10-','Nov.':'11-','Dec.':'12-'}
       s = open(self.path+'/StartTimes.log')
       L = s.readlines()
       for i in range(len(L)):
            l = L[i]
            if not l.find("(Info,Run)")>0: continue
            offset = 0
            if l.find('PM')>0: offset = 12*3600
            offset -= 2*3600  # stupid time zone
            for x in ['AM','PM']:
                 l = l.replace(x,'')
            tmp = l.split("(Info,Run)")
            tmp[0] = tmp[0].replace(' ','')
            for m in months: tmp[0] = tmp[0].replace(m,months[m])
            time_obj = time.strptime(tmp[0], '%m-%d,%Y-%H:%M:%S')
            r = tmp[1].rfind('Run')
            if r > 4570: continue #not needed, extracted from json file in run directory
            run = int(tmp[1][r+3:].split('Started')[0])
            self.startTimes[run] = calendar.timegm(time_obj)+offset

   def getFillNrFromElog(self):
       # starts with 4362 and ends with 4693
       # from 4663 - 4687 are not valid
       e = open(self.path+'/ELOG DAQ Test.html')

       self.fromElogX = { }
       L = e.readlines()
       for i in range(len(L)):
            l = L[i]
            k = l.find('- New Fill')
            if k<0: continue
            fillNr = int(l[k+10:].split(' ')[1])
            for j in range(i+1,len(L)):
               k = L[j].find('Started with')
               if k>0:
                   m = L[j][:k].rfind('Run')
                   runNr =  int(L[j][m+3:k])
                   self.fromElogX[runNr] = fillNr
               if L[j].find('- New Fill')>0: break

# by hand manipulations:
# 4560, only stops no start
# 4661, only stops no start
       self.fromElogX[4560]=8037
       self.fromElogX[4661]=8083

   def getNameOfFillingscheme(self,fillnr):
        tmp = fillnr
        alternative = self.alternativeFill(str(fillnr))
        if alternative: tmp = alternative
        Y = "2022"
        if fillnr > 8500 : Y = "2023"
        if fillnr >= 9323 : Y="2024"
        if not self.lpcFillingscheme:
             with urlopen('https://lpc.web.cern.ch/cgi-bin/fillTable.py?year='+Y) as webpage:
               self.lpcFillingscheme = webpage.read().decode()
               self.tagi = '<td>XXXX</td>'
               self.tagj = 'fillingSchemes/'+Y+'/candidates/'
               # end of line
               self.tagl = '</td></tr>'
        fs = 0
        i = self.lpcFillingscheme.find(self.tagi.replace('XXXX',str(tmp)))
        if int(Y)>=2024:
          if i>0:
            end_of_line = self.lpcFillingscheme[i:].find(self.tagl)            
            # In rare cases there is a link to download the FS file
            # One needs to skip this link data
            end_link_data = self.lpcFillingscheme[i:i+end_of_line].find('</a>')
            if end_link_data!=-1: 
              end_of_line = self.lpcFillingscheme[i:].find('</a>'+self.tagl)
            line_of_interest = self.lpcFillingscheme[i:i+end_of_line]
            last_separator = line_of_interest.rfind("<td>")
            # Skip download link data, if it exists
            last_separator_link_check = line_of_interest.rfind('.csv">')
            if last_separator_link_check!=-1:
              fs = line_of_interest[last_separator_link_check+6:]
            else:
              fs = line_of_interest[last_separator+4:]
        else:
          if i>0:
            j = self.lpcFillingscheme[i:].find(self.tagj)+i+len(self.tagj)
            if j>0:
               k = self.lpcFillingscheme[j:].find('.csv')
               if k>0:
                 fs = self.lpcFillingscheme[j:j+k]
        return fs
        
   def getFillNrFromRunNr(self,runNumber):
       if runNumber in fromElog:
            return fromElog[runNumber]
       FILL_NUMBER_MASK = 0x000000000000FFFF
       R = ROOT.TFile.Open(os.environ['EOSSHIP']+\
       options.rawData+"/run_"+str(runNumber).zfill(6)+"/data_0000.root")
       try:
         if R.Get('event'):
            rc = R.event.GetEvent(R.event.GetEntries()-1)
            flags = R.event.flags
         else:
            event = R.data
            event.GetEvent(0)
            flags = event.evt_flags
         fillNumber = numpy.bitwise_and(FILL_NUMBER_MASK,flags)
         if fillNumber<1: fillNumber = False
         print('fill number =',fillNumber )
       except:
         fillNumber = False
       if runNumber == 6296 : fillNumber = 8897  # LHC mistake, twice the same fill number
       return fillNumber

   def getLumiAtIP1(self,fillnr=None,fromnxcals=False, fromAtlas=False):
     Y = "2022"
     if fillnr>8500: Y = "2023"
     if fillnr>=9323: Y="2024"
     if not fromnxcals and not fromAtlas:
       try:
          with urlopen('https://lpc.web.cern.ch/cgi-bin/fillAnalysis.py?year='+Y+'&action=fillData&exp=ATLAS&fillnr='+str(fillnr)) as webpage:
              tmp = webpage.read().decode()
       except:
          print('lumi info not avaible from lpc.',fillnr)
          return -1
       exec("self.content = "+tmp)
       """ Each lumi file contains:
            time stab l dl sl dsl
            where
            time =  UNIX time, i.e. in seconds since UTC Jan 1, 1970, 00:00:00, not counting leap seconds.
            stab =  stable beam flag: a float value between 0 and 1 which corresponds to the fraction of time spent in stable beams for this time bin.
            l    =  luminosity in Hz/ub
            dl   =  point-to-point error on luminosity in Hz/ub
            sl   =  specific luminosity in Hz/ub  (see below)
            dsl  =  point-to-point error on specific luminosity in Hz/ub
            self.content['data']['fillData']['data'][0]
            [1660892004.2119756, 1.0, 9907.5947265625, 0.0, 3.2100153151839246e-22, 0.0]
            time.ctime(1660892004.2119756)   -> 'Fri Aug 19 08:53:24 2022'
       """
       fs = self.getNameOfFillingscheme(fillnr)
       self.lumiAtIP1 = {'startTime':self.content['data']['fillData']['data'][0][0],'lumiTime':ROOT.TGraph(),'fillingScheme':fs}
       X = self.content['data']['fillData']['data']
       t0 =  self.lumiAtIP1['startTime']
       for n in range(len(X)):
            self.lumiAtIP1['lumiTime'].AddPoint(X[n][0]-t0,X[n][2])
       return 0

     elif fromnxcals:
       # nxcals old:  /eos/experiment/sndlhc/nxcals_data/
       fileLoc = os.environ['EOSSHIP']+"/eos/experiment/sndlhc/nxcals_data/fill_"+str(fillnr).zfill(6)+".root"
       try:
           fill =  ROOT.TFile.Open(fileLoc)
       except:
           print('Fill information not found in nxcals ',fillnr)
           return -1

       if not fill.LuminosityIP1.Get('ATLAS_OFFLINE_LUMI_TOT_INST'):
           if not fill.LuminosityIP1.Get('ATLAS_LUMI_TOT_INST'):
                print('Lumi information not found in nxcals ',fillnr)
                return -1
           LtreeOff = fill.LuminosityIP1.ATLAS_LUMI_TOT_INST
       else:
           LtreeOff = fill.LuminosityIP1.ATLAS_OFFLINE_LUMI_TOT_INST
# other useful info
       fillScheme = " "
       if fill.LHC.FindObjectAny('LHC_STATS_LHC_INJECTION_SCHEME'):
          rc = fill.LHC.LHC_STATS_LHC_INJECTION_SCHEME.GetEvent(0)
          fillScheme = str(fill.LHC.LHC_STATS_LHC_INJECTION_SCHEME.var)
       else:
          fillScheme = self.getNameOfFillingscheme(fillnr)

       rc = LtreeOff.GetEvent(0)
       self.lumiAtIP1 = {'startTime':LtreeOff.unix_timestamp,'lumiTime':ROOT.TGraph(),'fillingScheme':fillScheme}
       t0 =  self.lumiAtIP1['startTime']
       for e in LtreeOff:
            self.lumiAtIP1['lumiTime'].AddPoint(e.unix_timestamp-t0,e.var)

       return 0
       
     elif fromAtlas:
       fileLoc = os.environ['EOSSHIP']+"/eos/experiment/sndlhc/atlas_lumi/fill_"+str(fillnr).zfill(6)+".root"
       try:
           fill =  ROOT.TFile.Open(fileLoc)
       except:
           print('Fill information not found in atlas_lumi ',fillnr)
           return -1

       LtreeOff = fill.atlas_lumi
       fillScheme = self.getNameOfFillingscheme(fillnr)

       rc = LtreeOff.GetEvent(0)
       self.lumiAtIP1 = {'startTime':LtreeOff.unix_timestamp,'lumiTime':ROOT.TGraph(),'fillingScheme':fillScheme}
       t0 =  self.lumiAtIP1['startTime']
       for e in LtreeOff:
            self.lumiAtIP1['lumiTime'].AddPoint(e.unix_timestamp-t0,e.var)

       return 0
   def drawLumi(self,runNumber):
       R = ROOT.TFile.Open(www+"offline/run"+str(runNumber).zfill(6)+".root")
       ROOT.gROOT.cd()
       bCanvas = R.daq.Get('T')
       Xt = {'time':None,'timeWtDS':None,'timeWt':None}
       for x in Xt:
            Xt[x] = bCanvas.FindObject(x)
            if Xt[x]: self.h[x] = Xt[x].Clone(x)
            else: self.h[x].Reset()
       self.h['c1'].cd()
       ROOT.gROOT.cd()
       self.options.fillNumbers = self.getFillNrFromRunNr(runNumber)
       # for overlay, need SND@LHC startTime to adjust with lumiTime 
       runDir = options.rawData+"/run_"+str(runNumber).zfill(6)
       jname = "run_timestamps.json"
       dirlist  = str( subprocess.check_output("xrdfs "+os.environ['EOSSHIP']+" ls "+runDir,shell=True) )
       startTime=0
       if jname in dirlist:
         with client.File() as f:
            f.open(os.environ['EOSSHIP']+runDir+"/run_timestamps.json")
            status, jsonStr = f.read()
         exec("self.date = "+jsonStr.decode())
         time_str = self.date['start_time'].replace('Z','')
         time_obj = time.strptime(time_str, '%Y-%m-%dT%H:%M:%S')
         self.startTime = calendar.timegm(time_obj)
       else:  # try reading ecs log file
         if len(self.startTimes)==0: self.readStartTimes()
         if runNumber in self.startTimes:
             self.startTime = self.startTimes[runNumber]
         else: return
       self.date['start_time'] = time.ctime(self.startTime)

       for x in Xt:
         if not Xt[x]: continue
         self.h[x+'10'] = self.h[x].Clone(x+'10')
         self.h[x+'10'].Rebin(10)
         self.h[x+'10'].SetMinimum(0)
         self.h[x+'10'].Scale(self.options.postScale/10.)
       if self.date['start_time'].find('Tu')<0 and self.date['start_time'].find('Th')<0:
           tmp = self.date['start_time'].replace('T',' ').replace('Z','')
       else: tmp = self.date['start_time']
       self.h['time10'].SetTitle('Run '+str(runNumber)+'  Fill '+str(self.options.fillNumbers)+'  '+tmp)
# what to do with spikes? 
       mx = self.h['time10'].GetMaximumBin()
       side = self.h['time10'].GetBinContent(mx-1)+self.h['time10'].GetBinContent(mx+1)
       if self.h['time10'].GetBinContent(mx+1)<0.2*self.h['time10'].GetBinContent(mx-1): side = 2*self.h['time10'].GetBinContent(mx-1) # happens at end of fill
       newMx = max(10,0.75*side)
# special runs
       if runNumber==4423: newMx = 100
       if runNumber==4415: newMx = 150
       if runNumber==4362: newMx = 7
       if runNumber==5003: newMx = 4100
       if self.h['time10'].GetBinContent(mx) > side: self.h['time10'].SetMaximum(newMx)
       self.h['time10'].SetMinimum(0)
       self.h['time10'].Draw('hist')
       self.h['timeWt10'].Draw('histsame')
       self.h['timeWtDS10'].Draw('histsame')
       self.h['c1'].Update()
       self.myPrint('c1','noLumi-run'+str(runNumber).zfill(6))

       nbins = self.h['time'].GetNbinsX()
       endTime = self.h['time'].GetBinCenter(nbins)  # in seconds
       rc = self.getLumiAtIP1(fillnr=options.fillNumbers,fromnxcals=True,fromAtlas=False)
       if not rc<0:
          if FS.lumiAtIP1['lumiTime'].GetN()<2:
              rc = self.getLumiAtIP1(fillnr=options.fillNumbers,fromnxcals=False,fromAtlas=True)
       if rc < 0:
           rc = self.getLumiAtIP1(fillnr=options.fillNumbers,fromnxcals=False,fromAtlas=False)
           if rc<0: return

       self.lumiAtlas = ROOT.TGraph()
       deltaT = self.lumiAtIP1['startTime']  - self.startTime # account for timezone/summertime
       self.Lmax = 0
       self.Lint = 0
       self.Lsnd = 0
       tprev = [-1,0]
       for n in range(self.lumiAtIP1['lumiTime'].GetN()):
               t = self.lumiAtIP1['lumiTime'].GetPointX(n) + deltaT
               l = self.lumiAtIP1['lumiTime'].GetPointY(n)
               self.lumiAtlas.AddPoint(t,l)
               if l>self.Lmax: self.Lmax = l
               if tprev[0] < 0: 
                   tprev = [t,l]
               else:
                   dt = t - tprev[0]
                   X =  (tprev[1]+l)/2.
                   self.Lint +=X*dt
                   if t>0 and t<endTime: self.Lsnd += X*dt
                   tprev[0] = t
                   tprev[1] = l
       self.LumiInt[runNumber] = [self.Lint,self.Lsnd]

       if not self.Lmax>0: 
           print('no lumi for run ',runNumber)
           return
# work with second axis
       rightmax = 1.1*self.Lmax/1000
       self.scale = ROOT.gPad.GetUymax()/rightmax
       self.lumiAtlasS = ROOT.TGraph()
       self.lumiAtlasS.SetLineColor(ROOT.kMagenta)
       self.lumiAtlasS.SetLineWidth(3)

       for n in range(self.lumiAtIP1['lumiTime'].GetN()):
             t = self.lumiAtIP1['lumiTime'].GetPointX(n) + deltaT
             l = self.lumiAtIP1['lumiTime'].GetPointY(n)
             self.lumiAtlasS.AddPoint(t,l*self.scale/1000)
       self.lumiAtlasS.Draw('same')
       self.h['ax1'] = ROOT.TGaxis(ROOT.gPad.GetUxmax(), ROOT.gPad.GetUymin(),
                   ROOT.gPad.GetUxmax(), ROOT.gPad.GetUymax(),
                   0, rightmax, 510, "+L")
       l = self.Lsnd/1E9
       ul = 'fb'
       if l < 0.01:
           l = self.Lsnd/1E9
           ul = 'pb'
       self.h['ax1'].SetTitle('L [Hz/nb]   Integral '+"%5.2F %s^{-1}"%(l,ul))
       self.h['ax1'].SetTextFont(42)
       self.h['ax1'].SetLabelFont(42)
       self.h['ax1'].SetTextColor(ROOT.kMagenta)
       self.h['ax1'].Draw()
       self.h['time10'].Draw('histsame')
       self.myPrint('c1','Lumi-run'+str(runNumber).zfill(6))
   def alternativeFill(self,fillNr):
       alternative = None
       if fillNr=='8470': 
                  # 25ns_156b_144_90_96_48bpi_4inj_MD7003 from ELOG
             alternative = '8471'
       if fillNr=='8461': 
                  # 2nominals_10pilots_lossmaps_coll_allIPs from ELOG
             alternative = '8184'
       if fillNr=='8256':
                  # 8256 = 25ns_2461b_2448_1737_1733_180bpi_16inj_1INDIV
             alternative = '8253'
       if fillNr=='8140':
                  # 8140 = 25ns_2413b_2400_1836_1845_240bpi_12inj_1INDIV, same as 8142
             alternative = '8142'
       elif fillNr=='8105':
             # from Cris:  '25ns_2173b_2160_1804_1737_240bpi_11inj_1INDIV'  wrong
             # try:        '25ns_1935b_1922_1758_1842_240bpi_12inj_3INDIV'
             # from elog  : 25ns_1935b_1922_1602_1672_192bpi_14inj_3INDIVs
             alternative = '8106'
       elif fillNr=='8074':
                  # 8073 25ns_1227b_1214_1054_1102_144bpi_14inj
                  # 8076 25ns_1551b_1538_1404_1467_144bpi_16inj
             alternative = '8073'
       elif fillNr=='8070':
                  # 8068 and 8072: 25ns_1227b_1214_1054_1102_144bpi_14inj
             alternative = '8068'
       elif fillNr=='8056':
                  # from Cris: '25ns_987b_974_876_912_96bpi_17inj'
             alternative = '8057'
       elif fillNr=='8045':
                  # 8043 25ns_987b_974_878_917_144bpi_13inj
             alternative = '8043'
       elif fillNr=='8025':
                  # 8023 25ns_603b_590_524_542_48bpi_17inj
                  # 8027 25ns_603b_590_526_547_96bpi_13inj
             alternative = '8023'  # ???
       elif fillNr=='8011':
                  # 8016 rampup	25ns_603b_590_524_542_48bpi_17inj	UFO	true	false	First 600b ramp-up fill
                  # 8007 rampup	25ns_315b_302_237_240_48bpi_11inj	RF	false	false	Third 300b ramp-up fill
             alternative = '8007'  # ???
       elif fillNr=='8388':
                  # 25ns_2462b_2450_1737_1735_180bpi_17inj_2INDIV
             alternative = '8387'  #  
       elif fillNr=='8478':
                  # Single_12b_9_1_3_BSRT_2018_pilot
             alternative = '8479'  #  
       elif fillNr=='8294':
                  # 25ns_315b_302_237_240_48bpi_11inj,
             alternative = '8295'  #?  8293'  #  
       return alternative
       
   def extractFillingScheme(self,fillNr):
        alternative = self.alternativeFill(str(fillNr))
        # Get the FS name from the all-year LPC table
        fs_name_table = self.getNameOfFillingscheme(int(fillNr))
        # this 2024 ion run FS has its attributes swapped btw LPC table and json
        # this is the most elegant fix since many fills are affected
        if fs_name_table=="50ns_119b_58_51_58_56bpi_9inj_3INDIV_4NC_PbPb":
           fs_name_table="50ns_119b_58_51_58_56bpi_9inj_4NC_3INDIV_PbPb"
        print('Name of filling scheme: ',fs_name_table)
        if fs_name_table==0: 
          return -1
        # since 2024 new there is new storage of FS in json files, no csv
        fs_url="https://gitlab.cern.ch/lhc-injection-scheme/injection-schemes/-/raw/master/"+\
                 fs_name_table+".json"
        F = ROOT.TFile(self.path+'fillingScheme-'+fillNr+'.root','recreate')
        nt = ROOT.TNtuple('fill'+fillNr,'b1 IP1 IP2','B1:IP1:IP2:IsB2')
        # Get the 2024 data
        # 9323 is first fillNr for 2024
        if int(fillNr) >= 9323: 
          # Get the JSON file content from the web
          response = requests.get(fs_url)
          # If JSON file is not found, try adding char 's' to FS name
          if response.status_code == 404:
            fs_url="https://gitlab.cern.ch/lhc-injection-scheme/injection-schemes/-/raw/master/"+\
                     fs_name_table+"s.json"
            response = requests.get(fs_url)
          response.raise_for_status()
          fs_data_json = response.json()
          # check if collision data exists - for 2024 ion runs it doesn't
          # and SND files from eos are to be used.
          # These files were generated using the LPC Filling Scheme Editor.
          if 'collsPatternB1' not in fs_data_json:
             with open('/eos/experiment/sndlhc/filling_schemes/2024/ion_run/'+
                         fs_name_table+'_snd_generated.json', 'r') as custom_json:
                fs_data_json = json.load(custom_json)
             print('Using FS generated using LPC Filling Scheme Editor!\nJSON:'\
                    '/eos/experiment/sndlhc/filling_schemes/2024/ion_run/'+
                     fs_name_table+'_snd_generated.json')
          # check name of file and fs_name in file are the same
          if 'schemeName' in fs_data_json:
           fs_name_json = fs_data_json['schemeName']
           if fs_name_json!=fs_name_table:
             print('FS name differs btw the LPC JSON and the all-year LPC table, check!', '\n', \
                    'JSON:',  fillNr, fs_name_json, '\n', \
                    'all-year table:', fs_name_table)
             # Accept if difference is char 's'
             if fs_name_json!=fs_name_table+'s':
               return -1
             
           nB1 = fs_data_json['collsPatternB1']#B1 bucket number,IP1,IP2,IP5,IP8
           nB2 = fs_data_json['collsPatternB2']#B2 bucket number,IP1,IP2,IP5,IP8
           # Sanity checks.
           if not len(nB1)==5 or not len(nB2)==5:
             print("missing collsPattern data in the FS JSON file")	
             return -1

           # Get N colliding bunches from name of the FS json file
           # Convention is {spacing}_{bunches}_{IP1/5}_{IP2}_{IP8}_{trainlength}_{injections}_{special info}
           fs_n_bunches = re.findall(r'(?<=_)\d+(?=_)', fs_name_table)
           n_ip1_in_title = int(fs_n_bunches[0])
           n_ip2_in_title = int(fs_n_bunches[1])
           n_ip8_in_title = int(fs_n_bunches[2])
           
           summary_collisions = {'B1':[], 'B2':[]}
           collsPatterns = {'B1':nB1, 'B2':nB2}
           for beam in collsPatterns.keys():
             for i in range(1,5): 
               summary_collisions[beam].append(numpy.count_nonzero(numpy.array(collsPatterns[beam][i])))
               if i==1 or i==3: #IP1/5
                  if not summary_collisions[beam][i-1]== n_ip1_in_title:
                    print("For the number of IP1/5 bunches for {0} got {1} expected {2}. Check the FS!".format(beam,summary_collisions[beam][i-1],n_ip1_in_title))
               if beam=='B1': # asymmetric collisions in IP2 and IP8
                 if i==2: #IP2
                   if not summary_collisions[beam][i-1]== n_ip2_in_title:
                     print("For the number of IP2 bunches for {0} got {1} expected {2}. Check the FS!".format(beam,summary_collisions[beam][i-1],n_ip2_in_title))
                 if i==4: #IP8
                   if not summary_collisions[beam][i-1]== n_ip8_in_title:
                     print("For the number of IP8 bunches for {0} got {1} expected {2}. Check the FS!".format(beam,summary_collisions[beam][i-1],n_ip8_in_title))
           print("From the json file, we got:\nBeam1:\nIP1 {0}, IP2 {1}, IP5 {2}, IP8 {3}".format(*summary_collisions['B1']))
           print("Beam2:\nIP1 {0}, IP2 {1}, IP5 {2}, IP8 {3}".format(*summary_collisions['B2']))
           
           for i in range(len(nB1[0])):
             if nB1[1][i]==1: # 1 if headon in IP1
                 b1_ip1_id = int(nB1[0][i])
             else: b1_ip1_id = -1
             if nB1[2][i]==1: # 1 if headon in IP2
                b1_ip2_id = int(nB1[0][i])
             else: b1_ip2_id = -1
             rc = nt.Fill(int(nB1[0][i]),b1_ip1_id,b1_ip2_id,0)
           for i in range(len(nB2[0])):
             if nB2[1][i]==1: # 1 if headon in IP1
                b2_ip1_id = int(nB2[0][i])
             else: b2_ip1_id = -1
             if nB2[2][i]==1: # 1 if headon in IP2
                b2_ip2_id = int(nB2[0][i])
             else: b2_ip2_id = -1
             rc = nt.Fill(int(nB2[0][i]),b2_ip1_id, b2_ip2_id,1)
             
        else:
          # 2022-2023 FS storage in csv files
          # cases where only a binary csv file exists or the json file is wrong
          if fillNr in ['9231', '9232']:
            File = urlopen('https://lpc.web.cern.ch/fillingSchemes/2023/candidates/50ns_1123b_1010_1010_320_56bpi_23inj_3INDIV_PbPb.csv')
            X = File.read()
            File.close()
            csv = X.decode().split('\n')
          else:
            fs_url="https://lpc.web.cern.ch/cgi-bin/schemeInfo.py?fill="
            if alternative:
              fs_url=fs_url+alternative+'&fmt=json'
            else:
              fs_url = fs_url+fillNr+'&fmt=json'
            with urlopen(fs_url) as webpage:
              tmp = webpage.read().decode()
              
            exec("self.content = "+tmp)
            if len(self.content['fills']) < 1: 
              print('Filling scheme not yet known',fillNr,self.options.runNumbers)
              return -1
            if alternative:
               self.content['fills'][fillNr] = self.content['fills'][alternative]
            if (self.content['fills'][fillNr]['name'] != fs_name_table ):
               print('FS data differs btw the LPC JSON and the all-year LPC table, check!', '\n', \
                     'JSON:',  fillNr, self.content['fills'][fillNr]['name'], '\n', \
                     'all-year table:', fs_name_table, '\n', \
                     'One can look for the candidates csv files here\n' \
                     "https://lpc.web.cern.ch/fillingSchemes/202X/candidates")
               return -1
            csv = self.content['fills'][fillNr]['csv'].split('\n')
            
          nB1 = csv.index('B1 bucket number,IP1,IP2,IP5,IP8')
          while nB1>0:
            tmp = csv[nB1+1].split(',')
            if len(tmp)!=5: break
            nB1+=1
            rc = nt.Fill(int(tmp[0]),int(tmp[1].replace('-','-1')),int(tmp[2].replace('-','-1')),0)
          nB2 = csv.index('B2 bucket number,IP1,IP2,IP5,IP8')
          while nB2>0:
            tmp = csv[nB2+1].split(',')
            if len(tmp)!=5: break
            nB2+=1
            rc = nt.Fill(int(tmp[0]),int(tmp[1].replace('-','-1')),int(tmp[2].replace('-','-1')),1)

        nt.Write()
        F.Close()
        return 0

   def extractPhaseShift(self,fillNr,runNumber):
         # Check if the offline monitoring file exists
         try:
           R = ROOT.TFile.Open(www+"offline/run"+str(runNumber).zfill(6)+".root")
           ROOT.gROOT.cd()
           try:
             self.h['bnr'] = R.daq.Get('bunchNumber').FindObject('bnr').Clone('bnr')
           except:
             self.h['bnr'] = R.daq.Get('shifter/bunchNumber').FindObject('bnr').Clone('bnr')         
           R.Close()
         # create the bunch number plot if offline monitoring file is missing
         except:
           try:
             self.h['bnr']=self.h['bnr_from_data']
           except:
             self.h['bnr'] = self.BunchNumberPlotFromData(runNumber)         
         Nbunches = self.h['bnr'].GetNbinsX()
#Filling scheme
         self.F = ROOT.TFile(self.path+'fillingScheme-'+fillNr+'.root')
         self.fs = self.F.Get('fill'+fillNr)
# convert to dictionary
         self.FSdict[runNumber] = {"fillNumber":fillNr,'phaseShift1':0,'phaseShift2':0,"B1":{},"B2":{}}
         fsdict = self.FSdict[runNumber]
         for x in self.fs:
              if x.IsB2>0:
                   if Nbunches==3564:
                     fsdict['B2'][(x.B1-1)/10]={'IP1':x.IP1>0,'IP2':x.IP2>0}
                   if Nbunches==1782:
                     fsdict['B2'][(x.B1-1)//20]={'IP1':x.IP1>0,'IP2':x.IP2>0}
              else:
                   if Nbunches==3564:
                     fsdict['B1'][(x.B1-1)/10]={'IP1':x.IP1>0,'IP2':x.IP2>0}
                   if Nbunches==1782:
                     fsdict['B1'][(x.B1-1)//20]={'IP1':x.IP1>0,'IP2':x.IP2>0}
         self.matches = {}
         for phase1 in range(0,Nbunches):
               self.matches[phase1]=0
               for n in range(0,Nbunches):
                   if not n in fsdict['B1']: continue
                   j = (n+phase1)%Nbunches + 1
                   if fsdict['B1'][n]['IP1']: self.matches[phase1]+=self.h['bnr'].GetBinContent(j)
         self.phaseShift1 = max(self.matches,key=self.matches.get)
         print('phaseShift1 found:',self.phaseShift1,Nbunches-self.phaseShift1)
         self.matches = {}
         if Nbunches == 1782: self.phaseShift2 = Nbunches-64
         else:
           for phase2 in range(0,Nbunches):
               self.matches[phase2]=0
               for n in range(0,Nbunches):
                   if not n in fsdict['B2']:
                       continue
                   # calculate the bunch number of SND events
                   j = (n+self.phaseShift1+phase2)%Nbunches + 1    # bin number
                   # adjust it with the found B1 phase to determine(and exclude)
                   # bunches associated with IP1 collisions
                   ip1 = (j-1+Nbunches-self.phaseShift1)%Nbunches
                   if ip1 in fsdict['B1']:
                       if fsdict['B1'][ip1]['IP1']: continue
                   if fsdict['B2'][n]['IP2'] or 1>0: 
                      self.matches[phase2]+=self.h['bnr'].GetBinContent(j)
           self.phaseShift2 = max(self.matches,key=self.matches.get)
           print('phaseShift2 found:',self.phaseShift2,Nbunches-self.phaseShift2)
           if not (Nbunches-self.phaseShift2) == 129:
              print('There is a problem with phaseshift2 for run',runNumber,Nbunches-self.phaseShift2)
              if self.phaseShift2 == 129:
                     print("!!! Probably beam 1 and beam 2 are interchanged. Try reverse")
                     self.phaseShift1 = FS.phaseShift1 +129
           if fillNr=="8178":
                print('special LHCf run. Phaseshift determined by hand: 430-1017+3564')
                self.phaseShift1 = 430-1017+3564
           if fillNr=="8056":
                print('run with very low lumi at IP1, beam 2 background dominates')
                self.phaseShift1 = 3564-1732+129
           if fillNr=="8056":
                print('run with very low lumi at IP1, beam 2 background dominates')
                self.phaseShift1 = 3564-1603
           if fillNr=="8140" or fillNr=="8070" or fillNr=="8045":
                print('run with very low lumi at IP1, beam 2 background dominates')
                self.phaseShift1 = 3564-1456
           if fillNr == "8383":
                print('run with very low lumi at IP1, fit does not converge correctly')
                self.phaseShift1 = 1978 + 130
           if fillNr == "8342":
                print('run with very low lumi at IP1, beam 2 background dominates')
                self.phaseShift1 = 2107
           if fillNr == "8256":
                print('run with very low lumi at IP1, beam 2 background dominates')
                self.phaseShift1 = 2108
           if fillNr == "8294":
                print('run with very low lumi at IP1, beam 2 background dominates')
                self.phaseShift1 =  2598 +129
           # force 129 for proton beams
           # It is determined using ~480m distance to IP1 and 25-ns distance between bunches:
           # 482.5m/speed of light/bunch distance = 64.33 
           # This is multiplied by 2 to arrive at 129.
           self.phaseShift2 = Nbunches - 129
         print('phaseShift2 is set to:',self.phaseShift2,Nbunches-self.phaseShift2)
         fsdict['phaseShift2'] = self.phaseShift2
         fsdict['phaseShift1'] = self.phaseShift1

   def plotBunchStructure(self,fillNr,runNumber):
         h=self.h
         self.F = ROOT.TFile(self.path+'fillingScheme-'+fillNr+'.root')
         self.fs = self.F.Get('fill'+fillNr)
         
         try:
           R = ROOT.TFile.Open(www+"offline/run"+str(runNumber).zfill(6)+".root")
           ROOT.gROOT.cd()
           bCanvas = R.daq.Get('bunchNumber')
           if not bCanvas:
             bCanvas = R.daq.shifter.Get('bunchNumber')
           h['bnr']= bCanvas.FindObject('bnr').Clone('bnr')           
         # create the bunch number plot if offline monitoring file is missing
         except:
            try:
               h['bnr'] = h["bnr_from_data"]
            except:
               h['bnr'] = self.BunchNumberPlotFromData(runNumber)
         Nbunches = h['bnr'].GetNbinsX()
         ROOT.gROOT.cd()
         
         ut.bookHist(h,'b1','b1',35640,-0.5,35639.5)
         ut.bookHist(h,'IP1','IP1',35640,-0.5,35639.5)
         ut.bookHist(h,'IP2','IP2',35640,-0.5,35639.5)
         ut.bookHist(h,'b1z','b1',Nbunches,-0.5,Nbunches-0.5)
         ut.bookHist(h,'b2z','b2',Nbunches,-0.5,Nbunches-0.5)
         ut.bookHist(h,'IP1z','IP1',Nbunches,-0.5,Nbunches-0.5)
         ut.bookHist(h,'IP2z','IP2',Nbunches,-0.5,Nbunches-0.5)
         h['b1'].Draw()
         h['b1z'].SetLineColor(ROOT.kBlue)
         h['b2z'].SetLineColor(ROOT.kCyan)
         h['IP1z'].SetLineColor(ROOT.kRed)
         h['IP2z'].SetLineColor(ROOT.kOrange)
         h['b1z'].SetStats(0)
         h['b2z'].SetStats(0)
         h['IP1z'].SetStats(0)
         h['IP2z'].SetStats(0)

         self.Draw()

   def Draw(self):
         h = self.h
         Nbunches = h['bnr'].GetNbinsX()
         eq = ''
         if Nbunches==1782: 
           equation_str ='floor((B1-1)/20)'
         if Nbunches==3564: 
           equation_str = '(B1-1)/10'
         h['c1'].cd()
         self.fs.Draw('(( '+equation_str+'+'+str(self.phaseShift1)+')%'+str(Nbunches)+')>>b1z','!(IsB2>0)','hist')
         self.fs.Draw('(( '+equation_str+'+'+str(self.phaseShift1)+')%'+str(Nbunches)+')>>IP1z','IP1>-0.6&&(!(IsB2>0))','hist')
         self.fs.Draw('(( '+equation_str+'+'+str(self.phaseShift1+ self.phaseShift2)+')%'+str(Nbunches)+')>>IP2z','IP2>-0.6&&IsB2>0','hist')
         self.fs.Draw('(( '+equation_str+'+'+str(self.phaseShift1+ self.phaseShift2)+')%'+str(Nbunches)+')>>b2z','IsB2>0','hist')
         norm = h['bnr'].GetBinContent(h['bnr'].GetMaximumBin())
         h['b1z'].Scale(norm*1.5)
         h['IP1z'].Scale(norm*1.0)
         h['b2z'].Scale(norm*0.5)
         h['IP2z'].Scale(norm*0.3)
         h['bnr'].SetStats(0)
         h['bnr'].SetFillColor(17)
         h['bnr'].SetLineColor(ROOT.kBlack)
         h['b1z'].Draw('hist')
         h['bnr'].Draw('histsame')
         h['b1z'].Draw('histsame')
         txt = 'phase shift B1, B2: '+str(Nbunches-self.phaseShift1)+','+str(Nbunches-self.phaseShift2)+' for run '+str(options.runNumbers)
         txt += " fill nr "+options.fillNumbers
         h['b1z'].SetTitle(txt)
         if self.options.withIP2: 
               h['IP2z'].Draw('histsame')
               h['b2z'].Draw('histsame')
         h['IP1z'].Draw('histsame')
         h['bnr'].Draw('histsame')

   def Xbunch(self):
         h = self.h
         ut.bookHist(h,'Xb1z','b1',3564*4,-0.5,3564*4-0.5)
         ut.bookHist(h,'Xb2z','b2',3564*4,-0.5,3564*4-0.5)
         ut.bookHist(h,'XIP1z','IP1',3564*4,-0.5,3564*4-0.5)
         ut.bookHist(h,'XIP2z','IP2',3564*4,-0.5,3564*4-0.5)
         h['Xb1z'].SetLineColor(ROOT.kBlue)
         h['Xb2z'].SetLineColor(ROOT.kCyan)
         h['XIP1z'].SetLineColor(ROOT.kRed)
         h['XIP2z'].SetLineColor(ROOT.kOrange)
         h['Xb1z'].SetStats(0)
         h['Xb2z'].SetStats(0)
         h['XIP1z'].SetStats(0)
         h['XIP2z'].SetStats(0)
         R = ROOT.TFile.Open(www+"offline/run"+str(runNumber).zfill(6)+".root")
         ROOT.gROOT.cd()
         bCanvas = R.daq.Get('sndclock')
         h['Xbnr']= bCanvas.FindObject('Xbnr').Clone('Xbnr')
         ROOT.gROOT.cd()
         h['c1'].cd()
         for j in range(4):
            self.fs.Draw('(( (B1-1)/2.5+'+str(self.phaseShift1*4-2)+')%(3564*4))+'+str(j)+'>>+Xb1z','!(IsB2>0)','hist')
            self.fs.Draw('(( (B1-1)/2.5+'+str(self.phaseShift1*4-2)+')%(3564*4))+'+str(j)+'>>+XIP1z','IP1>-0.6&&(!(IsB2>0))','hist')
            self.fs.Draw('(( (B1-1)/2.5+'+str(self.phaseShift1*4-2+ self.phaseShift2*4)+')%(3564*4))+'+str(j)+'>>+XIP2z','IP2>-0.6&&IsB2>0','hist')
            self.fs.Draw('(( (B1-1)/2.5+'+str(self.phaseShift1*4-2+ self.phaseShift2*4)+')%(3564*4))+'+str(j)+'>>+Xb2z','IsB2>0','hist')
         norm = h['Xbnr'].GetBinContent(h['Xbnr'].GetMaximumBin())
         h['Xb1z'].Scale(norm*1.5)
         h['XIP1z'].Scale(norm*1.0)
         h['Xb2z'].Scale(norm*0.5)
         h['XIP2z'].Scale(norm*0.3)
         h['Xbnr'].SetStats(0)
         h['Xbnr'].SetFillColor(17)
         h['Xbnr'].SetLineColor(ROOT.kBlack)
         h['Xb1z'].Draw('hist')
         h['Xbnr'].Draw('histsame')
         h['Xb1z'].Draw('histsame')
         Nbunches = h['bnr'].GetNbinsX()
         txt = 'phase shift B1, B2: '+str(Nbunches-self.phaseShift1)+','+str(Nbunches-self.phaseShift2)+' for run '+str(options.runNumbers)
         txt += " fill nr "+options.fillNumbers
         h['Xb1z'].SetTitle(txt)
         h['XIP2z'].Draw('histsame')
         h['Xb2z'].Draw('histsame')
         h['XIP1z'].Draw('histsame')
         # overlay all snd subcycles
         ut.bookHist(h,'scycle','overlay',4,-0.5,3.5)
         if Nbunches == 3564: div=4
         if Nbunches == 1782: div=8
         for n in range(h['XIP1z'].GetNbinsX()):
            if h['XIP1z'].GetBinContent(n+1)>0:
              rc = h['scycle'].Fill(n%div, h['Xbnr'].GetBinContent(n+1))

   def Extract(self):
        if self.options.fillNumbers=='':
           fillNumber = self.getFillNrFromRunNr(int(options.runNumbers))
           if not fillNumber: 
               print('Fill number not found')
           else:
               rc = self.extractFillingScheme(str(fillNumber))
               if not rc<0:
                 self.options.fillNumbers = str(fillNumber)
                 self.extractPhaseShift(self.options.fillNumbers,int(self.options.runNumbers))
                 r = int(self.options.runNumbers)
                 self.plotBunchStructure(self.options.fillNumbers,r)
                 self.myPrint('c1','FS-run'+str(r).zfill(6))
                 # add the FS to the file without running all other modules
                 self.merge()
                 self.storeDict(self.FSdict,'FSdict','FSdict') 
       
        else:
           for r in options.fillNumbers.split(','):
              self.extractFillingScheme(r)

   def test(self,runnr,I=True):
       h = self.h
       Nbunches = h['bnr'].GetNbinsX()
       p = open("FSdict.pkl",'rb')
       self.FSdict = pickle.load(p)
       if runnr in self.FSdict: fsdict = self.FSdict[runnr]
       else: 
            print('run number not known. Exit.')
            return()
       options.runNumbers = str(runnr)
       fillNr = fsdict['fillNumber']
       if I:
          f=open('fill'+str(fillNr)+'.csv')
          X = f.readlines()
          keys = {'A6R4.B1':0,'A6R4.B2':0,'B6R4.B1':0,'B6R4.B2':0}
          for k in keys:
             n=0
             for l in X:
                if l.find(k)>0: keys[k]=n
                n+=1
          print(keys)
          A = X[keys['A6R4.B2']//2].replace('\n','').split(',')
          print(len(A))
          for bunchNumber in range(1,Nbunches+1):
             b1 = (bunchNumber-1) in fsdict['B1']
             if b1 and float(A[bunchNumber])>1: continue
             if not b1 and float(A[bunchNumber])<1: continue
             print(bunchNumber,A[bunchNumber],b1)
          return
       FS.phaseShift1 = fsdict['phaseShift1']
       self.plotBunchStructure(fsdict['fillNumber'],runnr)
       w = {}
       for x in ['b1z','IP1z','b2z','IP2z']:
          w[x]=h[x].GetMaximum()
          h[x].Reset()
       for bunchNumber in range(0,Nbunches):
             nb1 = ( Nbunches+bunchNumber - fsdict['phaseShift1'])%Nbunches
             if nb1 in fsdict['B1']:
                 rc = h['b1z'].Fill(bunchNumber,w['b1z'])
                 if fsdict['B1'][nb1]['IP1']: rc = h['IP1z'].Fill(bunchNumber,w['IP1z'])
             nb2 = ( Nbunches + bunchNumber - fsdict['phaseShift1'] - fsdict['phaseShift2'])%Nbunches
             if nb2 in fsdict['B2']:
                 rc = h['b2z'].Fill(bunchNumber,w['b2z'])
                 if fsdict['B2'][nb2]['IP2']: rc = h['IP2z'].Fill(bunchNumber,w['IP2z'])

   def b1b2(self,runnr,b):
     Nbunches = self.h['bnr'].GetNbinsX()
     if runnr in self.FSdict: 
            fsdict = self.FSdict[runnr]
            nb1 = ( Nbunches + b - fsdict['phaseShift1'])%Nbunches
            nb2 = ( Nbunches + b - fsdict['phaseShift1'] - fsdict['phaseShift2'])%Nbunches
            print('b1 bunch number',nb1,nb2)

   def calcMu(self):
       sigma = 80E6 # 80mb
       Nbunches = self.h['bnr'].GetNbinsX()
       self.L = ROOT.TFile.Open("Lumi.root")
       L = self.L
       h = self.h
       runInfo = self.runInfo
       h['muAv']={}
       for k in L.GetListOfKeys():
           runNumber = k.GetName()[3:]
           tc = L.Get(k.GetName())
           h[runNumber+'_Mu']=ROOT.TGraph()
       # need to know the number of colliding bunches. Take from filling scheme
           fs = runInfo[int(runNumber)]['FillingScheme']
           if fs==' ' or fs=='' or fs==0: 
              print('filling scheme not in runinfo, try ',runNumber)
              fillnr = runInfo[int(runNumber)]['Fillnumber']
              fs = self.getNameOfFillingscheme(fillnr)
              if fs==' ' or fs=='' or fs==0: 
                            print('filling scheme not found')
                            continue
              else:print('filling scheme found!')
              runInfo[int(runNumber)]['FillingScheme'] = fs
           tag = 2
           if fs.find('Multi')==0: tag=3
           IP1 = int(fs.split('_')[tag])
           collPerTurn = IP1/Nbunches
           i=-1
           for x in tc.GetListOfPrimitives():
                i+=1
                if x.ClassName() == "TGraph":  gn = i
                if x.ClassName() == "TGaxis":  an = i
                if x.ClassName() == "TFrame":  fn = i
                if x.GetName().find("timeWt")==0:
                    h[x.GetName()] = x.Clone(x.GetName())
                    name = runNumber+'_'+x.GetName()+'_Mu'
                    h[name] = x.Clone(name)
           g = tc.GetListOfPrimitives()[gn]
           axis = tc.GetListOfPrimitives()[an]
           frame = tc.GetListOfPrimitives()[fn]
           scale = (axis.GetWmax()-axis.GetWmin())/(frame.GetY2()-frame.GetY1())
           MuMax = 0
           for n in range(g.GetN()):
               mu = g.GetPointY(n) * scale * sigma / collPerTurn * 25E-9
               h[runNumber+'_Mu'].AddPoint(g.GetPointX(n),mu)
               if mu>MuMax: MuMax=mu
           for t in ['timeWt10','timeWtDS10']:
               hmu = runNumber+'_'+t+'_Mu'
               for i in range(1,h[t].GetNbinsX()+1):
                   h[hmu].SetBinContent(i,h[t].GetBinContent(i)/collPerTurn * 25E-9)
                   h[hmu].SetBinError(i,h[t].GetBinError(i)/collPerTurn * 25E-9)
# find mean values start and end time = 10% of max
           for n in range(h[runNumber+'_Mu'].GetN()):
               if h[runNumber+'_Mu'].GetPointY(n)>0.1*MuMax:
                  startT = n
                  break
           for n in range(h[runNumber+'_Mu'].GetN(),0,-1):
               if h[runNumber+'_Mu'].GetPointY(n)>0.1*MuMax:
                  endT = n
                  break
           rc = h[runNumber+'_Mu'].Fit('pol0','SQ','',h[runNumber+'_Mu'].GetPointX(startT),h[runNumber+'_Mu'].GetPointX(endT))
           res = rc.Get()
           if not res: print('calcMu: something went wrong',runNumber)
           muAv = res.Parameter(0)
           h['muAv'][runNumber] = {'':muAv,'Scifi':0,'DS':0}
           for t in ['timeWt10','timeWtDS10']:
             hmu = runNumber+'_'+t+'_Mu'
             if h[hmu].GetSumOfWeights()==0:continue
             for n in range(1,h[hmu].GetNbinsX()):
               if h[hmu].GetBinContent(n)>0.1*h[hmu].GetMaximum():
                  startT = h[hmu].GetBinCenter(n)
                  break
             for n in range(h[hmu].GetNbinsX(),1,-1):
               if h[hmu].GetBinContent(n)>0.1*h[hmu].GetMaximum():
                  endT = h[hmu].GetBinCenter(n)
                  break
             rc = h[hmu].Fit('pol0','SQ','',startT,endT)
             res = rc.Get()
             if not res: print('calcMu '+t+': something went wrong',runNumber)
             muAv = res.Parameter(0)
             tag ='Scifi'
             if t.find('DS')>0: tag='DS'
             h['muAv'][runNumber][tag] = muAv
       fout = ROOT.TFile('Mu.root','recreate')
       for x in h:
          if x.find('Mu')>0: h[x].Write()
       fout.Close()
# update runInfo
       for runNumber in h['muAv']:
          r = int(runNumber)
          if not r in runInfo:
              print('calcMu: run not in runInfo',r)
              continue
          runInfo[r]['muAv'] = {'':h['muAv'][runNumber][''],'Scifi':h['muAv'][runNumber]['Scifi'],'DS':h['muAv'][runNumber]['DS']}
       
   def FwBw(self,runNumber):
       options.runNumbers = runNumber
       options.fillNumbers = self.runInfo[options.runNumbers]['Fillnumber']
# analyze run for forward / backward tracks per bunch type
       h = self.h
       self.F = ROOT.TFile.Open(www+"offline/run"+str(runNumber).zfill(6)+".root")
       self.B = ROOT.TFile.Open(www+"offline/BunchStructure.root")
       self.L = ROOT.TFile.Open(www+"offline/Lumi.root")
       xing = {'all':False,'B1only':False,'B2noB1':False,'noBeam':False}
                
       for T in ['Txing','TD','T']:
          h[T] = self.F.daq.Get(T).Clone(T)
       for X in ['time','timeWt','timeWtDS']:
            h[X] = h['T'].FindObject(X).Clone(X)
       for t in ['time','timeWt','timeWtDS','bnr']:
         for x in xing:
           if x=='all': continue
           X = t+x
           h[X] = h['Txing'].FindObject(X).Clone(X)
       for x in xing:
           X = 'trackDir'+x
           h[X] = h['TD'].FindObject(X).Clone(X)
           h[X].SetTitle('Track Velocity; 1/v - 1/c  [ns/cm]')
           h[X].SetName('Track Velocity '+x)
       h['BunchStructure'] = self.B.Get("run"+str(runNumber).zfill(6)).Clone("run"+str(runNumber).zfill(6))
       h['Lumi'] = self.L.Get("run"+str(runNumber).zfill(6)).Clone("run"+str(runNumber).zfill(6))
       for x in h['Lumi'].GetListOfPrimitives(): 
         if x.ClassName() == "TGraph": break
       h['LumiT'] = x.Clone()
       Lmax = 0
       for n in range(h['LumiT'].GetN()):
               l = h['LumiT'].GetPointY(n)
               if l>Lmax: Lmax = l

       for x in ['IP1z','b1z','b2z']:      
             h[x] = h['BunchStructure'].FindObject(x)
       ROOT.gROOT.cd()
       ut.bookCanvas(h,'dirRes','',1600,900,2,1)
       tc = h['dirRes'].cd(1)
       rc = h['trackDirB2noB1'].Fit('gaus','QS')
       rc = h['trackDirB1only'].Fit('gaus','QS')
       h['trackDirB2noB1'].Draw()
       tc.Update()
       stats = h['trackDirB2noB1'].FindObject('stats')
       stats.SetOptFit(1111111)
       stats.SetX1NDC(0.15)
       stats.SetY1NDC(0.5)
       stats.SetX2NDC(0.52)
       stats.SetY2NDC(0.86)
       tc.Update()
       tc = h['dirRes'].cd(2)
       h['trackDirB1only'].Draw()
       tc.Update()
       stats = h['trackDirB1only'].FindObject('stats')
       stats.SetOptFit(1111111)
       stats.SetX1NDC(0.15)
       stats.SetY1NDC(0.5)
       stats.SetX2NDC(0.52)
       stats.SetY2NDC(0.86)
       tc.Update()
       h['dirRes'].Update()
       self.myPrint('dirRes','dirRes-'+str(runNumber).zfill(6))

# stats:
       norm = {}
       for x in xing:
             if x=='all': continue
             y = h['bnr'+x]
             norm[x]=0
             for i in range(y.GetNbinsX()+1):
                 if y.GetBinContent(i)>0: norm[x]+=1
       self.stats = {}
       self.statsPerBunch = {}
       bunches = {}
       for x in ['IP1z','b1z','b2z']:
         bunches[x]=0
         for b in range(h[x].GetNbinsX()):
             if h[x].GetBinContent(b+1)>0: bunches[x]+=1
       print(bunches)
       print(norm)
       txt = {'timeWt':'scifi tracks','timeWtDS':'DS tracks'}
       self.frac = {}
       for t in txt:
           self.stats[t]={}
           self.statsPerBunch[t] = {}
           for x in xing:
              if x=='all': continue
              self.stats[t][x]  =  h[t+x].GetSumOfWeights()
           c = 'noBeam'
           self.statsPerBunch[t][c] = self.stats[t][c]/norm[c]
           c = 'B2noB1'
           self.statsPerBunch[t][c] = (self.stats[t][c] - self.statsPerBunch[t]['noBeam']*norm[c])/norm[c]
           c = 'B1only'
           self.statsPerBunch[t][c] = (self.stats[t][c] - self.statsPerBunch[t]['noBeam']*norm[c])/norm[c]
           print('events with '+t+'/bunch, noBeam subtracted for run ',runNumber)
           print('B1 = %5.4F'%(self.statsPerBunch[t]['B1only']))
           print('B2 = %5.4F'%(self.statsPerBunch[t]['B2noB1']))
           NIP1 = h[t].GetSumOfWeights() - self.stats[t]['noBeam'] - self.stats[t]['B1only'] - self.stats[t]['B2noB1']
           self.frac[t] = {}
           for c in xing:
                 if c=='all': continue
                 self.frac[t][c] = self.statsPerBunch[t][c]*bunches['IP1z']/NIP1

           print(txt[t]+' expected B1, B2, noBeam events in IP1: %5.2F%%    %5.2F%%    %5.2F%%'%(
                 self.frac[t]['B1only']*100,self.frac[t]['B2noB1']*100,self.frac[t]['noBeam']*100))

# plot of B2, B1 and no beam with lumi
       self.addBunchCurrent(options.fillNumbers,b=2)
       self.addBunchCurrent(options.fillNumbers,b=1)
       rebin = {'B2noB1':100,'B1only':1000,'noBeam':1000}
       rescale = {'B2noB1':[100,20],'B1only':[5,5],'noBeam':[100,100]}
       for B in ['B2noB1','B1only','noBeam']:
        ut.bookCanvas(h,B,'',1200,900,1,1)
        h[B].cd()
        for j in ['time'+B,'timeWtDS'+B,'timeWt'+B]:
          h[j+'_100'] = h[j].Clone()
          h[j+'_100'].Rebin(rebin[B])
          h[j+'_100'].Scale(1/(rebin[B]*norm[B]))

        h['time'+B+'_100'].SetTitle(';[s];events/s per bunch')
        h['time'+B+'_100'].Draw()
        h['timeWt'+B+'_100'].Scale(rescale[B][0])
        h['timeWtDS'+B+'_100'].Scale(rescale[B][1])
        h['timeWtDS'+B+'_100'].Draw('same')
        h['timeWt'+B+'_100'].Draw('same')
        h[B].Update()
# work with second axis
        rightmax = 1.1*Lmax/1000
        scale = ROOT.gPad.GetUymax()/rightmax
        h['LumiT'+B] = h['LumiT'].Clone('LumiT'+B)
        h['LumiT'+B].Scale(scale/1000)
        h['LumiT'+B].Draw('same')
        h['ax1'+B] = ROOT.TGaxis(ROOT.gPad.GetUxmax(), ROOT.gPad.GetUymin(),
                   ROOT.gPad.GetUxmax(), ROOT.gPad.GetUymax(),
                   0, rightmax, 510, "+L")
        h['ax1'+B].SetTitle('L [Hz/nb]    ')
        h['ax1'+B].SetTextFont(42)
        h['ax1'+B].SetLabelFont(42)
        h['ax1'+B].SetTextColor(ROOT.kMagenta)
        h['ax1'+B].Draw()
        h['l'+B]=ROOT.TLegend(0.44,0.86,0.88,0.98)
        h['l'+B].AddEntry(h['timeB2noB1_100'],'triggered event rate ',"PL")
        h['l'+B].AddEntry(h['timeWtB2noB1_100'],'event rate#times'+str(rescale[B][0])+' with Scifi tracks',"PL")
        h['l'+B].AddEntry(h['timeWtDSB2noB1_100'],'event rate#times'+str(rescale[B][1])+' with DS tracks',"PL")
        h['l'+B].AddEntry(h['LumiT'],'IP1 instanteous luminosity',"PL")
        h['l'+B].Draw()
        h[B].Update()
        self.myPrint(B,B+'-'+str(runNumber).zfill(6))
        beam = -1
        if B=='B2noB1': beam=2
        elif B=='B1only': beam=1
        if beam>0:
           cur = 'b'+str(beam)+'A'
           scale = 0.8*h['time'+B+'_100'].GetMaximum()/self.beamCurrent[cur][0]
           self.beamCurrent[cur][1].Scale(scale)
           self.beamCurrent[cur][1].SetLineColor(ROOT.kRed)
           self.beamCurrent[cur][1].SetLineWidth=2
           self.beamCurrent[cur][1].Draw('same')
           cur = 'b'+str(beam)+'B'
           self.beamCurrent[cur][1].Scale(scale)
           self.beamCurrent[cur][1].SetLineColor(ROOT.kBlue)
           self.beamCurrent[cur][1].SetLineWidth=2
           self.beamCurrent[cur][1].Draw('same')
           self.myPrint(B,B+'-'+str(runNumber).zfill(6)+'_withCurrent')

# plot track direction / velocity for different xings normalized
       for B in ['B2noB1','B1only','noBeam']:
           h['trackDir'+B+'_norm'] = h['trackDir'+B].Clone('trackDir'+B+'_norm')
       h['trackDirB2noB1_norm'].Scale(bunches['b1z']/norm['B2noB1'])
       h['trackDirB1only_norm'].Scale(bunches['b1z']/norm['B1only'])
       ut.bookCanvas(h,'V','',1200,900,1,1)
       tc = h['V'].cd()
       tc.SetLogy(1)
       h['trackDirall'].Draw()
       h['trackDirall'].SetStats(0)
       self.myPrint('V','trackDirAll-'+str(runNumber).zfill(6))
       h['trackDirB2noB1_norm'].SetLineColor(ROOT.kCyan)
       h['trackDirB1only_norm'].SetLineColor(ROOT.kBlue)
       h['trackDirnoBeam_norm'].SetLineColor(ROOT.kGreen)
       h['trackDirB2noB1_norm'].Draw('Histsame')
       h['trackDirB1only_norm'].Draw('Histsame')
        # h['trackDirnoBeam_norm'].Draw('same')
       self.myPrint('V','trackDirXing-'+str(runNumber).zfill(6))
# plot track angles and positions
       ut.bookCanvas(h,'2dslopes','',1200,1200,1,1)
       ut.bookCanvas(h,'1dslopes','',1200,900,1,1)
       for B in ['B2noB1','B1only','noBeam']:
         for x in ['scifi-trackDir','scifi-TtrackPos']:
           T = x+B
           tmp = self.F.scifi.Get(T)
           if not tmp:
              tmp = self.F.scifi.Get(B+'/'+T)
           h[T] = tmp.Clone(T)
           if x.find('Dir')>0:
            for y in ['scifi-trackSlopesXL','slopeXL','slopeYL']:
              h[y+B] = self.h[T].FindObject(y+B).Clone(y+B)
              h[y+B].SetStats(0)
              if y.find('track')>0:
                 cv = '2dslopes'
                 h[cv].cd()
                 txt = y+B
                 h[y+B].Draw('colz')
              else: 
                 txt = 'scifi-'+y+B
                 cv = '1dslopes'
                 h[cv].cd()
                 h[y+B].Rebin(4)
                 h[y+B].Draw()
              self.myPrint(cv,txt+'-'+str(runNumber).zfill(6))
           elif x.find('Pos')>0:
              cv = '2dslopes'
              h[cv].cd()
              y = 'scifi-trackPos'
              h[y+B] = self.h[T].FindObject(y+B).Clone(y+B)
              h[y+B].SetStats(0)
              h[y+B].Draw('colz')
              self.myPrint(cv,y+B+'-'+str(runNumber).zfill(6))

         for x in ['muonDSTracks','mufi-TtrackPos']:
           T = x+B
           tmp = self.F.mufilter.Get(T)
           if not tmp:
              tmp = self.F.mufilter.Get(B+'/'+T)
           h[T] = tmp.Clone(T)
           if x.find('DSTrack')>0:
            for y in ['mufi-slopes','slopeX','slopeY']:
              h[y+B] = self.h[T].FindObject(y+B).Clone(y+B)
              h[y+B].SetStats(0)
              if not y.find('mufi')<0:
                 cv = '2dslopes'
                 h[cv].cd()
                 txt = y+B
                 h[y+B].Draw('colz')
              else: 
                 cv = '1dslopes'
                 h[cv].cd()
                 txt = 'mufi-'+y+B
                 h[y+B].Draw()
              self.myPrint(cv,txt+'-'+str(runNumber).zfill(6))
           elif x.find('Pos')>0:
              y = 'mufi-trackPos'
              cv = '2dslopes'
              h[cv].cd()
              h[y+B] = self.h[T].FindObject(y+B).Clone(y+B)
              h[y+B].SetStats(0)
              h[y+B].Draw('colz')
              self.myPrint(cv,y+B+'-'+str(runNumber).zfill(6))


   def addBunchCurrent(self,fillNr,b=2):
     F = ROOT.TFile.Open('root://eospublic.cern.ch//eos/experiment/sndlhc/nxcals_data/fill_'+str(fillNr).zfill(6)+'.root')
     LHC = F.LHC
     injection_scheme = LHC.LHC_STATS_LHC_INJECTION_SCHEME
     rc = injection_scheme.GetEvent(0)
     print(injection_scheme.var)
     betastar = LHC.HX_BETASTAR_IP1
     rc = betastar.GetEvent(0)
     print('beta* = ',betastar.var)
     beam = {}
     beam['b1A'] = LHC.LHC_BCTFR_A6R4_B1_BEAM_INTENSITY
     beam['b2A'] = LHC.LHC_BCTFR_A6R4_B2_BEAM_INTENSITY
     beam['b1B'] = LHC.LHC_BCTFR_B6R4_B1_BEAM_INTENSITY
     beam['b2B'] = LHC.LHC_BCTFR_B6R4_B2_BEAM_INTENSITY
     t0 =  self.runInfo[options.runNumbers]['StartTime']
     X = 'b'+str(b)+'A'
     self.beamCurrent[X] = [0,ROOT.TGraph()]
     mx = 0
     for e in beam[X]:
            self.beamCurrent[X][1].AddPoint(e.unix_timestamp-t0,e.var)
            if e.var > mx: mx=e.var
     self.beamCurrent[X][0] = mx

     X = 'b'+str(b)+'B'
     self.beamCurrent[X] = [0,ROOT.TGraph()]
     mx = 0
     for e in beam[X]:
            self.beamCurrent[X][1].AddPoint(e.unix_timestamp-t0,e.var)
            if e.var > mx: mx=e.var
     self.beamCurrent[X][0] = mx
     
# work with second axis
     if 1<0:
        rightmax = 1.1*Lmax/1000
        scale = ROOT.gPad.GetUymax()/rightmax
        h['LumiT'+B] = h['LumiT'].Clone('LumiT'+B)
        h['LumiT'+B].Scale(scale/1000)
        h['LumiT'+B].Draw('same')
        h['ax1'+B] = ROOT.TGaxis(ROOT.gPad.GetUxmax(), ROOT.gPad.GetUymin(),
                   ROOT.gPad.GetUxmax(), ROOT.gPad.GetUymax(),
                   0, rightmax, 510, "+L")
        h['ax1'+B].SetTitle('L [Hz/nb]    ')
        h['ax1'+B].SetTextFont(42)
        h['ax1'+B].SetLabelFont(42)
        h['ax1'+B].SetTextColor(ROOT.kMagenta)
        h['ax1'+B].Draw()
        h['l'+B]=ROOT.TLegend(0.44,0.86,0.91,0.98)
        h['l'+B].AddEntry(h['timeB2noB1_100'],'triggered event rate ',"PL")
        h['l'+B].AddEntry(h['timeWtB2noB1_100'],'event rate#times'+str(rescale[B][0])+' with Scifi tracks',"PL")
        h['l'+B].AddEntry(h['timeWtDSB2noB1_100'],'event rate#times'+str(rescale[B][1])+' with DS tracks',"PL")
        h['l'+B].AddEntry(h['LumiT'],'IP1 instanteous luminosity',"PL")
        h['l'+B].Draw()
        h[B].Update()
        self.myPrint(B,B+'-'+str(runNumber).zfill(6))



   def merge(self):
        h = self.h
        for fname in os.listdir():
            if fname.find('FS')==0 and fname.find('.root')>0 and fname.find('dict')<0:
                rname = fname.split('-')[1].split('.')[0]
                F = ROOT.TFile(fname)
                h[rname] = F.c1.Clone(rname)
                h[rname].SetName(rname)
                h[rname].SetTitle(rname)
        F = ROOT.TFile('BunchStructure.root','recreate')
        keys = list(h.keys())
        keys.sort(reverse=True)
        for r in keys:
           if r.find('run')==0: 
              if int(r.split('run')[1])<options.rmin: continue
              h[r].Write()
        F.Close()

   def mergeLumi(self):
        h = self.h
        Llist = []
        for fname in os.listdir():
            if fname.find('Lumi-run')==0 and fname.find('.root')>0:
                rname = fname.split('-')[1].split('.')[0]
                F = ROOT.TFile(fname)
                if not F.Get('c1'):
                   print('error file',fname)
                   continue
                h[rname] = F.c1.Clone(rname)
                h[rname].SetName(rname)
                h[rname].SetTitle(rname)
                Llist.append(rname)
        F = ROOT.TFile('Lumi.root','recreate')
        Llist.sort(reverse=True)
        for r in Llist:
           print('write 0',r,h[r])
           if r.find('run')==0: h[r].Write()
           print('write 1',r,h[r])
        F.Close()

   def lhcNumbering(self):
        h = self.h
        Nbunches = h['bnr'].GetNbinsX()
        F = ROOT.TFile('BunchStructure.root')
        p = open("FSdict.pkl",'rb')
        self.FSdict = pickle.load(p)
        for k in F.GetListOfKeys():
                rname = k.GetName()
                newname = 'lhc-'+rname
                h[newname] = F.Get(rname).Clone(newname)
                h[newname].SetName(rname)
                h[newname].SetTitle(rname)
        F.Close()
        F = ROOT.TFile('BunchStructureLHC.root','recreate')
        for newname in h:
                if not newname.find('lhc-') == 0:continue
                X = {}
                for p in h[newname].GetListOfPrimitives():
                   X[p.GetName()] = p
                a = X['b1z'].GetTitle().replace(' ','')
                X['bnr'].SetLineColor(ROOT.kBlack)
                runnr = int(a.split('run')[1].split('fill')[0])
                phaseShift1 = self.FSdict[runnr]['phaseShift1']
                phaseShift2 = self.FSdict[runnr]['phaseShift2']
                for hname in X:
                   histo=X[hname]
                   if not hasattr(histo,'GetBinContent'): continue
                   tmp = {}
                   for i in range(Nbunches):
                      tmp[i+1] = histo.GetBinContent(i+1)
                   for i in range(Nbunches):
                        newBin = (Nbunches - phaseShift1 + i)%Nbunches
                        histo.SetBinContent(newBin,tmp[i+1])
                h[newname].Update()
                h[newname].Write()

   def BunchNumberPlotFromData(self,r):
# check for partitions
          runNr = str(r).zfill(6)
          partitions = []
          conv_data_path = options.rawData.replace("raw_data", "convertedData")
          eventChain = ROOT.TChain('rawConv')
          eventChain.Add(os.environ['EOSSHIP']+conv_data_path+'run_'+runNr+'/*.root')
          nEvents = eventChain.GetEntries()
# make the plot
          # figure out the number of bunches in the LHC
          rc = eventChain.GetEvent(0)
          if eventChain.EventHeader.GetAccMode()==12: # ion runs
            Nbunches = 1782
            div = 8
          else: # proton runs
            Nbunches = 3564
            div = 4
          ut.bookHist(self.h,'bnr_from_data','bunch number; LHC bunch number', Nbunches,-0.5,Nbunches-0.5)
          # use postscale, same logic as in the monitoring task
          postScale = 0
          if nEvents>10E6: postScale = 10
          if nEvents>100E6: postScale = 100
          print('using postScale ',postScale,' for run ',r)
          for event in eventChain:
            if postScale>0:
              if ROOT.gRandom.Rndm()>1./postScale: continue
            self.h['bnr_from_data'].Fill(int((event.EventHeader.GetEventTime()%(div*Nbunches))/div+0.5))
          
          return self.h['bnr_from_data']

   def getEntriesPerRun(self,r):
# check for partitions
          runNr = str(r).zfill(6)
          partitions = []
          dirlist  = str( subprocess.check_output("xrdfs "+os.environ['EOSSHIP']+" ls "+options.convpath+"run_"+runNr,shell=True) )
          for x in dirlist.split('\\n'):
             ix = x.find('sndsw_raw-')
             if ix<0: continue
             partitions.append(x[ix:])
          eventChain = ROOT.TChain('rawConv')
          for p in partitions:
             eventChain.Add(os.environ['EOSSHIP']+options.convpath+'run_'+runNr+'/'+p)
          return eventChain.GetEntries(),partitions

   def getTotalStat(self):
      L = 0
      N = 0
      for r in self.runInfo:
           L+=self.runInfo[r]['lumiAtIP1withSNDLHC']
           N+=self.runInfo[r]['Entries']

   def makeLatex(self):
        # make latex code 
        # filling schemes:  FS-run004626.pdf  and  Lumi-run004626.pdf
        L,N= 0,0
        for r in self.runInfo:
          if r>options.rmin:
            L+=self.runInfo[r]['lumiAtIP1withSNDLHC']
            N+=self.runInfo[r]['Entries']
        lines = []
        lines.append("\documentclass{beamer}")
        lines.append("\mode<presentation>")
        lines.append("{\\usetheme{Singapore}}")
        lines.append("\\usepackage{graphicx}")
        lines.append("\\usepackage[space]{grffile}")
        lines.append("\\usepackage[english]{babel}")
        lines.append("\\usepackage[latin1]{inputenc}")
        lines.append("\\usepackage[T1]{fontenc}")
        lines.append("\\title[Short Paper Title] % (optional, use only with long paper titles)")
        if options.convpath.find('2022')>0:
          lines.append("{SND@LHC Run Summary July - November 2022}")
          lines.append("\date[Short Occasion] % (optional)")
          lines.append("{ 17 November 2022}")
          lines.append("\\begin{document}")
          lines.append("\\begin{frame}{}")
          lines.append("17 November 2022")
          lines.append("\\newline  ")
          lines.append("\\newline  ")
          lines.append("Run Summary for July - November 2022")
        else:
          lines.append("{SND@LHC Run Summary March - July 2023}")
          lines.append("\date[Short Occasion] % (optional)")
          lines.append("{ 16 July 2023}")
          lines.append("\\begin{document}")
          lines.append("\\begin{frame}{}")
          lines.append("16 July 2023")
          lines.append("\\newline  ")
          lines.append("\\newline  ")
          lines.append("Run Summary for March - July 2023")
        nTXT = "$%5.2F\\times 10^9 $"%(N/1E9)
        lines.append("\\begin{itemize}")
        lines.append("\item total number of events: "+nTXT)
        lines.append("\item integrated luminosity (lower limit): $%5.2F\mathrm{fb}^{-1}$"%(L/1E9))
        lines.append("\end{itemize}")
        lines.append("\\begin{center}")
        lines.append("\includegraphics[width = 0.7\\textwidth]{Lumi-time.pdf}")
        lines.append("\end{center}")
        lines.append("\end{frame}")
        lines.append("\\begin{frame}{}")
        lines.append("\\begin{center}")
        elist = list(emulsionReplacements.values())
        elist.sort(reverse=True)
        k=0
        for emulsionNr in elist:
           if k==6:
                k=0
                lines.append("\end{center}")
                lines.append("\end{frame}")
                lines.append("\\begin{frame}{}")
                lines.append("\\begin{center}")
           if "ScifitrackDens"+str(emulsionNr)+".pdf" in os.listdir('.'):
              lines.append("\includegraphics[width = 0.4\\textwidth]{ScifitrackDens"+str(emulsionNr)+".pdf}")
              k+=1
        lines.append("\end{center}")
        lines.append("\end{frame}")

        R = list(self.runInfo.keys())
        R.sort(reverse=True)

        lines.append("\\begin{frame}{}")
        lines.append("Overview of  runs \\newline")
        lines.append("\\scriptsize")
        lines.append("\\begin{scriptsize}")
        lines.append("\\begin{tabular}{lcrrrl}")
        lines.append("  Run &  Fill &  events & mu & Lumi $\mathrm{pb}^{-1}$ & start time \\\\ ")
        ilines = 0
        for  i in range(len(R)):
           r = R[i]
           if r < options.rmin: continue
           lumi = self.runInfo[r]['lumiAtIP1withSNDLHC']/1E6
           N =  self.runInfo[r]['Entries']
           fill =  str(self.runInfo[r]['Fillnumber'])
           if fill == '': fill = ' -- '
           mu = ''
           if 'muAv' in self.runInfo[r]: mu = "%4.1F"%(self.runInfo[r]['muAv'][''])
           lines.append(" %i & %6s & %10i & %s & $%5.1F$ & %s \\\\"%(r,fill,N,mu,lumi,self.runInfo[r]['StartTimeC']))
           ilines+=1
           if ilines%19==0: 
                lines.append("\end{tabular}")
                lines.append("\\end{scriptsize}")
                lines.append("\end{frame}")
                lines.append("\\begin{frame}{}")
                lines.append("\\begin{scriptsize}")
                lines.append("\\begin{tabular}{lcrrrl}")
                lines.append("  Run &  Fill &  events & mu &  Lumi $\mathrm{pb}^{-1}$ & start time \\\\ ")
        lines.append("\end{tabular}")
        lines.append("\\end{scriptsize}")
        lines.append("\end{frame}")

#
# runs with beam present measured
        RwL = []
        self.runsWithBeam()
        for  r in self.runs:
             withBeam = False
             if r not in self.runInfo: continue
             if 'timeWtDS' in  self.runs[r]:
                 if self.runs[r]['timeWtDS']/self.runs[r]['time'] > 0.25: withBeam = True
             if self.runInfo[r]['lumiAtIP1withSNDLHC']>0 or withBeam: RwL.append(r)
        RwL.sort(reverse=True)
        R = RwL
        lines.append("\\begin{frame}{}")
        k=0
        for i in range(len(R)):
           print('at run ',R[i])
           r = str(R[i]).zfill(6)
           if not "FS-run"+r+".pdf" in os.listdir('.'): continue
           if "Lumi-run"+r+".pdf" in os.listdir('.'):
              lines.append("\includegraphics[width = 0.3\\textwidth]{Lumi-run"+r+".pdf}")
           else:
             lines.append("\includegraphics[width = 0.3\\textwidth]{noLumi-run"+r+".pdf}")
           if "FS-run"+r+".pdf" in os.listdir('.'):
             lines.append("\includegraphics[width = 0.3\\textwidth]{FS-run"+r+".pdf}")
           if "run"+r+"Lumi-tracks.pdf" in os.listdir('.'):
             lines.append("\includegraphics[width = 0.3\\textwidth]{run"+r+"Lumi-tracks.pdf}")
           lines.append("\\newline")
           if (3*k+3)%12==0: 
                lines.append("\end{frame}")
                lines.append("\\begin{frame}{}")
           k+=1
        lines.append("\end{frame}")
        lines.append(" ")
#
        lines.append("\end{document}")
        outFile = open(os.environ["HOME"]+'/dummy','w')
        for l in lines:
             rc = outFile.write(l+"\n")
        outFile.close()
        os.system('cp $HOME/dummy LumiSummary.tex')

   def getIntegratedLumiFromPlot(dateA,dateB):
      time_objA = time.strptime(dateA,'%m-%d,%Y-%H')
      time_objB = time.strptime(dateB,'%m-%d,%Y-%H')
      TA = calendar.timegm(time_objA)
      TB = calendar.timegm(time_objB)
      if not 'cL' in h: ut.readHists(h,'Lumi-time.root')
      n=0
      for x in h['cL'].GetListOfPrimitives():
         if x.ClassName() == 'TGraph':
             h['Lumi'+str(n)] = x.Clone('Lumi'+str(n))
             n+=1
         if x.ClassName() == 'TGaxis':
             h['IntLumiAxis'] = x.Clone('IntLumiAxis')
      d = h['Lumi1'].Eval(TB)-h['Lumi1'].Eval(TA)
      scale = h['IntLumiAxis'].GetWmax() / h['IntLumiAxis'].GetY2()
      print('Lumi between ',dateA,dateB,' = ',d*scale)

   def plotLumiPerTime(self):
        h = self.h
        runInfo = self.runInfo
        h['LumiT']=ROOT.TGraph()
        h['ILumiT']=ROOT.TGraph()
        Lint = 0
        runList = list(runInfo.keys())
        runList.sort()
        lmax = 0
        for r in runList:
           if r<options.rmin: continue
           h['LumiT'].AddPoint(runInfo[r]['StartTime'],0)
           X = runInfo[r]['lumiAtIP1withSNDLHC']/1E6
           h['LumiT'].AddPoint(runInfo[r]['StartTime']+1,X)
           if X>lmax : lmax = X
           h['LumiT'].AddPoint(runInfo[r]['StartTime']+3600,0)
           Lint+=X
           h['ILumiT'].AddPoint(runInfo[r]['StartTime'],Lint/1000)
        tstart = h['LumiT'].GetPointX(0)
        tend = h['LumiT'].GetPointX(h['LumiT'].GetN()-1)
        ut.bookHist(h,'LumiTime','; ; pb^{-1}',100,tstart,tend)
        ut.bookCanvas(self.h,'cL','',2400,800,1,1)
        h['cL'].cd()
        h['LumiTime'].GetXaxis().SetTimeFormat("%d-%m")
        h['LumiTime'].GetXaxis().SetTimeOffset(0,'gmt')
        h['LumiTime'].SetMaximum(lmax*1.2)
        h['LumiTime'].SetStats(0)
        h['LumiTime'].Draw()
        h['cL'].Update()
# work with second axis
        rightmax = 1.1*Lint/1000.
        scale = ROOT.gPad.GetUymax()/rightmax
        h['ILumiT'].Scale(scale)
        h['ax2'] = ROOT.TGaxis(ROOT.gPad.GetUxmax(), ROOT.gPad.GetUymin(),
                   ROOT.gPad.GetUxmax(), ROOT.gPad.GetUymax(),
                   0, rightmax, 510, "+L")
        h['ax2'].SetTitle('L  Integral; fb^{-1}')
        h['ax2'].SetTextFont(42)
        h['ax2'].SetLabelFont(42)
        h['ax2'].SetTextColor(ROOT.kRed)
        h['ax2'].Draw()
        h['LumiT'].SetLineColor(ROOT.kBlue)
        h['LumiT'].SetLineWidth(2)
        h['LumiT'].Draw('Lsame')
        h['ILumiT'].SetLineColor(ROOT.kRed)
        h['ILumiT'].SetLineWidth(3)
        h['ILumiT'].Draw('same')
        self.myPrint('cL','Lumi-time')

   def LumiIntegral(self,rmin,rmax):
        L = 0
        for r in self.runInfo:
           if r>= rmin and r<=rmax:
             L+=self.runInfo[r]['lumiAtIP1withSNDLHC']
        print('Lumi for run ',rmin,' to run ',rmax,' = ',L)
   def LumiPerFill(self):
      lumiPerFill={}
      for r in  self.runInfo:
         lumiPerFill[ self.runInfo[r]['Fillnumber'] ]=self.runInfo[r]['lumiAtIP1']
         I = 0
      for x in lumiPerFill:
         I+=lumiPerFill[x]
      print('total:',l)

   def runsWithBeam(self):
# potential runs with beam selected by looking for daq rate > cosmics
           self.listOfRuns = {}
           if options.convpath.find('2022')>0:
              offline =www + "offline.html"
           else:
              offline =www + "offline2023.html"
           with client.File() as f:
               f.open(offline)
               status, L = f.read()
               Lhtml = L.decode().split('\n')
               f.close()
           self.runs = {}
           for x in Lhtml:
                if x.find('#event')<0: continue
                ir = x.find("run ")
                if ir<0: continue
                k = x[ir:].find(" ")
                runNumber = int(x[ir+4:ir+4+k+1])
                if runNumber<options.rmin: continue
                R = ROOT.TFile.Open(www+"offline/run"+str(runNumber).zfill(6)+".root")
                bCanvas = R.daq.Get('T')
                if not bCanvas:
                       print('Error with root file',runNumber)
                       continue
                Xt = {'time':None,'timeWtDS':None,'timeWt':None}
                self.runs[runNumber] = {}
                for x in Xt:
                    Xt[x] = bCanvas.FindObject(x)
                    if Xt[x]:
                        Ntot = Xt[x].GetSumOfWeights()
                        Ttot = Xt[x].GetBinCenter(Xt[x].GetNbinsX()-1)
                        self.runs[runNumber][x] = Ntot/Ttot
           for r in self.runs:
              if 'timeWtDS' in  self.runs[r]:
                 if self.runs[r]['timeWtDS']/self.runs[r]['time'] > 0.25:
                    if not r in self.runInfo: print('run with beam but no filling scheme:',r,'Fillnumber',self.runs[r])
              else:
                 print('run not uptodate',r)

   def tracksPerLumi(self,aRun=False):
        h=self.h
        pol1 = ROOT.TF1('pol1','[0]+[1]*x',0,1E7)
        self.F = ROOT.TFile.Open(options.path+"Lumi.root")
        if aRun:
            keys = ['run'+str(aRun).zfill(6)]
            Frun = ROOT.TFile.Open(www+"offline/"+keys[0]+'.root')
        else:
            keys = self.F.GetListOfKeys()
        ROOT.gROOT.cd()
        for k in keys:
             if aRun: nm = k
             else:    nm = k.GetName()
             runNumber = int(nm.split('run')[1])
             if runNumber < options.rmin: continue
             tc = self.F.Get(nm)
             for x in tc.GetListOfPrimitives():
                  if x.GetTitle()=='' and x.GetName()=='': lumi = x
                  if x.GetTitle().find('Hz/nb')>0: yaxis = x
             if not lumi: 
                 print('lumi not available',runNumber)
                 continue
             calib = yaxis.GetWmax()/yaxis.GetY2()
             lumi.Scale(calib)
             mean = lumi.GetMean(axis=2)
             for iMin in range(lumi.GetN()):
                  tMin =  lumi.GetPointX(iMin)
                  if lumi.GetPointY(iMin)>mean*0.001: break
             for i in range(lumi.GetN()-1,0,-1):
                  tMax =  lumi.GetPointX(i)
                  if lumi.GetPointY(i)/lumi.GetPointY(iMin)>0.001: break
             # special cases
             if runNumber == 5157: tMin = 4000
             if runNumber == 5122: tMin = 8000
             if runNumber == 5120: tMin = 4000
             if runNumber == 4449: tMin = 5100
             if runNumber == 4504: 
                   tMin = 4900
                   tMax = 15000
             for x in ["timeWtDS","timeWt"]:
               hx = nm+x+'100'
               if aRun:
                 h[hx] = Frun.daq.T.FindObject(x).Clone(hx)
                 h[hx].Rebin(100)
                 h[hx].Scale(0.01)
               else:
                 h[hx] = tc.FindObject(x+'10').Clone(hx)
                 h[hx].Rebin(10)
                 h[hx].Scale(0.1)
               h[hx].GetYaxis().SetTitle('dt = 100s  [evts/nb^{-1}]')
               h[hx].SetTitle('DS(cyan) and Scifi(red) tracks   run '+str(runNumber))
               if tMin < h[hx].GetBinCenter(1): tMin = h[hx].GetBinCenter(3)
               if tMax >  h[hx].GetBinCenter(h[hx].GetNbinsX()-1): tMax = h[hx].GetBinCenter(h[hx].GetNbinsX()-1)
               if h[hx].GetEntries()>0:
                for i in range(h[hx].GetNbinsX()):
                    t1 = h[hx].GetBinLowEdge(i+1)
                    t2 = t1 + h[hx].GetBinWidth(i+1)
                    tl = [0,0]
                    rMean,rK =0,0
                    if t1>=tMin and t2<=tMax:
                      rc = lumi.Fit(pol1,'q','',t1,t2)
                      L = pol1.Integral(t1,t2)/(t2-t1)  # s-1 nb-1
                      if not L>0:
                        print('error with lumi',runNumber,t1,t2)
                      else:
                        tl[0] = h[hx].GetBinContent(i+1)/L
                        tl[1] = h[hx].GetBinError(i+1)/L
                    h[hx].SetBinContent(i+1,tl[0])
                    rMean += tl[0]
                    rK+=1
                    h[hx].SetBinError(i+1,tl[1])
# look for outliers
                rMean = rMean/rK
                for i in range(h[hx].GetNbinsX()):
                     if h[hx].GetBinContent(i+1)< 0.4*rMean:
                            h[hx].SetBinContent(i+1,0)
                            h[hx].SetBinError(i+1,0)
             tc = h['c1'].cd()
             h[nm+'timeWtDS100'].SetMaximum(140)
             h[nm+'timeWtDS100'].SetMinimum(0.0)
             h[nm+'timeWtDS100'].Draw()
             textInfo = ROOT.TLatex()
             if  h[nm+'timeWt100'].GetEntries()>0:
                 h[nm+'timeWt100'].Draw('same')
                 rc = h[nm+'timeWt100'].Fit('pol1','qS','',tMin,tMax)
                 res = rc.Get()
                 if res:
                   chi2 = res.Chi2()/(tMax-tMin)
                   fun = h[nm+'timeWt100'].GetFunction('pol1')
                   if chi2<10 and abs(fun.GetParameter(1)*3600)<5 and fun.GetParameter(0) < 500:
                     b,m = fun.GetParameter(0),fun.GetParameter(1)
                     meanV = m*(tMin+tMax)/2+b
                     txtScifi = "Scifi tracks per nb^{-1} mean: %5.1F  slope: %5.2F per hour   %5.1F "%(meanV,fun.GetParameter(1)*3600,chi2)
                     textInfo.DrawLatexNDC(0.2, 0.85,txtScifi)
                   else:
                     print('Fit failed',nm+'timeWt100',tMin,tMax,chi2,abs(fun.GetParameter(1)*3600),fun.GetParameter(0))
                     if h[nm+'timeWt100'].GetFunction('pol1'): h[nm+'timeWt100'].GetFunction('pol1').Delete()

             rc = h[nm+'timeWtDS100'].Fit('pol1','Sq','',tMin,tMax)
             res = rc.Get()
             if res:
              chi2 = res.Chi2()/(tMax-tMin)
              fun = h[nm+'timeWtDS100'].GetFunction('pol1')
              if chi2<20 and abs(fun.GetParameter(1)*3600)<15 and fun.GetParameter(0) < 500:
                b,m = fun.GetParameter(0),fun.GetParameter(1)
                meanV = m*(tMin+tMax)/2+b
                txtDS = "   DS tracks per nb^{-1} mean: %5.1F  slope: %5.1F per hour  %5.1F "%(meanV,m*3600,chi2)
                textInfo.DrawLatexNDC(0.2, 0.8,txtDS)
              else:
                print('Fit failed',nm+'timeWtDS100',tMin,tMax,chi2,abs(fun.GetParameter(1)*3600),fun.GetParameter(0))
                if h[nm+'timeWtDS100'].GetFunction('pol1'): h[nm+'timeWtDS100'].GetFunction('pol1').Delete()
             tc.Update()
             h['c1'].Print(nm+'Lumi-tracks.root')
             h['c1'].Print(nm+'Lumi-tracks.pdf')
# integral of tracks per cm2
             self.R = ROOT.TFile.Open(www+"offline/run"+str(runNumber).zfill(6)+".root")
             Nevts = self.R.daq.Get('T').FindObject('time').GetEntries()
             postScale = self.runInfo[runNumber]['Entries']/Nevts
             T = 'scifi-TtrackPos'
             h[T] = self.R.scifi.Get(T).FindObject('scifi-trackPosBeam').Clone(T)
             T = 'mufi-TtrackPos'
             h[T] = self.R.mufilter.Get(T).FindObject('mufi-trackPosBeam').Clone(T)
             ROOT.gROOT.cd()
             rList = list(emulsionReplacements.keys())
             rList.sort(reverse=True)
             for runNr in rList:
                if runNumber >= runNr: 
                    emulsionNr = emulsionReplacements[runNr]
                    break
             for k in [ 'scifi-TtrackPos','mufi-TtrackPos']:
                 if not 'I'+k+str(emulsionNr) in h:
                    h['I'+k+str(emulsionNr)] = h[k].Clone('I'+k)
                    h['I'+k+str(emulsionNr)].Scale(postScale)
                    if k=='scifi-TtrackPos': h['emulsionILumi'+str(emulsionNr)] =  self.runInfo[runNumber]['lumiAtIP1withSNDLHC']
                 else: 
                    h['I'+k+str(emulsionNr)].Add(h[k],postScale)
                    if k=='scifi-TtrackPos': h['emulsionILumi'+str(emulsionNr)] +=  self.runInfo[runNumber]['lumiAtIP1withSNDLHC']

# merge
        for k in self.F.GetListOfKeys():
             nm = k.GetName()
             tnm = 't'+nm
             if not nm+'Lumi-tracks.root' in os.listdir('.'): continue
             X = ROOT.TFile(nm+'Lumi-tracks.root')
             ROOT.gROOT.cd()
             if not X.Get('c1'):
                 print('problem with',k,nm)
             h[tnm] = X.c1.Clone(tnm)
             h[tnm].SetName(tnm)
             h[tnm].SetTitle(tnm)
        F = ROOT.TFile('Lumi-tracks.root','recreate')
        keys = list(h.keys())
        keys.sort(reverse=True)
        for r in keys:
           if r.find('trun')==0: h[r].Write()
#
        elist = list(emulsionReplacements.values())
        for emulsionNr in elist:
            if not 'Iscifi-TtrackPos'+str(emulsionNr) in h: continue
            ut.bookCanvas(h,'ScifitrackDens'+str(emulsionNr),'',900,900,1,1)
            tc = h['ScifitrackDens'+str(emulsionNr)].cd()
            histo = h['Iscifi-TtrackPos'+str(emulsionNr)]
            histo.SetTitle(histo.GetTitle()+' emulsion Nr '+str(emulsionNr)+" #int L=%5.2Ffb^{-1} "%(h['emulsionILumi'+str(emulsionNr)]/1E9))
            histo.GetZaxis().SetTitle(' N/cm^{2} ')
            histo.Draw('colz')
            ROOT.gPad.SetRightMargin(0.13)
            zaxis = histo.GetZaxis()
            zaxis.SetMaxDigits(3)
            zaxis.SetTitleOffset(1.4)
            tc.Update()
            pal = histo.FindObject('palette')
            pal.SetX1NDC(0.86)
            pal.SetX2NDC(0.89)
            tc.Update()
            stats = histo.FindObject('stats')
            stats.SetX1NDC(0.11)
            stats.SetY1NDC(0.57)
            stats.SetX2NDC(0.31)
            stats.SetY2NDC(0.81)
            tc.Update()
            ut.bookCanvas(h,'MufitrackDens'+str(emulsionNr),'',900,900,1,1)
            tc = h['MufitrackDens'+str(emulsionNr)].cd()
            histo = h['Imufi-TtrackPos'+str(emulsionNr)]
            histo.SetTitle(histo.GetTitle()+' emulsion Nr '+str(emulsionNr)+" #int Ldt=%5.2Ffb^{-1} "%(h['emulsionILumi'+str(emulsionNr)]/1E9))
            histo.GetZaxis().SetTitle(' N/cm^{2} ')
            histo.Draw('colz')
            ROOT.gPad.SetRightMargin(0.13)
            zaxis = histo.GetZaxis()
            zaxis.SetMaxDigits(3)
            zaxis.SetTitleOffset(1.4)
            pal = histo.FindObject('palette')
            if not pal:
                print('error',emulsionNr)
                return
            pal.SetX1NDC(0.86)
            pal.SetX2NDC(0.89)
            tc.Update()
            stats = histo.FindObject('stats')
            stats.SetX1NDC(0.11)
            stats.SetY1NDC(0.57)
            stats.SetX2NDC(0.31)
            stats.SetY2NDC(0.81)
            tc.Update()
            h['ScifitrackDens'+str(emulsionNr)].Print(options.path+'ScifitrackDens'+str(emulsionNr)+'.pdf')
            h['MufitrackDens'+str(emulsionNr)].Print(options.path+'MufitrackDens'+str(emulsionNr)+'.pdf')

   def fillStats(self,runNr):
       fsdict = self.FSdict[runNr]
       Nbunches = self.h['bnr'].GetNbinsX()
       self.stats = {'B2noB1':0,'B1only':0,'noBeam':0,'IP1':0,'B2andB1':0,'B1':0,'B2':0}
       for bunchNumber in range(Nbunches):
             nb1 = (Nbunches + bunchNumber - fsdict['phaseShift1'])%Nbunches
             nb2 = (Nbunches + bunchNumber - fsdict['phaseShift1']- fsdict['phaseShift2'])%Nbunches
             b1 = nb1 in fsdict['B1']
             b2 = nb2 in fsdict['B2']
             IP1 = False
             IP2 = False
             if b1:
                IP1 =  fsdict['B1'][nb1]['IP1']
             if b2:
                IP2 =  fsdict['B2'][nb2]['IP2']
             if IP1: self.stats['IP1']+=1
             if b1: self.stats['B1']+=1
             if b2: self.stats['B2']+=1
             if b1 and not IP1 and not b2: self.stats['B1only']+=1
             if b2 and not b1:             self.stats['B2noB1']+=1
             if not b1 and not b2:         self.stats['noBeam']+=1
             if b1 and b2 and not IP1:     self.stats['B2andB1']+=1

   def hitMapsNormalized(self,runNumber,Q12MC=False):
       marker = {'B2noB1':22,'B1only':21,'noBeam':20}
       colors = {'B2noB1':ROOT.kRed,'B1only':ROOT.kOrange,'noBeam':ROOT.kGreen}
       h=self.h
       Nbunches = self.h['bnr'].GetNbinsX()
       self.fillStats(runNumber)
       stats = self.stats
       norm = {'B2noB1':stats['B2'],'B1only':stats['B1'],'noBeam':Nbunches}
       R = ROOT.TFile.Open(www+"offline/run"+str(runNumber).zfill(6)+".root")
       ROOT.gROOT.cd()
       for hitmaps in ['mufi-barmapsVeto','mufi-barmapsUS','mufi-barmapsDS']:
           h[hitmaps] = R.mufilter.Get(hitmaps).Clone(hitmaps)
           for B in marker:
               tc = R.mufilter.Get(B+'/'+hitmaps+B).Clone(hitmaps+B)
               for pad in tc.GetListOfPrimitives():
                for plane in pad.GetListOfPrimitives():
                   if plane.ClassName().find('TH')==0:
                    hname = plane.GetName()
                    h[hname] = plane.Clone(hname)
                    h[hname].Scale(norm[B]/stats[B])
                    h[hname].SetStats(0)
                    h[hname].SetMarkerStyle(marker[B])
                    h[hname].SetLineColor(colors[B])
                    h[hname].SetMarkerColor(h[hname].GetLineColor())
           h[hitmaps].Draw()
           tmp = h[hitmaps].Clone('tmp')
           j = 0
           for pad in tmp.GetListOfPrimitives():
               j+=1
               tc = h[hitmaps].cd(j)
               # tc.SetLogy(1)
               for plane in pad.GetListOfPrimitives():
                   if plane.ClassName().find('TH')==0:
                      hname = plane.GetName()
                      plane.SetMinimum(h[hname+'noBeam'].GetMinimum())
                      plane.SetLineColor(ROOT.kBlue)
                      plane.SetStats(0)
                      plane.Draw()
                      for B in marker:
                          h[hname+B].Draw('same')
           self.myPrint(hitmaps,hitmaps+'-'+str(runNumber).zfill(6))
       hitmaps = 'scifi-hitmaps'
       h[hitmaps] = R.scifi.Get(hitmaps).Clone(hitmaps)
       for B in marker:
           tc = R.scifi.Get(B+'/'+hitmaps+B).Clone(hitmaps+B)
           for pad in tc.GetListOfPrimitives():
              for plane in pad.GetListOfPrimitives():
                 if plane.ClassName().find('TH')==0:
                    hname = plane.GetName()
                    h[hname] = plane.Clone(hname)
                    h[hname].Scale(Nbunches/stats[B])
                    h[hname].SetStats(0)
                    h[hname].SetMarkerStyle(marker[B])
                    h[hname].SetLineColor(colors[B])
                    h[hname].SetMarkerColor(h[hname].GetLineColor())
                    h[hname].SetMarkerSize(0.5)
       h[hitmaps].Draw()
       tmp = h[hitmaps].Clone('tmp')
       j = 0
       for pad in tmp.GetListOfPrimitives():
           j+=1
           tc = h[hitmaps].cd(j)
           for plane in pad.GetListOfPrimitives():
                   if plane.ClassName().find('TH')==0:
                      hname = plane.GetName()
                      plane.SetMinimum(h[hname+'noBeam'].GetMinimum())
                      plane.SetLineColor(ROOT.kBlue)
                      plane.SetStats(0)
                      plane.Draw()
                      for B in marker:
                          h[hname+B].Draw('same')
       self.myPrint(hitmaps,hitmaps+'-'+str(runNumber).zfill(6))
       if Q12MC:
           fmc = ROOT.TFile(Q12MC)
           ROOT.gROOT.cd()
           for hitmaps in ['mufi-barmapsVeto','mufi-barmapsUS','mufi-barmapsDS']:
              tc = fmc.mufilter.Get(hitmaps).Clone(hitmaps+'MC')
              for pad in tc.GetListOfPrimitives():
                for plane in pad.GetListOfPrimitives():
                   if plane.ClassName().find('TH')==0:
                    hname = plane.GetName()+'MC'
                    hnameB2 = plane.GetName()+'B2noB1'
                    h[hname] = plane.Clone(hname)
                    h[hname].Scale(h[hnameB2].Integral(1,20)/h[hname].Integral(1,20))
                    h[hname].SetStats(0)
                    h[hname].SetMarkerStyle(ROOT.kMagenta)
                    h[hname].SetLineColor(ROOT.kMagenta)
                    h[hname].SetMarkerColor(h[hname].GetLineColor())
              h[hitmaps].Draw()
              tmp = h[hitmaps].Clone('tmp')
              j = 0
              for pad in tmp.GetListOfPrimitives():
                j+=1
                tc = h[hitmaps].cd(j)
                for plane in pad.GetListOfPrimitives():
                   if plane.ClassName().find('TH')==0:
                      hname = plane.GetName()
                      if len(hname)>11: continue
                      h[hname+'B2noB1'].SetMaximum( 1.1*max(h[hname+'B2noB1'].GetMaximum(),h[hname+'MC'].GetMaximum()))
                      h[hname+'B2noB1'].SetMinimum(0)
                      h[hname+'B2noB1'].Draw()
                      h[hname+'MC'].Draw('sameHist')
              self.myPrint(hitmaps,hitmaps+'-Q12MC')
               
       
   def storeDict(self,dictPtr,dictName,outFileName):
           fp = ROOT.TFile.Open(outFileName+'.root','recreate')
           pkl = Pickler(fp)
           pkl.dump(dictPtr,dictName)
           fp.Close()
           fp = open(outFileName+'.pkl','wb')
           pickle.dump(dictPtr,fp)
           fp.close()

   def checkSynch(self):
         for r in self.FSdict:
            if not r in self.runInfo:
                 print('run does not exist in runInfo',r)
            elif not self.runInfo[r]['phaseShift1'] == self.FSdict[r]['phaseShift1']:
                 print(r,self.runInfo[r]['phaseShift1'], self.FSdict[r]['phaseShift1'])
            
   def modifyFSdict(self,shift=-1):
         # shift = -1 for converting 2022 to reproc2022, otherwise 0
         fg  = ROOT.TFile.Open(options.path+'FSdict.root')
         pkl = Unpickler(fg)
         self.FSdict = pkl.load('FSdict')
         fg.Close()
         for r in self.FSdict:
            self.FSdict[r]['phaseShift1'] = self.FSdict[r]['phaseShift1'] - shift
         self.storeDict(self.FSdict,'FSdict','FSdict')
         fg  = ROOT.TFile.Open(options.path+'runInfo.root')
         pkl = Unpickler(fg)
         self.runInfo = pkl.load('runInfo')
         fg.Close()
         for r in self.runInfo:
               self.runInfo[r]['phaseShift1'] = self.FSdict[r]['phaseShift1']
         self.storeDict(self.runInfo,'runInfo','RunInfodict')

if __name__ == '__main__':

    from argparse import ArgumentParser
    parser = ArgumentParser()

    parser.add_argument("-r", "--runNumbers", dest="runNumbers", help="list of run numbers",required=False, type=str,default="")
    parser.add_argument("-F", "--fillNumbers",  dest="fillNumbers",   help="corresponding fill number",type=str,required=False,default="")
    parser.add_argument("-c", "--command", dest="command",       help="command", default=None)
    parser.add_argument("-p", dest="path",       help="path to filling scheme", default="./")
    parser.add_argument("-L", dest="lumiversion", help="offline or online lumi from ATLAS", default="offline")
    parser.add_argument("-ip2", dest="withIP2", help="with IP2",default=True)
    parser.add_argument("-raw", dest="rawData", help="path to rawData",default="/eos/experiment/sndlhc/raw_data/physics/2023")   # before "/eos/experiment/sndlhc/raw_data/commissioning/TI18/data"
    parser.add_argument("-www", dest="www", help="path to offline folder",default=os.environ['EOSSHIP']+"/eos/experiment/sndlhc/www/")
    parser.add_argument("-nMin", dest="nMin", help="min entries for a run",default=100000)
    parser.add_argument("--batch", help="run in batch mode",default=False,action='store_true')
    
    options = parser.parse_args()
    if options.batch:
      ROOT.gROOT.SetBatch(True)    
    www = options.www
    if options.rawData.find('2022')>0:
       options.convpath = "/eos/experiment/sndlhc/convertedData/physics/2022/"
       options.rmin = 4361-1
       offline =www+"offline.html"
    elif options.rawData.find('2023')>0:
       options.convpath = "/eos/experiment/sndlhc/convertedData/physics/2023/"
       options.rmin = 5413-1
       offline =www+"offline.html"
    elif options.rawData.find('2024')>0:
       # extract the em target run from the rawData path
       em_run = options.rawData[options.rawData.find("run_"):]
       options.convpath = "/eos/experiment/sndlhc/convertedData/physics/2024/"+em_run
       options.rmin = 7649-1
       offline =www+"offline.html"
    FS = fillingScheme()
    FS.Init(options)
    ut.bookCanvas(FS.h,'c1','c1',1800,900,1,1)
    FS.h['c1'].cd()

    if options.command == "extract":
        FS.Extract()
    elif options.command == "draw":
        options.withIP2=True
        FS.plotBunchStructure(options.fillNumbers,int(options.runNumbers))
    elif options.command == "makeAll" or options.command == "update":
           problems = {}
           with client.File() as f:
               f.open(offline)
               status, L = f.read()
               Lhtml = L.decode().split('\n')
               f.close()
           runs = []
           for x in Lhtml:
                if x.find('#event')<0: continue
                if x.find('noise run')>0: continue
                ir = x.find("run ")
                if ir<0: continue
                k = x[ir:].find(" ")
                runnr = int(x[ir+4:ir+4+k+1])
                j = x.find("#events")
                tmp=x[j:].split('=')[1].split(' ')
                tmp.sort()
                for y in tmp:
                    if y=='': continue
                    nev = int( y )
                    break
                if nev < options.nMin: continue
                scale = 1
                k = x.find('post scaled:')
                if k>0: scale = int(x[k+12:])
                if runnr == 4425: continue   # no beam, but with fill number
                if runnr > options.rmin: runs.append([runnr,scale])
           for z in runs:
                 r = z[0]
                 options.postScale = z[1]
                 FS.options.fillNumbers = ""
                 FS.options.runNumbers = r

                 if options.command == "update" and r in FS.runInfo:
                      if FS.runInfo[r]['lumiAtIP1withSNDLHC']>0:
                        continue
                 FS.Extract()
                 FS.drawLumi(r)
# fill dictionary with useful info
                 Nevts,partitions = FS.getEntriesPerRun(r)
                 fillScheme = "unknown"
                 if hasattr(FS,"lumiAtIP1"):
                    if 'fillingScheme' in FS.lumiAtIP1:
                        fillScheme = FS.lumiAtIP1['fillingScheme']
# cross check
                 postScale = Nevts/FS.h['time'].GetEntries()
                 problems[r]={}
                 if abs(options.postScale/postScale  -1) > 0.1 :
                      print('!!! inconsistent postscale:',r,'html:',options.postScale,' true:',postScale)
                      x = 1
                      if Nevts>5E6: x = 10
                      if Nevts>5E7: x = 100
                      print('should be ',Nevts,x)
                      problems[r]['postScale'] = postScale
                 if not r in FS.LumiInt:
                        FS.LumiInt[r]= [0,0]
                 if not r in FS.FSdict:
                        FS.FSdict[r]={'phaseShift1':0,'phaseShift2':0}
                 N_ScifiTracks = FS.h['timeWt'].GetEntries()
                 N_DSTracks   = FS.h['timeWtDS'].GetEntries()
                 FS.runInfo[r] = {'Fillnumber':int(FS.options.fillNumbers),'phaseShift1':FS.FSdict[r]['phaseShift1'],'phaseShift2':FS.FSdict[r]['phaseShift2'],
                                          'StartTime':FS.startTime,'StartTimeC':time.ctime(FS.startTime),'Entries':Nevts,'partitions':partitions,
                                          'N_scifiTracks':N_ScifiTracks*postScale,'N_DSTracks':N_DSTracks*postScale,
                                          'lumiAtIP1':FS.LumiInt[r][0],'lumiAtIP1withSNDLHC':FS.LumiInt[r][1],
                                          'OfflineMonitoring':"https://snd-lhc-monitoring.web.cern.ch/offline/run.html?file=run"+str(r).zfill(6)+".root&lastcycle",
                                          'FillingScheme':fillScheme}
           FS.merge()
           FS.storeDict(FS.FSdict,'FSdict','FSdict')

           FS.mergeLumi()
           FS.storeDict(FS.LumiInt,'LumiInt','Lumidict')

           # FS.calcMu()
           FS.storeDict(FS.runInfo,'runInfo','RunInfodict')

           L,N= 0,0
           for r in FS.runInfo:
              L+=FS.runInfo[r]['lumiAtIP1withSNDLHC']
              N+=FS.runInfo[r]['Entries']
           print('total nr of events',N,'  integrated luminosity ',L/1E9,'fb')
           print('run tracks per Lumi') 
           FS.tracksPerLumi()
           FS.plotLumiPerTime()
           print('make Latex', 'requires up to date files on EOS! pdflatex LumiSummary.tex')
           FS.makeLatex()
           # os.system('pdflatex LumiSummary.tex')
           
           print('problems',problems)
           # other functionalities of this manager that will be skipped for now
           '''
           print('do not forget to copy to EOS:')
           print('xrdcp -f  Lumi.root              $EOSSHIP/eos/experiment/sndlhc/www/offline/Lumi2023.root')
           print('xrdcp -f Lumidict.root          $EOSSHIP//eos/experiment/sndlhc/convertedData/physics/2023/')
           print('xrdcp -f Lumidict.pkl            $EOSSHIP//eos/experiment/sndlhc/convertedData/physics/2023/')
           print('xrdcp -f FSdict.root              $EOSSHIP//eos/experiment/sndlhc/convertedData/physics/2023/')
           print('xrdcp -f FSdict.pkl               $EOSSHIP//eos/experiment/sndlhc/convertedData/physics/2023/')
           print('xrdcp -f RunInfodict.root      $EOSSHIP//eos/experiment/sndlhc/convertedData/physics/2023/')
           print('xrdcp -f RunInfodict.pkl       $EOSSHIP//eos/experiment/sndlhc/convertedData/physics/2023/')
           print('xrdcp -f Lumi-tracks.root     $EOSSHIP//eos/experiment/sndlhc/www/offline/Lumi-tracks2023.root')
           print('xrdcp -f LumiSummary.pdf   $EOSSHIP//eos/experiment/sndlhc/www/offline/RunSummary2023.pdf')'''
