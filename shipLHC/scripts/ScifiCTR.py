import os,atexit
import shipunit as u
import rootUtils as ut
import ROOT
import ctypes
import pickle
import SndlhcGeo
from array import array
def pyExit():
       print("Make suicide until solution found for freezing")
       os.system('kill '+str(os.getpid()))
atexit.register(pyExit)

# supress on-screen histogram drawing
ROOT.gROOT.SetBatch(ROOT.kTRUE)

A,B=ROOT.TVector3(),ROOT.TVector3()

def FCN(npar, gin, f, par, iflag):
#calculate chisquare
       chisq  = 0
       X = ROOT.gROOT.FindObjectAny('commonBlock')
       for matH in range(nMats):
            for matV in range(nMats):
               tdiff = X.GetBinContent(matH*10+matV)
               d = tdiff - (par[matH] - par[matV+nMats])
               chisq += d**2
       f.value = chisq
       return

def FCNS(npar, gin, f, par, iflag):
#calculate chisquare
       chisq  = 0
       X = ROOT.gROOT.FindObjectAny('commonBlock')
       for s1 in range(1, nStations+1): 
          for s2 in range(s1+1, nStations+1): 
             tdiff = X.GetBinContent(s1*10+s2)
             d = tdiff - (par[s2-1] - par[s1-1])
             chisq += d**2
       f.value = chisq
       return

class Scifi_CTR(ROOT.FairTask):
   " time resolution comparing X/Y in same station"
   def Init(self,monitor):
       self.M = monitor
       h = self.M.h
       self.projs = {1:'V',0:'H'}
       
       # setup geometry
       if (options.geoFile).find('../')<0: self.snd_geo = SndlhcGeo.GeoInterface(options.path+options.geoFile)
       else:                               self.snd_geo = SndlhcGeo.GeoInterface(options.geoFile[3:])
       self.MuFilter = self.snd_geo.modules['MuFilter']
       self.Scifi       = self.snd_geo.modules['Scifi']

       self.tag = monitor.iteration
       tag = self.tag
       for s in range(1, nStations+1):
               ut.bookHist(h,'CTR_Scifi'+str(s)+tag,'CTR '+str(s)+tag+'; dt [ns]; ',100,-5.,5.)
               ut.bookHist(h,'CTR_Scifi_beam'+str(s)+tag,'CTR beam'+str(s)+tag+'; dt [ns]; ',100,-5.,5.)
               for matH in range(nMats):
                   for matV in range(nMats):
                    ut.bookHist(h,'CTR_Scifi'+str(s*100+10*matH+matV)+tag,'CTR '+str(s)+tag+'; dt [ns]; ',100,-5.,5.)
                    ut.bookHist(h,'CTR_Scifi_beam'+str(s*100+10*matH+matV)+tag,'CTR beam'+str(s)+tag+'; dt [ns]; ',100,-5.,5.)

               for p in self.projs:
                    ut.bookHist(h,'res'+str(s)+self.projs[p],'d;  [cm]; ',100,-1.,1.)
                    ut.bookHist(h,'resX'+str(s)+self.projs[p],'d;  [cm]; ',100,-1.,1.)
                    ut.bookHist(h,'extrap'+str(s)+self.projs[p],'SiPM distance; L [cm]; ',100,0.,50.)

       for s1 in range(1, nStations):
           for s2 in range(s1+1, nStations+1):
              ut.bookHist(h,'CTR_ScifiStation'+str(s1*10+s2)+tag,'CTR station'+str(s1*10+s2)+'; dt [ns]; ',100,-5.,5.)
              ut.bookHist(h,'CTR_ScifiStation_beam'+str(s1*10+s2)+tag,'CTR station beam'+str(s1*10+s2)+'; dt [ns]; ',100,-5.,5.)

       if self.tag == "v0":
           self.tdcScifiStationCalib = {}
           for s in range(1, nStations+1):
              if nMats == 1 :
                self.tdcScifiStationCalib[s] = [0,{'H':{0:0},'V':{0:0}}]
              if nMats == 3:
                self.tdcScifiStationCalib[s] = [0,{'H':{0:0,1:0,2:0},'V':{0:0,1:0,2:0}}]

       else:
            with open('ScifiTimeAlignment_'+self.tag, 'rb') as fh:
               self.tdcScifiStationCalib = pickle.load(fh)

       self.V =  M.Scifi.GetConfParF("Scifi/signalSpeed")

   def ExecuteEvent(self,event):
       h = self.M.h
       tag =  self.tag
       DetID2Key={}
       for nHit in range(event.Digi_ScifiHits.GetEntries()):
           DetID2Key[event.Digi_ScifiHits[nHit].GetDetectorID()] = nHit

       for aTrack in self.M.Reco_MuonTracks:
          if not aTrack.GetUniqueID()==1: continue
          fitStatus = aTrack.getFitStatus()
          if not fitStatus.isFitConverged(): continue
          state = aTrack.getFittedState()
          mom = state.getMom()
          slopeX = mom.X()/mom.Z()
          slopeY = mom.Y()/mom.Z()
          sortedClusters={}
