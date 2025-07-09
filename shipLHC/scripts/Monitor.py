#!/usr/bin/env python
import ROOT
import os,sys,subprocess
import time
import ctypes
from array import array
import rootUtils as ut
import shipunit as u
import SndlhcGeo
from XRootD import client
from XRootD.client.flags import DirListFlags, OpenFlags, MkDirFlags, QueryCode
from rootpyPickler import Unpickler

A,B=ROOT.TVector3(),ROOT.TVector3()
# for fixing a root bug,  will be solved in the forthcoming 6.26 release.

ROOT.gInterpreter.Declare("""
#include "MuFilterHit.h"
#include "AbsMeasurement.h"
#include "TrackPoint.h"

void fixRoot(MuFilterHit& aHit,std::vector<int>& key,std::vector<float>& value, bool mask) {
   std::map<int,float> m = aHit.GetAllSignals(false);
   std::map<int, float>::iterator it = m.begin();
   while (it != m.end())
    {
        key.push_back(it->first);
        value.push_back(it->second);
        it++;
    }
}
void fixRootT(MuFilterHit& aHit,std::vector<int>& key,std::vector<float>& value, bool mask) {
   std::map<int,float> m = aHit.GetAllTimes(false);
   std::map<int, float>::iterator it = m.begin();
   while (it != m.end())
    {
        key.push_back(it->first);
        value.push_back(it->second);
        it++;
    }
}
void fixRoot(MuFilterHit& aHit, std::vector<TString>& key,std::vector<float>& value, bool mask) {
   std::map<TString, float> m = aHit.SumOfSignals();
   std::map<TString, float>::iterator it = m.begin();
   while (it != m.end())
    {
        key.push_back(it->first);
        value.push_back(it->second);
        it++;
    }
}

void fixRoot(std::vector<genfit::TrackPoint*>& points, std::vector<int>& d,std::vector<int>& k, bool mask) {
      for(std::size_t i = 0; i < points.size(); ++i) {
        genfit::AbsMeasurement*  m = points[i]->getRawMeasurement();
        d.push_back( m->getDetId() );
        k.push_back( int(m->getHitId()/1000) );
    }
}
""")

Tkey  = ROOT.std.vector('TString')()
Ikey   = ROOT.std.vector('int')()
Value = ROOT.std.vector('float')()

class Monitoring():
   " set of monitor histograms "
   def __init__(self,options,FairTasks):
        self.options = options
        self.EventNumber = -1
        self.TStart = -1
        self.TEnd   = -1
        self.MonteCarlo = False
        self.Weight = 1

        path     = options.path
        self.myclient = None
        if path.find('eos')>0:
             path  = options.server+options.path
        if options.online:
             path = path.replace("raw_data","convertedData").replace("data/","")
             self.myclient = client.FileSystem(options.server)
# setup geometry
        if (options.geoFile).find('../')<0: self.snd_geo = SndlhcGeo.GeoInterface(path+options.geoFile)
        else:                                         self.snd_geo = SndlhcGeo.GeoInterface(options.geoFile[3:])
        self.MuFilter = self.snd_geo.modules['MuFilter']
        self.Scifi       = self.snd_geo.modules['Scifi']

# MuFilter mapping of planes and bars 
        self.systemAndPlanes   = {1:self.MuFilter.GetConfParI("MuFilter/NVetoPlanes"),
                     2:self.MuFilter.GetConfParI("MuFilter/NUpstreamPlanes"),
                     3:2*self.MuFilter.GetConfParI("MuFilter/NDownstreamPlanes")-1} # to arrive at 7 DS planes
        self.systemAndBars     = {1:self.MuFilter.GetConfParI("MuFilter/NVetoBars"),
                     2:self.MuFilter.GetConfParI("MuFilter/NUpstreamBars"),
                     3:self.MuFilter.GetConfParI("MuFilter/NDownstreamBars")}
        self.systemAndChannels = {1:[self.MuFilter.GetConfParI("MuFilter/VetonSiPMs"),0],
                     2:[self.MuFilter.GetConfParI("MuFilter/UpstreamnSiPMs")-2,2],
                     3:[self.MuFilter.GetConfParI("MuFilter/DownstreamnSiPMs"),0]}
        self.sdict                     = {0:'Scifi',1:'Veto',2:'US',3:'DS'}

        self.freq      =  160.316E6
        self.TDC2ns = 1E9/self.freq

        self.zPos = self.getAverageZpositions()

        self.h = {}   # histogram storage

        self.runNr   = str(options.runNumber).zfill(6)
# presenter file
        if hasattr(self, "saveTo") and options.saveTo!="":
          name = options.saveTo+'run'+self.runNr+'_'+str(options.nStart//1000000)+'.root'
        else:
           name = 'run'+self.runNr+'.root'
        if options.interactive: name = 'I-'+name
        self.presenterFile = ROOT.TFile(name,'recreate')
        for role in ['','shifter', 'expert']:
          self.presenterFile.mkdir('scifi/'+role)
          self.presenterFile.mkdir('mufilter/'+role)
          self.presenterFile.mkdir('daq/'+role)
          if options.interactive: self.presenterFile.mkdir('eventdisplay/'+role)
        self.FairTasks = {}
        for x in FairTasks:   #  keeps extended methods if from python class
                 self.FairTasks[x.GetName()] = x

# setup input
        if options.online:
            import ConvRawData
            options.chi2Max = 2000.
            options.saturationLimit  = 0.95
            options.stop = False
            options.withGeoFile = True
            self.converter = ConvRawData.ConvRawDataPY()
            self.converter.Init(options)
            self.options.online = self.converter
            self.fsdict = False
            self.hasBunchInfo = False
            self.eventTree = options.online.fSink.GetOutTree()
            self.Nkeys = 38   # need to find a way to get this number automatically
            if self.converter.newFormat: self.Nkeys = 1
            for t in self.FairTasks:
               T = self.FairTasks[t]
               self.converter.run.AddTask(T)
               if t=='simpleTracking':  
                   T.Init(self.converter)
                   self.trackTask = self.FairTasks[t]
                   self.Reco_MuonTracks = T.fittedTracks
                   self.clusMufi        = T.clusMufi
                   self.clusScifi       = T.clusScifi
               else: T.Init()
            self.run = self.converter.run
            if self.eventTree.EventHeader.GetAccMode()==12: # ion runs
               self.Nbunches = 1782
               self.div = 8
            else: # proton runs
               self.Nbunches = 3564
               self.div = 4
            return
        else:
            if options.fname:
                f=ROOT.TFile.Open(options.fname)
                eventChain = f.Get('rawConv')
                if not eventChain:   
                    eventChain = f.cbmsim
                    if eventChain.GetBranch('MCTrack'): self.MonteCarlo = True
                partitions = []
            else:
              partitions = 0
              if options.partition < 0:
                 partitions = []
                 if path.find('eos')>0:
# check for partitions
                    dirlist  = str( subprocess.check_output("xrdfs "+options.server+" ls "+options.path+"run_"+self.runNr,shell=True) )
                    for x in dirlist.split('\\n'):
                       ix = x.find('sndsw_raw-')
                       if ix<0: continue
                       partitions.append(x[ix:])
                 else:
# check for partitions
                   dirlist  = os.listdir(options.path+"run_"+self.runNr)
                   for x in dirlist:
                     if not x.find('sndsw_raw-')<0:
                          partitions.append(x)
              else:
                 partitions = ["sndsw_raw-"+ str(options.partition).zfill(4)+".root"]
              if options.runNumber>0:
                eventChain = ROOT.TChain('rawConv')
                for p in partitions:
                       eventChain.Add(path+'run_'+self.runNr+'/'+p)

            rc = eventChain.GetEvent(0)
            self.TStart = eventChain.EventHeader.GetEventTime()
            if options.nEvents <0:
               rc = eventChain.GetEvent(eventChain.GetEntries()-1)
            else:
               rc = eventChain.GetEvent(options.nEvents-1)
            self.TEnd = eventChain.EventHeader.GetEventTime()
            rc = eventChain.GetEvent(0)
# start FairRunAna
            self.run  = ROOT.FairRunAna()
            ioman = ROOT.FairRootManager.Instance()
            ioman.SetTreeName(eventChain.GetName())
            outFile = ROOT.TMemFile('dummy','CREATE')
            source = ROOT.FairFileSource(eventChain.GetCurrentFile())
            for i in range(1,len(partitions)):
                  p = partitions[i]
                  source.AddFile(path+'run_'+self.runNr+'/'+p)
            self.run.SetSource(source)
            self.sink = ROOT.FairRootFileSink(outFile)
            self.run.SetSink(self.sink)

            for t in FairTasks: 
                self.run.AddTask(t)

#avoiding some error messages
            xrdb = ROOT.FairRuntimeDb.instance()
            xrdb.getContainer("FairBaseParSet").setStatic()
            xrdb.getContainer("FairGeoParSet").setStatic()

            self.run.Init()
            if len(partitions)>0:  self.eventTree = ioman.GetInChain()
            else:                 self.eventTree = ioman.GetInTree()
#fitted tracks
            if "simpleTracking" in self.FairTasks:
               self.trackTask = self.FairTasks["simpleTracking"]
               self.Reco_MuonTracks = self.trackTask.fittedTracks
               self.clusMufi        = self.trackTask.clusMufi
               self.clusScifi       = self.trackTask.clusScifi
               self.trackTask.DSnPlanes = 3

# initialize detector class for access to eventheader
        rc = eventChain.GetEvent(0)
        if not self.MonteCarlo:
           self.snd_geo.modules['Scifi'].InitEvent(eventChain.EventHeader)
           self.snd_geo.modules['MuFilter'].InitEvent(eventChain.EventHeader)

        # define the number of bunches in the LHC
        if eventChain.EventHeader.GetAccMode()==12: # ion runs
          self.Nbunches = 1782
          self.div = 8
        else: # proton runs
          self.Nbunches = 3564
          self.div = 4
        
        # get filling scheme, only necessary if not encoded in EventHeader, before 2022 reprocessing
        self.hasBunchInfo = False
        self.fsdict = False
        if hasattr(eventChain.EventHeader,"GetBunchType"):
           if not eventChain.EventHeader.GetBunchType()<0:
                self.hasBunchInfo = True
                print('take bunch info from event header')
        if not self.hasBunchInfo:
         try:
           fg  = ROOT.TFile.Open(options.server+options.path+'FSdict.root')
           pkl = Unpickler(fg)
           FSdict = pkl.load('FSdict')
           fg.Close()
           if options.runNumber in FSdict: self.fsdict = FSdict[options.runNumber]
         except:
           print('continue without knowing filling scheme',options.server+options.path)
         if self.fsdict: 
           print('extract bunch info from filling scheme')
        if self.fsdict or self.hasBunchInfo: 
          for x in ['B1only','B2noB1','noBeam']:
            for role in ['shifter', 'expert']:
             self.presenterFile.mkdir('mufilter/'+role+'/'+x)
             self.presenterFile.mkdir('scifi/'+role+'/'+x)

   def GetEntries(self):
       if  self.options.online:
         if  self.converter.newFormat:  return self.converter.fiN.Get('data').GetEntries()
         else:                                   return self.converter.fiN.event.GetEntries()
       else:
           return self.eventTree.GetEntries()

   def updateSource(self,fname):
   # only needed in auto mode
     notOK = True
     nIter = 0
     while notOK:
      # self.converter.fiN.Close()
      if nIter > 100:
          print('too many attempts to read the file ',fname,' I am exiting.')
          quit()
      nIter+=1
      if self.converter.fiN.GetName() != fname:
          self.converter.fiN = ROOT.TFile.Open(fname)
      else:
          self.converter.fiN = ROOT.TFile.Open(fname)
          Nkeys = self.converter.fiN.ReadKeys()
          if Nkeys != self.Nkeys:
              time.sleep(10)
              self.converter.fiN = ROOT.TFile.Open(fname)
              print('wrong number of keys',Nkeys)
              continue

      listOfKeys = []
      for b in self.converter.fiN.GetListOfKeys(): listOfKeys.append(b.GetName())
      for name in listOfKeys:
        exec("self.converter.fiN."+name+".Refresh()")

      first = -1
      for name in listOfKeys:
          nentries = eval("self.converter.fiN."+name+".GetEntries()")
          if first<0: first = nentries
          if first != nentries:
              print('wrong number of entries',first,nentries)
              first = -1
              break
      if first<0: continue

      for name in listOfKeys:
            if name.find('board')==0:
                self.converter.boards[name]=eval("self.converter.fiN."+name)

      notOK = False

   def GetEvent(self,n):
      if not self.options.online:   # offline, FairRoot in charge

         if self.eventTree.GetBranchStatus('Reco_MuonTracks'):
            for aTrack in self.eventTree.Reco_MuonTracks:
                if aTrack: aTrack.Delete()
            self.eventTree.Reco_MuonTracks.Delete()
         if "simpleTracking" in self.FairTasks:
            self.Reco_MuonTracks.Delete()

      if self.options.online:
            online = self.options.online
            online.executeEvent(n)
            self.Reco_MuonTracks.Delete()
            if self.options.FairTask_convRaw:
                self.options.online.sTree.GetEvent(self.options.online.sTree.GetEntries()-1)
            for t in self.FairTasks: self.FairTasks[t].ExecuteTask()
            self.eventTree = self.options.online.sTree
      else: 
            self.eventTree.GetEvent(n)
            if not self.MonteCarlo:
              # initialize detector class for access to eventheader
              self.snd_geo.modules['Scifi'].InitEvent(self.eventTree.EventHeader)
              self.snd_geo.modules['MuFilter'].InitEvent(self.eventTree.EventHeader)
            if self.MonteCarlo: self.Weight = self.eventTree.MCTrack[0].GetWeight()
            for t in self.FairTasks:
              if t =='simpleTracking':
                 self.FairTasks[t].ExecuteTask(self.options.trackType)
              else:
                 self.FairTasks[t].ExecuteTask()
      self.EventNumber = n

# check for bunch xing type
      self.xing = {'all':True,'B1only':False,'B2noB1':False,'noBeam':False}
      if self.hasBunchInfo:
             binfo = self.eventTree.EventHeader
             self.xing['IP1']  = binfo.isIP1()
             self.xing['IP2']  = binfo.isIP2()
             self.xing['B1']   = binfo.isB1()
             self.xing['B2']   = binfo.isB2()
             self.xing['B1only']  = binfo.isB1Only()
             self.xing['B2noB1']  = binfo.isB2noB1()
             self.xing['noBeam']  = binfo.isNoBeam()
      elif self.fsdict:
             T   = self.eventTree.EventHeader.GetEventTime()
             bunchNumber = int((T%(self.div*self.Nbunches))/self.div+0.5)
             nb1 = (self.Nbunches + bunchNumber - self.fsdict['phaseShift1'])%self.Nbunches
             nb2 = (self.Nbunches + bunchNumber - self.fsdict['phaseShift1']- self.fsdict['phaseShift2'])%self.Nbunches
             b1 = nb1 in self.fsdict['B1']
             b2 = nb2 in self.fsdict['B2']
             IP1 = False
             IP2 = False
             if b1:
                IP1 =  self.fsdict['B1'][nb1]['IP1']
             if b2:
                IP2 =  self.fsdict['B2'][nb2]['IP2']
             self.xing['IP1']  = IP1
             self.xing['IP2']  = IP2
             self.xing['B1']   = b1
             self.xing['B2']   = b2
             self.xing['B1only']   = b1 and not IP1 and not b2
             self.xing['B2noB1']  = b2 and not b1
             self.xing['noBeam'] = not b1 and not b2
             if self.xing['B1only']  and self.xing['B2noB1']  or self.xing['B1only'] and self.xing['noBeam'] : print('error with b1only assignment',self.xing)
             if self.xing['B2noB1']  and self.xing['noBeam'] : print('error with b2nob1 assignment',self.xing)

      return self.eventTree

   def publishRootFile(self):
   # try to copy root file with TCanvas to EOS
       self.presenterFile.Close()
       if self.options.online:
           wwwPath = "/eos/experiment/sndlhc/www/online"
       elif self.options.path.find('2022')>0 :
           wwwPath = "/eos/experiment/sndlhc/www/reprocessing"
       else:    
           wwwPath = "/eos/experiment/sndlhc/www/offline"
       if self.options.sudo: 
         try:
            rc = os.system("xrdcp -f "+self.presenterFile.GetName()+"  "+os.environ['EOSSHIP']+wwwPath)
         except:
            print("copy of root file failed. Token expired?")
         self.presenterFile = ROOT.TFile('run'+self.runNr+'.root','update')

   def purgeMonitorHistos(self):
        wwwPath = "/eos/experiment/sndlhc/www/online/"
        for r in self.options.runNumbers.split(','):
            if r!= '': runList.append(int(r))
        for r in runList:
              runNr   = str(r).zfill(6)+'.root'
              f = ROOT.TFile.Open(options.server+wwwPath+ runNr)
              g = ROOT.TFile('tmp.root','recreate')
              for d in f.GetListOfKeys():
                  D = f.Get(d.GetName())
                  D.Purge()
              for d in f.GetListOfKeys():
                name = d.GetName()
                s = f.Get(name)
                g.mkdir(name)
                g.cd(name)
                for x in s.GetListOfKeys():
                    s.Get(x.GetName()).Write()
        g.Close()
        f.Close()
        os.system('xrdcp -f tmp.root  '+self.options.server+options.path+rName)

   def updateHtml(self):
      if self.options.online: destination="online"
      elif self.options.path.find('2022')>0: destination="reprocessing"
      else: destination="offline"
      rc = os.system("xrdcp -f "+os.environ['EOSSHIP']+"/eos/experiment/sndlhc/www/"+destination+".html  . ")
      old = open(destination+".html")
      oldL = old.readlines()
      old.close()
      tmp = open("tmp.html",'w')
      found = False
      for L in oldL:
           if not L.find(self.runNr)<0: return
           if L.find("https://snd-lhc-monitoring.web.cern.ch/"+destination+"/run.html?file=run")>0 and not found:
              r = str(self.options.runNumber)
              Lnew = '            <li> <a href="https://snd-lhc-monitoring.web.cern.ch/'+destination+'/run.html?file=run'
              Lnew+= self.runNr+'.root&lastcycle">run '+r+' </a>'+self.options.startTime
              if self.options.postScale>1: Lnew+="  post scaled:"+str(self.options.postScale)
              Lnew+='\n'
              tmp.write(Lnew)
              found = True
           tmp.write(L)
      tmp.close()
      os.system('cp '+destination+'.html '+destination+time.ctime().replace(' ','')+'.html ')  # make backup
      os.system('cp tmp.html '+destination+'.html')
      try:
            rc = os.system("xrdcp -f "+destination+".html  "+os.environ['EOSSHIP']+"/eos/experiment/sndlhc/www/")
      except:
            print("copy of html failed. Token expired?")

   def cleanUpHtml(self,destination="online"):
      rc = os.system("xrdcp -f "+os.environ['EOSSHIP']+"/eos/experiment/sndlhc/www/"+destination+".html  . ")
      old = open(destination+".html")
      oldL = old.readlines()
      old.close()
      tmp = open("tmp.html",'w')
      dirlist  = str( subprocess.check_output("xrdfs "+os.environ['EOSSHIP']+" ls /eos/experiment/sndlhc/www/"+destination+"/",shell=True) ) 
      for L in oldL:
           OK = True
           if L.find("https://snd-lhc-monitoring.web.cern.ch/"+destination)>0:
              k = L.find("file=")+5
              m =  L.find(".root")+5
              R = L[k:m]
              if not R in dirlist: OK = False  
           if OK: tmp.write(L)
      tmp.close()
      os.system('cp tmp.html '+destination+'.html')
      try:
            rc = os.system("xrdcp -f "+destination+".html  "+os.environ['EOSSHIP']+"/eos/experiment/sndlhc/www/")
      except:
            print("copy of html failed. Token expired?")

   def checkAlarm(self,minEntries=1000):
      self.alarms = {'scifi':[]}
      # check for empty histograms in scifi signal
      entries = {}
      for mat in range(30):
           entries[mat] = self.h['scifi-mat_'+str(mat)].GetEntries()
      res = sorted(entries.items(), key=operator.itemgetter(1),reverse=True)
      if res[0][1]>minEntries: # choose limit which makes it sensitive to check for empty mats
          for mat in range(30):
              if entries[mat] < 1:
                   s = mat//6 + 1
                   if mat%6 < 3 : p='H'
                   else : p='V'
                   e = str(s)+p+str(mat%3)
                   self.alarms['scifi'].append(e)
      if len(self.alarms['scifi']) > 0:  self.sendAlarm()

      self.alarms['Veto']=[]
      # check for empty histograms in veto signal
      entries = {}
      s = 1
      for p in range(2):
         for side in ['L','R']:
             entries[str(10*s+p)+side] = self.h['mufi-sig'+p+str(10*s+p)].GetEntries()
      res = sorted(entries.items(), key=operator.itemgetter(1),reverse=True)
      if res[0][1]>minEntries: # choose limit which makes it sensitive to check for empty boards
       for p in range(2):
         for side in ['L','R']:
             if entries[str(10*s+p)+side] < 1:
                self.alarms['Veto'].append(str(10*s+p)+side())
      # check for empty histograms in US signal
      entries = {}
      s = 2
      for p in range(5):
         for side in ['L','R']:
             entries[str(s*10+p)+side] = self.h['mufi-sig2'+p+str(s*10+p)].GetEntries()
      res = sorted(entries.items(), key=operator.itemgetter(1),reverse=True)
      if res[0][1]>1000: # choose limit which makes it sensitive to check for empty boards
       for p in range(5):
         for side in ['L','R']:
             if entries[str(s*10+p)+side] < 1:
                self.alarms['Veto'].append(str(s*10+p)+side())


   def sendAlarm(self):
       print('ALARM',self.alarms)

   def systemAndOrientation(self,s,plane):
      if s==1 or s==2: return "horizontal"
      if plane%2==1 or plane == 6: return "vertical"
      return "horizontal"

   def getAverageZpositions(self):
      zPos={'MuFilter':{},'Scifi':{}}
      for s in self.systemAndPlanes:
          for plane in range(self.systemAndPlanes[s]):
             bar = 4
             p = plane
             if s==3 and (plane%2==0 or plane==7): 
                bar = 90
                p = plane//2
             elif s==3 and plane%2==1:
                bar = 30
                p = plane//2
             self.MuFilter.GetPosition(s*10000+p*1000+bar,A,B)
             zPos['MuFilter'][s*10+plane] = (A.Z()+B.Z())/2.
      for s in range(1,self.Scifi.GetConfParI("Scifi/nscifi")+1):
         mat   = 1
         sipm = 1
         channel = 64
         for o in range(2):
             self.Scifi.GetPosition(channel+1000*sipm+10000*mat+100000*o+1000000*s,A,B)
             zPos['Scifi'][s*10+o] = (A.Z()+B.Z())/2.
      return zPos

   def smallSiPMchannel(self,i):
      if i==2 or i==5 or i==10 or i==13: return True
      else: return False

#  Scifi specific code
   def Scifi_xPos(self,detID):
        orientation = (detID//100000)%10
        nStation = 2*(detID//1000000-1)+orientation
        mat   = (detID%100000)//10000
        X = detID%1000+(detID%10000)//1000*128
        return [nStation,mat,X]   # even numbers are Y (horizontal plane), odd numbers X (vertical plane)

# decode MuFilter detID
   def MuFilter_PlaneBars(self,detID):
         s = detID//10000
         l  = (detID%10000)//1000  # plane number
         bar = (detID%1000)
         if s>2:
             l=2*l
             if bar>59:
                  bar=bar-60
                  if l<6: l+=1
         return {'station':s,'plane':l,'bar':bar}

   def map2Dict(self,aHit,T='GetAllSignals',mask=True):
      if T=="SumOfSignals":
         key = Tkey
      elif T=="GetAllSignals" or T=="GetAllTimes":
         key = Ikey
      else: 
           print('use case not known',T)
           1/0
      key.clear()
      Value.clear()
      if T=="GetAllTimes": ROOT.fixRootT(aHit,key,Value,mask)
      else:                         ROOT.fixRoot(aHit,key,Value,mask)
      theDict = {}
      for k in range(key.size()):
          if T=="SumOfSignals": theDict[key[k].Data()] = Value[k]
          else: theDict[key[k]] = Value[k]
      return theDict

   def fit_langau(self,hist,o,bmin,bmax,opt=''):
      if opt == 2:
         params = {0:'Width(scale)',1:'mostProbable',2:'norm',3:'sigma',4:'N2'}
         F = ROOT.TF1('langau',langaufun,0,200,len(params))
      else:
         params = {0:'Width(scale)',1:'mostProbable',2:'norm',3:'sigma'}
         F = ROOT.TF1('langau',twoLangaufun,0,200,len(params))
      for p in params: F.SetParName(p,params[p])
      rc = hist.Fit('landau','S'+o,'',bmin,bmax)
      res = rc.Get()
      if not res: return res
      F.SetParameter(2,res.Parameter(0))
      F.SetParameter(1,res.Parameter(1))
      F.SetParameter(0,res.Parameter(2))
      F.SetParameter(3,res.Parameter(2))
      F.SetParameter(4,0)
      F.SetParLimits(0,0,100)
      F.SetParLimits(1,0,100)
      F.SetParLimits(3,0,10)
      rc = hist.Fit(F,'S'+o,'',bmin,bmax)
      res = rc.Get()
      return res

   def twoLangaufun(self,x,par):
      N1 = self.langaufun(x,par)
      par2 = [par[0],par[1]*2,par[4],par[3]]
      N2 = self.langaufun(x,par2)
      return N1+abs(N2)

   def  langaufun(self,x,par):
   #Fit parameters:
   #par[0]=Width (scale) parameter of Landau density
   #par[1]=Most Probable (MP, location) parameter of Landau density
   #par[2]=Total area (integral -inf to inf, normalization constant)
   #par[3]=Width (sigma) of convoluted Gaussian function
   #
   #In the Landau distribution (represented by the CERNLIB approximation),
   #the maximum is located at x=-0.22278298 with the location parameter=0.
   #This shift is corrected within this function, so that the actual
   #maximum is identical to the MP parameter.
#
      # Numeric constants
      invsq2pi = 0.3989422804014   # (2 pi)^(-1/2)
      mpshift  = -0.22278298       # Landau maximum location
#
      # Control constants
      np = 100.0      # number of convolution steps
      sc =   5.0      # convolution extends to +-sc Gaussian sigmas
#
      # Variables
      summe = 0.0
#
      # MP shift correction
      mpc = par[1] - mpshift * par[0]
#
      # Range of convolution integral
      xlow = max(0,x[0] - sc * par[3])
      xupp = x[0] + sc * par[3]
#
      step = (xupp-xlow) / np
#
      # Convolution integral of Landau and Gaussian by sum
      i=1.0
      if par[0]==0 or par[3]==0: return 9999
      while i<=np/2:
         i+=1
         xx = xlow + (i-.5) * step
         fland = ROOT.TMath.Landau(xx,mpc,par[0]) / par[0]
         summe += fland * ROOT.TMath.Gaus(x[0],xx,par[3])
#
         xx = xupp - (i-.5) * step
         fland = ROOT.TMath.Landau(xx,mpc,par[0]) / par[0]
         summe += fland * ROOT.TMath.Gaus(x[0],xx,par[3])
#
      return (par[2] * step * summe * invsq2pi / par[3])

   def myPrint(self,tc,name,subdir='',withRootFile=True):
     srun = 'run'+str(self.options.runNumber)
     if isinstance(tc, ROOT.TCanvas) :
       tc.Update()
     if withRootFile:
         self.presenterFile.cd(subdir)
         tc.Write()
     else:
         if not os.path.isdir(srun): os.system('mkdir '+srun)
         pname = srun+'/'+name+'-'+srun
         tc.Print(pname+'.png')
         tc.Print(pname+'.pdf')

   def fillHist1(self,hname,parx):
      for x in ['','B1only','B2noB1','noBeam']:
         if x=='':  
             rc = self.h[hname].Fill(parx,self.Weight)
         elif self.xing[x]:
             rc = self.h[hname+x].Fill(parx,self.Weight)
   def fillHist2(self,hname,parx,pary):
      for x in ['','B1only','B2noB1','noBeam']:
         if x=='':  
             rc = self.h[hname].Fill(parx,pary,self.Weight)
         elif self.xing[x]:
             rc = self.h[hname+x].Fill(parx,pary,self.Weight)

class TrackSelector():
   " run reconstruction, select events with tracks"
   def __init__(self,options):
        self.options = options
        self.EventNumber = -1
        self.MonteCarlo = False

        path     = options.path
        if path.find('eos')>0:
             path  = options.server+options.path
# setup geometry
        if (options.geoFile).find('../')<0: self.snd_geo = SndlhcGeo.GeoInterface(path+options.geoFile)
        else:                                         self.snd_geo = SndlhcGeo.GeoInterface(options.geoFile[3:])
        self.MuFilter = self.snd_geo.modules['MuFilter']
        self.Scifi       = self.snd_geo.modules['Scifi']

        self.runNr   = str(options.runNumber).zfill(6)
        if options.runNumber > 0:
           if options.partition < 0:
              partitions = []
              if path.find('eos')>0:
# check for partitions
                 print("xrdfs "+options.server+" ls "+options.path+"run_"+self.runNr)
                 dirlist  = str( subprocess.check_output("xrdfs "+options.server+" ls "+options.path+"run_"+self.runNr,shell=True) )
                 for x in dirlist.split('\\n'):
                     ix = x.find('sndsw_raw-')
                     if ix<0: continue
                     partitions.append(x[ix:])
              else:
# check for partitions
                 dirlist  = os.listdir(options.path+"run_"+self.runNr)
                 for x in dirlist:
                     if not x.find("sndsw_raw-")<0:
                          partitions.append(x)
           else:
                 partitions = ["sndsw_raw-"+ str(options.partition).zfill(4)+".root"]
           eventChain = ROOT.TChain('rawConv')
           for p in partitions:
               eventChain.Add(path+'run_'+self.runNr+'/'+p)
        else:
# for MC data
                eventChain = ROOT.TChain("cbmsim")
                eventChain.Add(options.fname)
                self.MonteCarlo = True
                partitions = []
        rc = eventChain.GetEvent(0)
        self.snd_geo.modules['Scifi'].InitEvent(eventChain.EventHeader)
        self.snd_geo.modules['MuFilter'].InitEvent(eventChain.EventHeader)
# start FairRunAna
        self.run  = ROOT.FairRunAna()
        ioman = ROOT.FairRootManager.Instance()
        ioman.SetTreeName(eventChain.GetName())
        source = ROOT.FairFileSource(eventChain.GetCurrentFile())
        ioman.SetInChain(eventChain)
        first = True
        for p in partitions:
           if first:
                first = False
                continue
           source.AddFile(path+'run_'+self.runNr+'/'+p+'.root')
           self.run.SetSource(source)

#avoiding some error messages
        xrdb = ROOT.FairRuntimeDb.instance()
        xrdb.getContainer("FairBaseParSet").setStatic()
        xrdb.getContainer("FairGeoParSet").setStatic()

# init() tracking tasks
        if self.options.HoughTracking:
           if self.options.trackType == 'Scifi' or self.options.trackType == 'ScifiDS':
              self.muon_reco_task_Sf = options.FairTasks["houghTransform_Sf"]
              self.muon_reco_task_Sf.Init()
              self.genfitTrack = self.muon_reco_task_Sf.genfitTrack
           if self.options.trackType == 'DS' or self.options.trackType == 'ScifiDS':
              self.muon_reco_task_DS = options.FairTasks["houghTransform_DS"]
              self.muon_reco_task_DS.Init()
              self.genfitTrack = self.muon_reco_task_DS.genfitTrack
        if self.options.simpleTracking:
           self.trackTask = options.FairTasks["simpleTracking"]
           if not self.options.HoughTracking:
              self.genfitTrack = self.options.genfitTrack
           
           self.trackTask.SetTrackClassType(self.genfitTrack)
           self.trackTask.Init()

# prepare output tree, same branches as input plus track(s)
        self.outFile = ROOT.TFile(options.oname,'RECREATE')
        self.fSink    = ROOT.FairRootFileSink(self.outFile)

        self.outTree = eventChain.CloneTree(0)
        ROOT.gDirectory.pwd()
        
        # after track tasks init(), output track format is known
        if self.genfitTrack:
                self.fittedTracks = ROOT.TClonesArray("genfit::Track")
                self.fittedTracks.BypassStreamer(ROOT.kFALSE)
        else:
                self.fittedTracks = ROOT.TClonesArray("sndRecoTrack")
        self.MuonTracksBranch    = self.outTree.Branch("Reco_MuonTracks",self.fittedTracks,32000,0)

        if self.options.simpleTracking and not self.options.trackType.find('Scifi')<0 and not eventChain.GetBranch("Cluster_Scifi"):
           self.clusScifi   = ROOT.TClonesArray("sndCluster")
           self.clusScifiBranch    = self.outTree.Branch("Cluster_Scifi",self.clusScifi,32000,1)
        if self.options.simpleTracking and not self.options.trackType.find('DS')<0 and not eventChain.GetBranch("Cluster_Mufi"):
           self.clusMufi   = ROOT.TClonesArray("sndCluster")
           self.clusMufiBranch    = self.outTree.Branch("Cluster_Mufi",self.clusMufi,32000,1)

        B = ROOT.TList()
        B.SetName('BranchList')
        B.Add(ROOT.TObjString('Reco_MuonTracks'))
        B.Add(ROOT.TObjString('Scifi_sndCluster'))
        B.Add(ROOT.TObjString('Mufi_sndCluster'))
        B.Add(ROOT.TObjString('sndScifiHit'))
        B.Add(ROOT.TObjString('MuFilterHit'))
        B.Add(ROOT.TObjString('FairEventHeader'))
        self.fSink.WriteObject(B,"BranchList", ROOT.TObject.kSingleKey)
        self.fSink.SetRunId(options.runNumber)
        self.fSink.SetOutTree(self.outTree)

        self.eventTree = eventChain
        self.run.SetSink(self.fSink)
        self.OT = ioman.GetSink().GetOutTree()

   def ExecuteEvent(self,event):
           track_container_list = []
           # Delete SndlhcTracking fitted tracks container
           if self.options.simpleTracking:
              self.trackTask.fittedTracks.Delete()

           if self.options.trackType == 'ScifiDS':
              if self.options.HoughTracking:
                 self.muon_reco_task_Sf.Exec(0)
                 self.muon_reco_task_DS.Exec(0)
                 track_container_list = [self.muon_reco_task_Sf.kalman_tracks,self.muon_reco_task_DS.kalman_tracks]
              if self.options.simpleTracking:
                 self.trackTask.ExecuteTask(option='ScifiDS')
                 track_container_list.append(self.trackTask.fittedTracks)

           elif self.options.trackType == 'Scifi':
              if self.options.HoughTracking:
                 self.muon_reco_task_Sf.Exec(0)
                 track_container_list.append(self.muon_reco_task_Sf.kalman_tracks)
              if self.options.simpleTracking:
                 self.trackTask.ExecuteTask(option='Scifi')
                 track_container_list.append(self.trackTask.fittedTracks)
                 
           elif self.options.trackType == 'DS':
              if self.options.HoughTracking:
                 self.muon_reco_task_DS.Exec(0)
                 track_container_list.append(self.muon_reco_task_DS.kalman_tracks)
              if self.options.simpleTracking:
                 self.trackTask.ExecuteTask(option='DS')
                 track_container_list.append(self.trackTask.fittedTracks)

           i_muon = -1
           for item in track_container_list:
               for aTrack in item:
                   i_muon += 1
                   self.fittedTracks[i_muon] = aTrack

   def Execute(self):
      for n in range(self.options.nStart,self.options.nStart+self.options.nEvents):

          if self.options.scaleFactor > 1:
             if ROOT.gRandom.Rndm() > 1.0/self.options.scaleFactor: continue

          self.eventTree.GetEvent(n)
          # delete track containers
          self.fittedTracks.Delete()
          
          self.ExecuteEvent(self.eventTree)
          if not self.MonteCarlo and self.fittedTracks.GetEntries() == 0: continue
          if self.options.simpleTracking and not self.options.trackType.find('Scifi')<0:
             if not self.eventTree.GetBranch("Cluster_Scifi"):
                self.clusScifi.Delete()
                self.clusScifi.Expand(len(self.trackTask.clusScifi))
                for index, aCl in enumerate(self.trackTask.clusScifi):
                     self.clusScifi[index] = aCl
          if self.options.simpleTracking and not self.options.trackType.find('DS')<0:
             if not self.eventTree.GetBranch("Cluster_Mufi"):
                self.clusMufi.Delete()
                self.clusMufi.Expand(len(self.trackTask.clusMufi))
                for index, aCl in enumerate(self.trackTask.clusMufi):
                     self.clusMufi[index] = aCl

          # if using FairEventHeader, i.e. before sndlhc header was introduced
          if hasattr(self.OT.EventHeader, "SetMCEntryNumber"):
              self.OT.EventHeader.SetMCEntryNumber(n)
          self.fSink.Fill()

   def Finalize(self):
         self.fSink.Write()
         self.outFile.Close()