#
          pos = {}
          for s in range(1, nStations+1):  sortedClusters[s] = {'H':[],'V':[]}
          for nM in range(aTrack.getNumPointsWithMeasurement()):
              state = aTrack.getFittedState(nM)
              Meas = aTrack.getPointWithMeasurement(nM)
              W      = Meas.getRawMeasurement()
              clkey = W.getHitId()
              aCl    = self.M.trackTask.clusScifi[clkey]
              aHit = event.Digi_ScifiHits[DetID2Key[aCl.GetFirst()]]
              s = aCl.GetFirst()//1000000
              aCl.GetPosition(A,B)
              mat = (aCl.GetFirst()//10000)%10
              if aHit.isVertical(): 
                        L = B[1]-state.getPos()[1]
                        sortedClusters[s]['V'].append( [clkey,L,B[0],state.getPos()[1],mat,(A[2]+B[2])/2.] )
                        rc = h['res'+str(s)+'V'].Fill( (A[0]+B[0])/2.-state.getPos()[0])
              else:  
                        L = A[0]-state.getPos()[0]
                        sortedClusters[s]['H'].append( [clkey,L,A[1],state.getPos()[0],mat,(A[2]+B[2])/2.] )
                        rc = h['res'+str(s)+'H'].Fill( (A[1]+B[1])/2.-state.getPos()[1])
          # find station with exactly 1 x and 1 y cluster:
          for s in range(1, nStations+1):
              if not (len(sortedClusters[s]['V']) * len(sortedClusters[s]['H']) ) ==1: continue
              timeCorr = {}
              for proj in ['V','H']:
                  clkey = sortedClusters[s][proj][0][0]
                  aCl    = self.M.trackTask.clusScifi[clkey]
                  L = sortedClusters[s][proj][0][1]
                  rc = h['extrap'+str(s)+proj].Fill(abs(L))
                  time = aCl.GetTime()   # Get time in ns, use fastest TDC of cluster
                  if M.options.check: time = self.M.Scifi.GetCorrectedTime(aCl.GetFirst(),aCl.GetTime(),0)
                  mat = sortedClusters[s][proj][0][4]
                  time-=  self.tdcScifiStationCalib[s][1][proj][mat]
                  time-=  self.tdcScifiStationCalib[s][0]
                  timeCorr[proj] = time - abs(L)/self.V
                  # print(s,proj,time,L,L/self.V)
              dt = timeCorr['H']  - timeCorr['V']
              rc = h['CTR_Scifi'+str(s)+tag].Fill(dt)
              matH,matV = sortedClusters[s]['H'][0][4],sortedClusters[s]['V'][0][4]
              rc = h['CTR_Scifi'+str(100*s+10*matH+matV)+tag].Fill(dt)
              if abs(slopeX)<0.1 and abs(slopeY)<0.1:  
                    rc = h['CTR_Scifi_beam'+str(s)+tag].Fill(dt)
                    rc = h['CTR_Scifi_beam'+str(100*s+10*matH+matV)+tag].Fill(dt)
              dR = sortedClusters[s]['V'][0][2] - sortedClusters[s]['H'][0][3]
              rc = h['resX'+str(s)+'V'].Fill(dR)
              dR = sortedClusters[s]['H'][0][2]-sortedClusters[s]['V'][0][3]
              rc = h['resX'+str(s)+'H'].Fill(dR)
#
              if tag!='v0':
                 stationTimes = {}
                 for s in range(1, nStations+1):
                   if not (len(sortedClusters[s]['V']) * len(sortedClusters[s]['H']) ) ==1: continue
                   sTime = 0
                   for proj in ['V','H']:
                      clkey = sortedClusters[s][proj][0][0]
                      aCl    = self.M.trackTask.clusScifi[clkey]
                      L = sortedClusters[s][proj][0][1]
                      time =  aCl.GetTime()   # Get time in ns, use fastest TDC of cluster
                      if M.options.check: time = self.M.Scifi.GetCorrectedTime(aCl.GetFirst(),aCl.GetTime(),0)
                      mat  =  sortedClusters[s][proj][0][4]
                      time-=  self.tdcScifiStationCalib[s][1][proj][mat]
                      time-=  self.tdcScifiStationCalib[s][0]
                      time-=  abs(L)/self.V
                      sTime += time
                   stationTimes[s] = [sTime/2.,(sortedClusters[s]['H'][0][5] + sortedClusters[s]['V'][0][5])/2.]
                 for s1 in range(1, nStations):
                     if not s1 in stationTimes: continue
                     for s2 in range(s1+1, nStations+1):
                         if not s2 in stationTimes: continue
                         dT = stationTimes[s2][0] - stationTimes[s1][0]
# correct for slope
                         dZ = stationTimes[s2][1] - stationTimes[s1][1]
                         dL = dZ * ROOT.TMath.Sqrt( slopeX**2+slopeY**2+1 )
                         if slopeY>0.1:  dL = -dL     # cosmics from the back
                         dT -= dL / u.speedOfLight
                         rc = h['CTR_ScifiStation'+str(s1*10+s2)+tag].Fill(dT)
                         if abs(slopeX)<0.1 and abs(slopeY)<0.1:
                               rc = h['CTR_ScifiStation_beam'+str(s1*10+s2)+tag].Fill(dT)

   def Plot(self):
      h = self.M.h
      tag =  self.tag
      for b in ['','_beam']:
         ut.bookCanvas(h,'CTR'+b+tag,'CTR'+tag,1800,1200,3,2)
         for s in range(1, nStations+1): 
             tc =  h['CTR'+b+tag].cd(s)
             histo = h['CTR_Scifi'+b+str(s)+tag]
             rc = histo.Fit('gaus','SQ')
             tc.Update()
             stats = histo.FindObject('stats')
             stats.SetOptFit(1111111)
             stats.SetX1NDC(0.62)
             stats.SetY1NDC(0.63)
             stats.SetX2NDC(0.98)
             stats.SetY2NDC(0.94)
             histo.Draw()

         for s in range(1, nStations+1):
             ut.bookCanvas(h,'CTRM'+str(s)+b+tag,'CTR per mat combi',1800,1200,3,3)
             j = 1
             for matH in range(nMats):
                for matV in range(nMats):
                     tc =  h['CTRM'+str(s)+b+tag].cd(j)
                     j+=1
                     histo = h['CTR_Scifi'+b+str(100*s+10*matH+matV)+tag]
                     histo.Fit('gaus','SQ')
                     tc.Update()
                     stats = histo.FindObject('stats')
                     stats.SetOptFit(1111111)
                     stats.SetX1NDC(0.62)
                     stats.SetY1NDC(0.63)
                     stats.SetX2NDC(0.98)
                     stats.SetY2NDC(0.94)
                     histo.Draw()

      if tag!='v0':
        for b in ['','_beam']:
           ut.bookCanvas(h,'CTRS'+b,'CTR per station combination'+tag,1800,1200,5,2)
           k=1
           for s1 in range(1, nStations):
              for s2 in range(s1+1, nStations+1):
                  tc = h['CTRS'+b].cd(k)
                  k+=1
                  histo = h['CTR_ScifiStation'+b+str(s1*10+s2)+tag]
                  histo.Draw()
                  histo.Fit('gaus','SQ')
                  tc.Update()
                  stats = histo.FindObject('stats')
                  stats.SetOptFit(1111111)
                  stats.SetX1NDC(0.62)
                  stats.SetY1NDC(0.63)
                  stats.SetX2NDC(0.98)
                  stats.SetY2NDC(0.94)
                  histo.Draw()

   def ExtractMeanAndSigma(self):
      self.meanAndSigma = {}
      h = self.M.h
      tag =  self.tag
      ut.bookHist(h,'cor')
      for b in ['','_beam']:
         self.meanAndSigma[b]={}
         for s in range(1, nStations+1): 
             for matH in range(nMats):
                for matV in range(nMats):
                     key = 100*s+10*matH+matV
                     histo = h['CTR_Scifi'+b+str(key)+tag]
                     Fun = histo.GetFunction('gaus')
                     self.meanAndSigma[b][key] = [Fun.GetParameter(1),Fun.GetParameter(2)]

   def minimize(self,b=""):
      h = self.M.h
      self.ExtractMeanAndSigma()
      ut.bookHist(h,'commonBlock','',100,0.,100.)

      for s in range(1, nStations+1):
         for matH in range(nMats):
            for matV in range(nMats):
               key = 100*s+10*matH+matV
               dt = self.meanAndSigma[b][key][0]
               h['commonBlock'].SetBinContent(matH*10+matV,dt)
         npar = 2*nMats
         ierflg    = ctypes.c_int(0)
         vstart  = array('d',[0]*npar)
         gMinuit = ROOT.TMinuit(npar)
         gMinuit.SetFCN(FCN)
         gMinuit.SetErrorDef(1.0)
         gMinuit.SetMaxIterations(10000)
         err = 1E-3
         p = 0
         for proj in ['H','V']:
           for m in range(nMats):
             name = "s"+str(s)+proj+str(m)
             gMinuit.mnparm(p, name, vstart[p], err, 0.,0.,ierflg)
             p+=1
         gMinuit.FixParameter(0)
         strat = array('d',[0])
         gMinuit.mnexcm("SET STR",strat,1,ierflg) # 0 faster, 2 more reliable
         gMinuit.mnexcm("SIMPLEX",vstart,npar,ierflg)
         gMinuit.mnexcm("MIGRAD",vstart,npar,ierflg)

         cor    = ctypes.c_double(0)
         ecor  = ctypes.c_double(0)
         p = 0
         for proj in ['H','V']:
           for m in range(nMats):
             rc = gMinuit.GetParameter(p,cor,ecor)
             p+=1
             self.tdcScifiStationCalib[s][1][proj][m] = cor.value

      with open('ScifiTimeAlignment_v1', 'wb') as fh:
           pickle.dump(self.tdcScifiStationCalib, fh)

   def minimizeStation(self,b=""):
      h = self.M.h
      tag = self.tag
      self.meanAndSigmaStation = {}
      self.meanAndSigmaStation[b]={}
      for s1 in range(1, nStations+1): 
           for s2 in range(s1+1, nStations+1): 
                 key = s1*10+s2
                 histo = h[ 'CTR_ScifiStation'+b+str(key)+tag]
                 Fun = histo.GetFunction('gaus')
                 self.meanAndSigmaStation[b][key] = [Fun.GetParameter(1),Fun.GetParameter(2)]
      for s1 in range(1, nStations+1):
         for s2 in range(s1+1, nStations+1): 
            key = 10*s1+s2
            dt = self.meanAndSigmaStation[b][key][0]
            h['commonBlock'].SetBinContent(key,dt)
         npar = 5
         ierflg    = ctypes.c_int(0)
         vstart  = array('d',[0,-0.3365,-0.975,0.11,0.239])
         gMinuit = ROOT.TMinuit(npar)
         gMinuit.SetFCN(FCNS)
         gMinuit.SetErrorDef(1.0)
         gMinuit.SetMaxIterations(10000)
         err = 1E-3
         p = 0
         for s in range(1, nStations+1):
             name = "station"+str(s)
             gMinuit.mnparm(p, name, vstart[p], err, 0.,0.,ierflg)
             p+=1
         gMinuit.FixParameter(0)
         strat = array('d',[0])
         gMinuit.mnexcm("SET STR",strat,1,ierflg) # 0 faster, 2 more reliable
         gMinuit.mnexcm("SIMPLEX",vstart,npar,ierflg)
         gMinuit.mnexcm("MIGRAD",vstart,npar,ierflg)

         cor    = ctypes.c_double(0)
         ecor  = ctypes.c_double(0)
         for s in range(1, nStations+1):
             rc = gMinuit.GetParameter(s-1,cor,ecor)
             self.tdcScifiStationCalib[s][0] = cor.value

      with open('ScifiTimeAlignment_v2', 'wb') as fh:
           pickle.dump(self.tdcScifiStationCalib, fh)

class Scifi_TimeOfTracks(ROOT.FairTask):
   " dt versus dz"
   def Init(self,monitor):
       self.M = monitor
       self.trackTask = self.M.FairTasks['simpleTracking']
       h = self.M.h
       self.V = M.Scifi.GetConfParF("Scifi/signalSpeed")
       ut.bookHist(M.h,'dTvsZ','dT versus dL; dL/0.5cm [cm];dT/100ps [ns]',140,0.,70.,120,-6.,6.)
       ut.bookHist(M.h,'dTvsZ_beam','dT versus dL, beam dL/0.5cm [cm];dT/100ps [ns]',140,0.,70.,120,-6.,6.)
       with open('ScifiTimeAlignment_v2', 'rb') as fh:
           self.tdcScifiStationCalib = pickle.load(fh)

   def ExecuteEvent(self,event):
       h = self.M.h
       for aTrack in self.M.Reco_MuonTracks:
          if not aTrack.GetUniqueID()==1: continue
          fitStatus = aTrack.getFitStatus()
          if not fitStatus.isFitConverged(): continue
          state = aTrack.getFittedState()
          mom = state.getMom()
          slopeX = mom.X()/mom.Z()
          slopeY = mom.Y()/mom.Z()
          sortedClusters={}
          DetID2Key={}
          for nHit in range(event.Digi_ScifiHits.GetEntries()):
            DetID2Key[event.Digi_ScifiHits[nHit].GetDetectorID()] = nHit
#
          pos = {}
          for s in range(1, nStations+1):  sortedClusters[s] = {'H':[],'V':[]}
          for nM in range(aTrack.getNumPointsWithMeasurement()):
              state = aTrack.getFittedState(nM)
              Meas = aTrack.getPointWithMeasurement(nM)
              W      = Meas.getRawMeasurement()
              clkey = W.getHitId()
              aCl   = self.trackTask.clusScifi[clkey]
              aHit = event.Digi_ScifiHits[DetID2Key[aCl.GetFirst()]]
              s = aCl.GetFirst()//1000000
              aCl.GetPosition(A,B)
              mat = (aCl.GetFirst()//10000)%10

              if aHit.isVertical(): 
                        L = B[1]-state.getPos()[1]
                        sortedClusters[s]['V'].append( [clkey,L,B[0],state.getPos()[1],mat,(A[2]+B[2])/2.] )
              else:  
                        L = A[0]-state.getPos()[0]
                        sortedClusters[s]['H'].append( [clkey,L,A[1],state.getPos()[0],mat,(A[2]+B[2])/2.] )
          stationTimes = {}
          for s in range(1, nStations+1):
             if not (len(sortedClusters[s]['V']) * len(sortedClusters[s]['H']) ) ==1: continue
             sTime = 0
             for proj in ['V','H']:
                clkey = sortedClusters[s][proj][0][0]
                aCl    = self.trackTask.clusScifi[clkey]
                L = sortedClusters[s][proj][0][1]
                time =  aCl.GetTime()   # Get time in ns, use fastest TDC of cluster
                mat  =  sortedClusters[s][proj][0][4]
                time-=  self.tdcScifiStationCalib[s][1][proj][mat]  # correct as function of station / projection / mat
                time-=  self.tdcScifiStationCalib[s][0]                  # internal station calibration
                time-=  abs(L)/self.V

# cross check 
               # corTime = self.M.Scifi.GetCorrectedTime(aCl.GetFirst(),aCl.GetTime(),abs(L) )
               # print('test',aCl.GetTime(),time,corTime)

                sTime += time
             stationTimes[s] = [sTime/2.,(sortedClusters[s]['H'][0][5] + sortedClusters[s]['V'][0][5])/2.]
          # require station 1 to be present which defines T0
          if not 1 in stationTimes: continue

          T0 = stationTimes[1][0]
          Z0 = stationTimes[1][1]
          for s in stationTimes:
             if s == 1: continue
             dZ = stationTimes[s][1]-Z0
             dT = stationTimes[s][0]-T0
             dL = dZ * ROOT.TMath.Sqrt( slopeX**2+slopeY**2+1 )
             # if slopeY>0.1:  dL = -dL     # cosmics from the back
             # dT -= dL / u.speedOfLight
             rc = h['dTvsZ'].Fill(dL,dT)
             if abs(slopeX)<0.1 and abs(slopeY)<0.1:
                  rc = h['dTvsZ_beam'].Fill(dL,dT)



   def Plot(self):
      h = self.M.h
      ut.bookCanvas(h,'Tscifi','',1800,1200,2,1)
      h['Tscifi'].cd(1)
      h['dTvsZ'].Draw('colz')
      h['Tscifi'].cd(2)
      h['dTvsZ_beam'].Draw('colz')

class histStore():
     h = {}
     xCheck = ''

if __name__ == '__main__':
   import Monitor
   import SndlhcTracking
   from argparse import ArgumentParser
   parser = ArgumentParser()
   parser.add_argument("--server", dest="server", help="xrootd server",default=os.environ["EOSSHIP"])
   parser.add_argument("-r", "--runNumber", dest="runNumber", help="run number", type=int,default=-1)
   parser.add_argument("-p", "--path", dest="path", help="path to data",required=False,default="/eos/experiment/sndlhc/convertedData/physics/2022/")
   parser.add_argument("-P", "--partition", dest="partition", help="partition of data", type=int,required=False,default=-1)
   parser.add_argument("-f", "--inputFile", dest="fname", help="file name", type=str,default=None,required=False)
   parser.add_argument("-g", "--geoFile", dest="geoFile", help="file name", type=str,default="geofile_sndlhc_TI18_V0_2022.root",required=False)
   parser.add_argument("-c", "--command", dest="command", help="command",required=False,default="")
   parser.add_argument("-o", dest="check", help="use corrected times",action='store_true',default=False)
   parser.add_argument("-M", "--online", dest="online", help="online mode",default=False,action='store_true')
   parser.add_argument("--interactive", dest="interactive", action='store_true',default=False)
   parser.add_argument("-n", "--nEvents", dest="nEvents", help="number of events", default=-1,type=int)

   options = parser.parse_args()
   options.trackType = 'Scifi'

   FairTasks = []
   trackTask = SndlhcTracking.Tracking() 
   trackTask.SetName('simpleTracking')
   FairTasks.append(trackTask)
   M = Monitor.Monitoring(options,FairTasks)
   nMats = M.Scifi.GetConfParI("Scifi/nmats")
   nStations = M.Scifi.GetConfParI("Scifi/nscifi")

   if options.nEvents <0: options.nEvents = M.GetEntries()
   if options.command == "full":
       task = Scifi_CTR()
       M.iteration = 'v0'
       task.Init(M)
       for n in range(options.nEvents):
            event = M.GetEvent(n)
            task.ExecuteEvent(event)
       task.Plot()
       task.minimize(b="")   # all tracks, not yet enough stats for beam tracks
# cross check and station alignment
       M.iteration = 'v1'
       task.Init(M)
       for n in range(options.nEvents):
            event = M.GetEvent(n)
            task.ExecuteEvent(event)
       task.Plot()
# nw minimize the station alignments
       task.minimizeStation(b="")

# cross check
       M.iteration = 'v2'
       task.Init(M)
       for n in range(options.nEvents):
            event = M.GetEvent(n)
            task.ExecuteEvent(event)
       task.Plot()
       ut.writeHists(M.h,'ScifiTimeCalibration.root',plusCanvas=True)

       for s in range(1, nStations+1):
          C = task.tdcScifiStationCalib[s]
          if nMats == 1 :
            print ("station %1i  %5.2F  H: %5.2F   V:  %5.2F   "%( s, C[0],C[1]['H'][0],C[1]['V'][0] ) )
          if nMats == 3 :
            print ("station %1i  %5.2F  H: %5.2F  %5.2F  %5.2F   V:  %5.2F  %5.2F  %5.2F "%( s, C[0],C[1]['H'][0], C[1]['H'][1], C[1]['H'][2],C[1]['V'][0], C[1]['V'][1], C[1]['V'][2] ) )
       for s in range(1, nStations+1):
          C = task.tdcScifiStationCalib[s]
          if nMats == 1 :
            print ("station %1i  %5.3F*u.ns,  %5.3F*u.ns,  %5.3F*u.ns "%( s, C[0],C[1]['H'][0],C[1]['V'][0] ) )
          if nMats == 3 :
            print ("station %1i  %5.3F*u.ns,  %5.3F*u.ns,  %5.3F*u.ns,  %5.3F*u.ns,   %5.3F*u.ns,  %5.3F*u.ns,  %5.3F*u.ns "%( s, C[0],C[1]['H'][0], C[1]['H'][1], C[1]['H'][2],C[1]['V'][0], C[1]['V'][1], C[1]['V'][2] ) )

if options.command == "full" or options.command == "check":
# final test
       taskT = Scifi_TimeOfTracks()
       taskT.Init(M)
       for n in range(options.nEvents):
            event = M.GetEvent(n)
            taskT.ExecuteEvent(event)
       taskT.Plot()
       for s in range(1, nStations+1):
          C = taskT.tdcScifiStationCalib[s]
          if nMats == 1 :
            print ("station %1i  %5.2F  H: %5.2F   V:  %5.2F   "%( s, C[0],C[1]['H'][0],C[1]['V'][0] ) )
          if nMats == 3 :
            print ("station %1i  %5.2F  H: %5.2F  %5.2F  %5.2F   V:  %5.2F  %5.2F  %5.2F "%( s, C[0],C[1]['H'][0], C[1]['H'][1], C[1]['H'][2],C[1]['V'][0], C[1]['V'][1], C[1]['V'][2] ) )
       for s in range(1, nStations+1):
          C = taskT.tdcScifiStationCalib[s]
          if nMats == 1 :
            print ("station %1i  %5.3F*u.ns,  %5.3F*u.ns,  %5.3F*u.ns "%( s, C[0],C[1]['H'][0],C[1]['V'][0] ) )
          if nMats == 3 :
            print ("station %1i  %5.3F*u.ns,  %5.3F*u.ns,  %5.3F*u.ns,  %5.3F*u.ns,   %5.3F*u.ns,  %5.3F*u.ns,  %5.3F*u.ns "%( s, C[0],C[1]['H'][0], C[1]['H'][1], C[1]['H'][2],C[1]['V'][0], C[1]['V'][1], C[1]['V'][2] ) )

