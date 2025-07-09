#!/usr/bin/env python
import ROOT,os,sys
import rootUtils as ut
import shipunit as u
import numpy as np

A,B  = ROOT.TVector3(),ROOT.TVector3()
detector = "mufi-"
# a dictionary to keep track of US sides with gel/no gel
# L first, next R as usual
US_gel_dict = {20:["no GEL", "with GEL"],
               21:["with GEL", "no GEL"],
               22:["no GEL", "with GEL"],
               23:["with GEL", "no GEL"],
               24:["with GEL", "no GEL"]}

class Mufi_hitMaps(ROOT.FairTask):
   " produce hitmaps for MuFilter, Veto/US/DS"
   """
  veto system 2 layers with 7 bars and 8 sipm channels on both ends
              1 layer with 7 bars and 8 sipms on the top
  US system 5 layers with 10 bars and 8 sipm channels on both ends
  DS system horizontal(3) planes, 60 bars, readout on both sides, single channel
                          vertical(4) planes, 60 bar, readout on top, single channel
   """
   def Init(self,options,monitor):
       self.M = monitor
       sdict = self.M.sdict
       h = self.M.h
       run = ROOT.FairRunAna.Instance()
       self.trackTask = self.M.trackTask
       if not self.trackTask: self.trackTask = run.GetTask('houghTransform')
       ioman = ROOT.FairRootManager.Instance()
       self.OT = ioman.GetSink().GetOutTree()
       self.mufi_vsignal = self.M.Scifi.GetConfParF("Scifi/signalSpeed")
       
       channelsPerSystem = {1:self.M.MuFilter.GetConfParI("MuFilter/VetonSiPMs"),
                            2:self.M.MuFilter.GetConfParI("MuFilter/UpstreamnSiPMs"),
                            3:self.M.MuFilter.GetConfParI("MuFilter/DownstreamnSiPMs")}

# type of crossing, check for b1only,b2nob1,nobeam
       if self.M.fsdict or self.M.hasBunchInfo:   self.xing = {'':True,'B1only':False,'B2noB1':False,'noBeam':False}
       else:   self.xing = {'':True}
       for xi in self.xing:
         ut.bookHist(h,detector+'Noise'+xi,'events with hits in single plane; s*10+l;',40,0.5,39.5)         
         for s in monitor.systemAndPlanes:
            ut.bookHist(h,sdict[s]+'Mult'+xi,'QDCs vs nr hits; #hits; QDC [a.u.]',200,0.,800.,200,0.,300.)
            for l in range(monitor.systemAndPlanes[s]):
                  ut.bookHist(h,detector+'hitmult_'+str(s*10+l)+xi,'hit mult / plane '+sdict[s]+str(l)+'; #hits',61,-0.5,60.5)
                  ut.bookHist(h,detector+'hit_'+str(s*10+l)+xi,'channel map / plane '+sdict[s]+str(l)+'; #channel',160,-0.5,159.5)
                  ut.bookHist(h,detector+'Xhit_'+str(s*10+l)+xi,'Xchannel map / plane '+sdict[s]+str(l)+'; #channel',160,-0.5,159.5)

                  # Only large SiPMs will be monitored.
                  # Small SiPMs suffer from large time offsets extending to other events!
                  note = ''
                  NSmallSiPMs = 0
                  if s==2: 
                    note = ', large only'
                    NSmallSiPMs = 2
                  ut.bookHist(h,detector+'chanActiveRight_'+str(s*10+l),
                              sdict[s]+' '+str(l)+'R channel hit multiplicity; #channel; bar',
                              channelsPerSystem[s],-0.5,channelsPerSystem[s]-0.5,
                              monitor.systemAndBars[s],-0.5,monitor.systemAndBars[s]-0.5)
                  ut.bookHist(h,detector+'chanNfiredRight_'+str(s*10+l),
                              sdict[s]+' '+str(l)+'R number of fired channels '+note+'; N fired channels; bar',
                              channelsPerSystem[s]+1-NSmallSiPMs,-0.5,channelsPerSystem[s]+0.5-NSmallSiPMs,
                              monitor.systemAndBars[s],-0.5,monitor.systemAndBars[s]-0.5)
                  side = 'L'
                  if (s==1 and l==2) or (s==3 and (l%2==1 or l==6)):
                    side = 'T'
                  ut.bookHist(h,detector+'chanActiveLeft_'+str(s*10+l),
                              sdict[s]+' '+str(l)+side+' channel hit multiplicity; #channel; bar',
                              channelsPerSystem[s],-0.5,channelsPerSystem[s]-0.5,
                              monitor.systemAndBars[s],-0.5,monitor.systemAndBars[s]-0.5)
                  ut.bookHist(h,detector+'chanNfiredLeft_'+str(s*10+l),
                              sdict[s]+' '+str(l)+side+' number of fired channels '+note+'; N fired channels; bar',
                              channelsPerSystem[s]+1-NSmallSiPMs,-0.5,channelsPerSystem[s]+0.5-NSmallSiPMs,
                              monitor.systemAndBars[s],-0.5,monitor.systemAndBars[s]-0.5)

                  if s==3:  
                        ut.bookHist(h,detector+'bar_'+str(s*10+l)+xi,'bar map / plane '+sdict[s]+str(l)+'; bar',60,-0.5,59.5)
                        ut.bookHist(h,detector+'dT_'+str(s*10+l)+xi,'dT with respect to first scifi '+sdict[s]+str(l)+'; dt [ns] ;# bar + channel',      100,-25.,5.,120,-0.5,2*60-0.5)
                        ut.bookHist(h,detector+'dTcor_'+str(s*10+l)+xi,'dTcor with respect to first scifi '+sdict[s]+str(l)+'; dt [ns] ;# bar + channel',100,-25.,5.,120,-0.5,2*60-0.5)
                        if l == 4:
                          for ss in range(1,6):
                             ut.bookHist(h,'deltaTScifiMufiHit_'+str(ss)+xi,'deltaT scifi earliest hit versus DS hit 2H',200,-25.,25.)
                  else:       
                        ut.bookHist(h,detector+'bar_'+str(s*10+l)+xi,'bar map / plane '+sdict[s]+str(l)+'; bar',10,-0.5,9.5)
                        if s==1:
                           ut.bookHist(h,detector+'dT_'+str(s*10+l)+xi,'dT with respect to first scifi '+sdict[s]+str(l)+'; dt [ns] ;# bar + channel',      100,-25.,5.,120,-0.5,2*8*7-0.5)
                        ut.bookHist(h,detector+'dTcor_'+str(s*10+l)+xi,'dTcor with respect to first scifi '+sdict[s]+str(l)+'; dt [ns] ;# bar + channel',100,-25.,5.,120,-0.5,2*8*7-0.5)
                  ut.bookHist(h,detector+'sig_'+str(s*10+l)+xi,'signal / plane '+sdict[s]+str(l)+'; QDC [a.u.]',200,0.0,200.)
                  if s==2:    
                      ut.bookHist(h,detector+'sigS_'+str(s*10+l)+xi,'signal / plane '+sdict[s]+str(l)+'; QDC [a.u.]',200,0.0,200.)
                      ut.bookHist(h,detector+'TsigS_'+str(s*10+l)+xi,'signal / plane '+sdict[s]+str(l)+'; QDC [a.u.]',200,0.0,200.)
                      tagL = US_gel_dict[s*10+l][0]+": "
                      tagR = US_gel_dict[s*10+l][1]+": "
                  else:
                      tagL = ""
                      tagR = ""
                  histo_title_helper = 'signal / plane '+sdict[s]+str(l)
                  ut.bookHist(h,detector+'sigL_'+str(s*10+l)+xi,tagL+histo_title_helper+'; QDC [a.u.]',200,0.0,200.)
                  ut.bookHist(h,detector+'sigR_'+str(s*10+l)+xi,tagR+histo_title_helper+'; QDC [a.u.]',200,0.0,200.)
                  ut.bookHist(h,detector+'Tsig_'+str(s*10+l)+xi,histo_title_helper+'; QDC [a.u.]',200,0.0,200.)
                  ut.bookHist(h,detector+'TsigL_'+str(s*10+l)+xi,tagL+histo_title_helper+'; QDC [a.u.]',200,0.0,200.)
                  ut.bookHist(h,detector+'TsigR_'+str(s*10+l)+xi,tagR+histo_title_helper+'; QDC [a.u.]',200,0.0,200.)
                  # not used currently?
                  ut.bookHist(h,detector+'occ_'+str(s*10+l)+xi,'channel occupancy '+sdict[s]+str(l),100,0.0,200.)
                  ut.bookHist(h,detector+'occTag_'+str(s*10+l)+xi,'channel occupancy '+sdict[s]+str(l),100,0.0,200.)

                  ut.bookHist(h,detector+'leftvsright_1'+xi,'Veto hits in left / right; Left: # hits; Right: # hits',10,-0.5,9.5,10,-0.5,9.5)
                  ut.bookHist(h,detector+'leftvsright_2'+xi,'US hits in left / right; L: # hits; R: # hits',10,-0.5,9.5,10,-0.5,9.5)
                  ut.bookHist(h,detector+'leftvsright_3'+xi,'DS hits in left / right; L: # hits; R: # hits',2,-0.5,1.5,2,-0.5,1.5)
                  ut.bookHist(h,detector+'leftvsright_signal_1'+xi,'Veto signal in left / right; Left: QDC [a.u.]; Right: QDC [a.u.]',100,-0.5,200.,100,-0.5,200.)
                  ut.bookHist(h,detector+'leftvsright_signal_2'+xi,'US signal in left / right; L: QDC [a.u.]; R: QDC [a.u.]',100,-0.5,200.,100,-0.5,200.)
                  ut.bookHist(h,detector+'leftvsright_signal_3'+xi,'DS signal in left / right; L: QDC [a.u.]; R: QDC [a.u.]',100,-0.5,200.,100,-0.5,200.)

                  ut.bookHist(h,detector+'dtime'+xi,'delta event time; dt [ns]',100,0.0,1000.)
                  ut.bookHist(h,detector+'dtimeu'+xi,'delta event time; dt [us]',100,0.0,1000.)
                  ut.bookHist(h,detector+'dtimem'+xi,'delta event time; dt [ms]',100,0.0,1000.)

                  ut.bookHist(h,detector+'bs'+xi,'beam spot; x[cm]; y[cm]',100,-100.,10.,100,0.,80.)
                  ut.bookHist(h,detector+'bsDS'+xi,'beam spot,#bar X, #bar Y',60,-0.5,59.5,60,-0.5,59.5)
                  ut.bookHist(h,detector+'slopes'+xi,'muon DS track slopes; slope X [rad]; slope Y [rad]',150,-1.5,1.5,150,-1.5,1.5)
                  ut.bookHist(h,detector+'trackPos'+xi,'muon DS track pos; x [cm]; y [cm]',100,-90,10.,80,0.,80.)
                  ut.bookHist(h,detector+'trackPosBeam'+xi,'beam track pos slopes<0.1rad; x [cm]; y [cm]',100,-90,10.,80,0.,80.)

                  for bar in range(monitor.systemAndBars[s]):
                     ut.bookHist(h,detector+'chanmult_'+str(s*1000+100*l+bar)+xi,'channels firing per bar '+sdict[s]+str(l)+" bar "+str(bar)+'; fired channels',20,-0.5,19.5)
#
                  xmin = options.Mufixmin
                  xmax = -xmin
                  ut.bookHist(h,detector+'resX_'+sdict[s]+str(s*10+l)+xi,'residual X'+str(s*10+l)+'; [#cm]',
                      100,xmin,xmax,60,-60.,0.)
                  ut.bookHist(h,detector+'resY_'+sdict[s]+str(s*10+l)+xi,'residual  Y'+str(s*10+l)+'; [#cm]',
                      100,xmin,xmax,70,2.,68.)

       for x in h:
         if isinstance(h[x], ROOT.TH2):
           h[x].SetStats(0)

       self.listOfHits = {1:[],2:[],3:[]}
   def ExecuteEvent(self,event):
       systemAndPlanes =self.M.systemAndPlanes
       sdict = self.M.sdict
       h = self.M.h
       mult = {}
       planes = {}
       for i in self.listOfHits:  self.listOfHits[i].clear()
       for s in systemAndPlanes:
           for l in range(systemAndPlanes[s]):   mult[s*10+l]=0

       self.beamSpot(event)
       withDSTrack = False
       for aTrack in self.M.Reco_MuonTracks:
           if aTrack.GetUniqueID()==3: withDSTrack = True

       for aHit in event.Digi_MuFilterHits:
           Minfo = self.M.MuFilter_PlaneBars(aHit.GetDetectorID())
           s,l,bar = Minfo['station'],Minfo['plane'],Minfo['bar']
           nSiPMs = aHit.GetnSiPMs()
           nSides  = aHit.GetnSides()
           for c in aHit.GetAllSignals(False,False):
                if aHit.isMasked(c.first):
                     channel = bar*nSiPMs*nSides + c.first
                     self.M.fillHist1(detector+'Xhit_'+str(s)+str(l),channel)

           if not aHit.isValid(): continue
           mult[s*10+l]+=1
           key = s*100+l
           if not key in planes: planes[key] = {}
           sumSignal = self.M.map2Dict(aHit,'SumOfSignals')
           planes[key][bar] = [sumSignal['SumL'],sumSignal['SumR']]
# check left/right
           allChannels = self.M.map2Dict(aHit,'GetAllSignals')
           for c in allChannels:
               self.listOfHits[s].append(allChannels[c])
           Nleft,Nright,Sleft,Sright = 0,0,0,0
           # count the small SiPMs seperately
           NSmallLeft, NSmallRight = 0,0
           for c in allChannels:
              if  nSiPMs > c:  # left side
                    Nleft+=1
                    if s==2 and (c==2 or c==5): NSmallLeft+=1
                    Sleft+=allChannels[c]
                    h[detector+'chanActiveLeft_'+str(s*10+l)].Fill(c, bar)
              else:
                    Nright+=1
                    if s==2 and (c==10 or c==13): NSmallRight+=1
                    Sright+=allChannels[c]
                    h[detector+'chanActiveRight_'+str(s*10+l)].Fill(c-nSiPMs, bar)                    
           self.M.fillHist1(detector+'chanmult_'+str(s*1000+100*l+bar),Nleft)
           self.M.fillHist1(detector+'chanmult_'+str(s*1000+100*l+bar),10+Nright)
           h[detector+'chanNfiredLeft_'+str(s*10+l)].Fill(Nleft-NSmallLeft, bar)
           h[detector+'chanNfiredRight_'+str(s*10+l)].Fill(Nright-NSmallRight, bar)
           if not aHit.isVertical():  # vertical DS plane is read out only on one side
              self.M.fillHist2(detector+'leftvsright_'+str(s),Nleft,Nright)
              self.M.fillHist2(detector+'leftvsright_signal_'+str(s),Sleft,Sright)
#
           for c in allChannels:
               channel = bar*nSiPMs*nSides + c
               self.M.fillHist1(detector+'hit_'+str(s)+str(l),int(channel))
               self.M.fillHist1(detector+'bar_'+str(s)+str(l),bar)
               if s==2 and self.M.smallSiPMchannel(c) : 
                     self.M.fillHist1(detector+'sigS_'+str(s)+str(l),allChannels[c])
                     if withDSTrack: self.M.fillHist1(detector+'TsigS_'+str(s)+str(l),allChannels[c])
               elif c<nSiPMs: 
                     self.M.fillHist1(detector+'sigL_'+str(s)+str(l),allChannels[c])
                     if withDSTrack: self.M.fillHist1(detector+'TsigL_'+str(s)+str(l),allChannels[c])
               else             :             
                     self.M.fillHist1(detector+'sigR_'+str(s)+str(l),allChannels[c])
                     if withDSTrack: self.M.fillHist1(detector+'sigR_'+str(s)+str(l),allChannels[c])
               self.M.fillHist1(detector+'sig_'+str(s)+str(l),allChannels[c])
               if withDSTrack:  self.M.fillHist1(detector+'sig_'+str(s)+str(l),allChannels[c])
           allChannels.clear()
#
       # noise event with many hits in one plane
       onePlane = []
       for x in mult:
           if mult[x]>3: onePlane.append(x)
       if len(onePlane)==1:
           self.M.fillHist1(detector+'Noise',onePlane[0])

#
       for s in self.listOfHits:
           nhits = len(self.listOfHits[s])
           qcdsum = 0
           for i in range(nhits):
               self.M.fillHist2(sdict[s]+'Mult',nhits, self.listOfHits[s][i])
       for s in systemAndPlanes:
          for l in range(systemAndPlanes[s]):   
             self.M.fillHist1(detector+'hitmult_'+str(s*10+l),mult[s*10+l])
# mufi residuals with scifi tracks
       for aTrack in self.M.Reco_MuonTracks:
           if not aTrack.GetUniqueID()==1: continue
           fitStatus = aTrack.getFitStatus()
           if not fitStatus.isFitConverged(): continue
           posMom = {}
           fstate =  aTrack.getFittedState()
           posMom['first'] = [fstate.getPos(),fstate.getMom()]
           # fstate =  aTrack.getFittedState(aTrack.getNumPointsWithMeasurement()-1) does not make a difference
           posMom['last'] = [fstate.getPos(),fstate.getMom()]
           rc = self.trackTask.trackDir(aTrack)
           scifi_time0 = rc[2]
           pos,mom = posMom['first']
           lam = (self.trackTask.firstScifi_z-pos.z())/mom.z()
           # nominal first position
           pos1 = ROOT.TVector3(pos.x()+lam*mom.x(),pos.y()+lam*mom.y(),self.trackTask.firstScifi_z)
           dsHitTimes = []
           for aHit in event.Digi_MuFilterHits:
              if not aHit.isValid(): continue
              detID = aHit.GetDetectorID()
              Minfo = self.M.MuFilter_PlaneBars(detID)
              s,l,bar = Minfo['station'],Minfo['plane'],Minfo['bar']
              self.M.MuFilter.GetPosition(detID,A,B)
# calculate DOCA
              if s==1: pos,mom = posMom['first']
              else: pos,mom = posMom['last']
              zEx = self.M.zPos['MuFilter'][s*10+l]
              lam = (zEx-pos.z())/mom.z()
              xEx,yEx = pos.x()+lam*mom.x(),pos.y()+lam*mom.y()
              pq = A-pos
              uCrossv= (B-A).Cross(mom)
              doca = pq.Dot(uCrossv)/uCrossv.Mag()
              self.M.fillHist2(detector+'resX_'+sdict[s]+str(s*10+l),doca/u.cm,xEx)
              self.M.fillHist2(detector+'resY_'+sdict[s]+str(s*10+l),doca/u.cm,yEx)
# calculate time difference for DS
              if (s==3 and abs(doca)<2.5*u.cm) or (s==1 and abs(doca)<6*u.cm):
                 # horizontal layers have left and right sipms
                 if aHit.isVertical(): nmax = 1
                 else: nmax = 2
                 barMult = 2
                 if s==1: 
                     nmax = 16
                     barMult = 16
                 for i in range(nmax):
                   if aHit.GetTime(i) < 0: continue # not valid time
                   posM = ROOT.TVector3(xEx,yEx,zEx)
                 # correct for flight length
                   trajLength = (posM-pos1).Mag()
                 # correct for signal speed, need to know left or right
                   if s==3:
                     if i==1:                      X = B-posM   # B is right  only horizontal planes have a second readout 
                     else:                         X = A-posM   # A is on the left, or top for vertical planes
                   if s==1:
                     if i<8:                       X = A-posM  
                     else:                         X = B-posM  
                   L = X.Mag()/self.mufi_vsignal
                   tM = aHit.GetTime(i)*self.M.TDC2ns - L - trajLength/u.speedOfLight
                   self.M.fillHist2(detector+'dT_'+str(s*10+l),tM-scifi_time0,bar*barMult+i)
                   # use corrected time
                   if s==3:
                     corTime = self.M.MuFilter.GetCorrectedTime(detID, i, aHit.GetTime(i)*self.M.TDC2ns, X.Mag())
                     tM = corTime - trajLength/u.speedOfLight
                     self.M.fillHist2(detector+'dTcor_'+str(s*10+l),tM-scifi_time0,bar*barMult+i)
                 if s==3 and l==2:
                   timeLeft = aHit.GetTime(0)
                   timeRight = aHit.GetTime(1)
                   if timeLeft>0 and timeRight>0:
                      dL = abs(A[0]-B[0])
                      avTime = self.M.MuFilter.GetCorrectedTime(detID, 0, timeLeft*self.M.TDC2ns,0) + \
                               self.M.MuFilter.GetCorrectedTime(detID, 1, timeRight*self.M.TDC2ns,0)
                      dsHitTimes.append( (avTime-abs(A[0]-B[0])/15)/2) # L/2 / 15cm/ns
# fill histograms with time difference of earliest scifi hit in station i and last horizontal DS time, average left and right
           if len(dsHitTimes)>0:
            dsHitTimes.sort()
            scifiHitTimes = {1:[],2:[],3:[],4:[],5:[]}
            for scifiHit in event.Digi_ScifiHits:
              detID = scifiHit.GetDetectorID()
              s = int(scifiHit.GetDetectorID()/1000000)
              scifiHitTimes[s].append(self.M.Scifi.GetCorrectedTime(detID,scifiHit.GetTime()*self.M.TDC2ns,0))
            for s in scifiHitTimes:
                if len(scifiHitTimes[s])<1: continue
                scifiHitTimes[s].sort()
                deltaT = dsHitTimes[0] - scifiHitTimes[s][0] - (self.M.zPos['MuFilter'][34]-self.M.zPos['Scifi'][s*10])/u.speedOfLight
                self.M.fillHist1('deltaTScifiMufiHit_'+str(s),deltaT)
   def beamSpot(self,event):
      if not self.trackTask: return
      h = self.M.h
      W = self.M.Weight
      Xbar = -10
      Ybar = -10
      for aTrack in self.M.Reco_MuonTracks:
         if not aTrack.GetUniqueID()==3: continue
         state = aTrack.getFittedState()
         pos    = state.getPos()
         rc = h[detector+'bs'].Fill(pos.x(),pos.y(),W)
         mom = state.getMom()
         slopeX= mom.X()/mom.Z()
         slopeY= mom.Y()/mom.Z()
         pos = state.getPos()

         self.M.fillHist2(detector+'slopes',slopeX,slopeY)
         self.M.fillHist2(detector+'trackPos',pos.X(),pos.Y())
         if abs(slopeX)<0.1 and abs(slopeY)<0.1:  self.M.fillHist2(detector+'trackPosBeam',pos.X(),pos.Y())
         if not Ybar<0 and not Xbar<0 and abs(slopeY)<0.01: self.M.fillHist2(detector+'bsDS',Xbar,Ybar)

   def Plot(self):
      h = self.M.h
      sdict = self.M.sdict
      systemAndPlanes =self.M.systemAndPlanes
      S = {1:[1800,800,systemAndPlanes[1],1],2:[1800,1500,2,3],3:[1800,1800,2,4]}
      for xi in self.xing:
       if not self.M.fsdict and not self.M.hasBunchInfo and xi!='': continue

       for s in S:
           ut.bookCanvas(h,detector+'hitmaps' +sdict[s]+xi,'hitmaps' +sdict[s],S[s][0],S[s][1],S[s][2],S[s][3])
           ut.bookCanvas(h,detector+'Xhitmaps' +sdict[s]+xi,'Xhitmaps' +sdict[s],S[s][0],S[s][1],S[s][2],S[s][3])
           ut.bookCanvas(h,detector+'barmaps'+sdict[s]+xi,'barmaps'+sdict[s],S[s][0],S[s][1],S[s][2],S[s][3])
           if s==3 or s==1: 
               ut.bookCanvas(h,detector+'dTScifi'+sdict[s]+xi,'dt rel to scifi'+sdict[s],S[s][0],S[s][1],S[s][2],S[s][3])
               ut.bookCanvas(h,detector+'dTcorScifi'+sdict[s]+xi,'dtcor rel to scifi'+sdict[s],S[s][0],S[s][1],S[s][2],S[s][3])

           for l in range(systemAndPlanes[s]):
              n = l+1
              if s==3 and n==7: n=8
              tc = h[detector+'hitmaps'+sdict[s]+xi].cd(n)
              tag = str(s)+str(l)+xi
              h[detector+'hit_'+tag].Draw()
              tc = h[detector+'Xhitmaps'+sdict[s]+xi].cd(n)
              h[detector+'Xhit_'+tag].Draw()

              tc = h[detector+'barmaps'+sdict[s]+xi].cd(n)
              h[detector+'bar_'+tag].Draw()
              if s==3 or s==1:
                 tc = h[detector+'dTScifi'+sdict[s]+xi].cd(n)
                 h[detector+'dT_'+tag].Draw('colz')
                 tc = h[detector+'dTcorScifi'+sdict[s]+xi].cd(n)
                 h[detector+'dTcor_'+tag].Draw('colz')

       ut.bookCanvas(h,detector+'hitmult'+xi,'hit multiplicities per plane',2000,1600,4,3)
       k=1
       for s in systemAndPlanes:
           for l in range(systemAndPlanes[s]):
              tc = h[detector+'hitmult'+xi].cd(k)
              tc.SetLogy(1)
              k+=1
              rc = h[detector+'hitmult_'+str(s*10+l)+xi].Draw()
       ut.bookCanvas(h,'noise'+xi,' ',1200,1800,1,1)
       tc = h['noise'+xi].cd()
       h[detector+'Noise'+xi].Draw()

       ut.bookCanvas(h,'VETO'+xi,' ',1200,1800,1,2)
       for l in range(2):
          tc = h['VETO'+xi].cd(l+1)
          hname = detector+'hit_'+str(1)+str(l)+xi
          h[hname].SetStats(0)
          h[hname].Draw()
          for n in range(7):
                x = (n+1)*16-0.5
                y = h[detector+'hit_'+str(1)+str(l)+xi].GetMaximum()
                lname = 'L'+str(n)+hname
                h[lname] = ROOT.TLine(x,0,x,y)
                h[lname].SetLineColor(ROOT.kRed)
                h[lname].SetLineStyle(9)
                h[lname].Draw('same')

       ut.bookCanvas(h,'USBars'+xi,' ',1200,900,1,1)
       colours = {0:ROOT.kOrange,1:ROOT.kRed,2:ROOT.kGreen,3:ROOT.kBlue,4:ROOT.kMagenta,5:ROOT.kCyan,
                  6:ROOT.kAzure,7:ROOT.kPink,8:ROOT.kSpring}
       for i in range(5): 
           h[detector+'bar_2'+str(i)+xi].SetLineColor(colours[i])
           h[detector+'bar_2'+str(i)+xi].SetLineWidth(2)
           h[detector+'bar_2'+str(i)+xi].SetStats(0)
       h[detector+'bar_20'+xi].Draw()
       h[detector+'bar_21'+xi].Draw('same')
       h[detector+'bar_22'+xi].Draw('same')
       h[detector+'bar_23'+xi].Draw('same')
       h[detector+'bar_24'+xi].Draw('same')
       h[detector+'lbar2'+xi]=ROOT.TLegend(0.6,0.6,0.99,0.99)
       for i in range(5): 
            h[detector+'lbar2'+xi].AddEntry(h[detector+'bar_2'+str(i)+xi],'plane '+str(i+1),"f")
       h[detector+'lbar2'+xi].Draw()
       for i in range(7): 
            h[detector+'hit_3'+str(i)+xi].SetLineColor(colours[i])
            h[detector+'hit_3'+str(i)+xi].SetLineWidth(2)
            h[detector+'hit_3'+str(i)+xi].SetStats(0)
       h[detector+'hit_30'+xi].Draw()
       for i in range(1,7):
           h[detector+'hit_3'+str(i)+xi].Draw('same')
       h[detector+'lbar3'+xi]=ROOT.TLegend(0.6,0.6,0.99,0.99)
       for i in range(7): 
           h[detector+'lbar3'+xi].AddEntry(h[detector+'hit_3'+str(i)+xi],'plane '+str(i+1),"f")
           h[detector+'lbar3'+xi].Draw()

       ut.bookCanvas(h,detector+'LR'+xi,' ',1800,900,3,2)
       for i in range(1,4):
          h[detector+'LR'+xi].cd(i)
          h[detector+'leftvsright_'+str(i)+xi].Draw('textBox')
          h[detector+'LR'+xi].cd(i+3)
          h[detector+'leftvsright_signal_'+str(i)+xi].SetMaximum(h[detector+'leftvsright_signal_'+str(i)+xi].GetBinContent(10,10))
          h[detector+'leftvsright_signal_'+str(i)+xi].Draw('colz')

       ut.bookCanvas(h,detector+'LRinEff'+xi,' ',1800,450,3,1)
       for s in range(1,4):
           h[detector+'lLRinEff'+str(s)+xi]=ROOT.TLegend(0.6,0.54,0.99,0.93)
           name = detector+'leftvsright_signal_'+str(s)+xi
           h[name+'0Y'] = h[name].ProjectionY(name+'0Y',1,1)
           h[name+'0X'] = h[name].ProjectionX(name+'0X',1,1)
           h[name+'1X'] = h[name].ProjectionY(name+'1Y')
           h[name+'1Y'] = h[name].ProjectionX(name+'1X')
           tc = h[detector+'LRinEff'+xi].cd(s)
           tc.SetLogy()
           h[name+'0X'].SetStats(0)
           h[name+'0Y'].SetStats(0)
           h[name+'1X'].SetStats(0)
           h[name+'1Y'].SetStats(0)
           h[name+'0X'].SetLineColor(ROOT.kRed)
           h[name+'0Y'].SetLineColor(ROOT.kGreen)
           h[name+'1X'].SetLineColor(ROOT.kMagenta)
           h[name+'1Y'].SetLineColor(ROOT.kCyan)
           h[name+'0X'].SetMaximum(max(h[name+'1X'].GetMaximum(),h[name+'1Y'].GetMaximum()))
           h[name+'0X'].Draw()
           h[name+'0Y'].Draw('same')
           h[name+'1X'].Draw('same')
           h[name+'1Y'].Draw('same')
   # Fill(Sleft,Sright)
           h[detector+'lLRinEff'+str(s)+xi].AddEntry(h[name+'0X'],'left with no signal right',"f")
           h[detector+'lLRinEff'+str(s)+xi].AddEntry(h[name+'0Y'],'right with no signal left',"f")
           h[detector+'lLRinEff'+str(s)+xi].AddEntry(h[name+'1X'],'left all',"f")
           h[detector+'lLRinEff'+str(s)+xi].AddEntry(h[name+'1Y'],'right all',"f")
           h[detector+'lLRinEff'+str(s)+xi].Draw()

       listSipmTypes = ['L','R','S']
       plane_label = {}
       # parameters for the signal median
       q05=np.array([0.5])
       signal_attributes={}
#
       for tag in ["","T"]:
        ut.bookCanvas(h,tag+'signalUSVeto'+xi,' ',1200,1600,3,self.M.systemAndPlanes[1]+self.M.systemAndPlanes[2])
        s = 1
        l = 1
        Xaxis_bin = 1
        for plane in range(self.M.systemAndPlanes[1]):
                for side in listSipmTypes:
                   tc = h[tag+'signalUSVeto'+xi].cd(l)
                   l+=1 
                   if side=='S' or (plane==2 and side =='R'): continue
                   rc = h[detector+tag+'sig'+side+'_'+str( s*10+plane)+xi]
                   rc.Draw()
                   if side=='S' or tag=='T': continue
                   if plane==2: plane_label[Xaxis_bin] = "Veto "+str(plane)+" T" 
                   else: plane_label[Xaxis_bin] = "Veto "+str(plane)+" "+side
                   med=np.array([0.])        
                   rc.GetQuantiles(1,med,q05)
                   signal_attributes[Xaxis_bin]={
                                  "median":med[0],
                                  "std":rc.GetStdDev(),
                                  "max":rc.FindLastBinAbove(0),
                                  "percent_overflow":rc.GetBinContent(rc.GetNbinsX()+1)/rc.Integral()*100. if rc.Integral()>0 else 0.}
                   Xaxis_bin += 1
        s=2
        for plane in range(self.M.systemAndPlanes[2]):
                for side in listSipmTypes:
                   tc = h[tag+'signalUSVeto'+xi].cd(l)
                   l+=1
                   rc = h[detector+tag+'sig'+side+'_'+str( s*10+plane)+xi]
                   rc.Draw()
                   if side=='S' or tag=='T': continue
                   plane_label[Xaxis_bin] = "US "+str(plane)+" "+side
                   med=np.array([0.])
                   rc.GetQuantiles(1,med,q05)
                   signal_attributes[Xaxis_bin]={
                                   "median":med[0],
                                   "std":rc.GetStdDev(),
                                   "max":rc.FindLastBinAbove(0),
                                   "percent_overflow":rc.GetBinContent(rc.GetNbinsX()+1)/rc.Integral()*100. if rc.Integral()>0 else 0.}
                   Xaxis_bin += 1
        ut.bookCanvas(h,tag+'signalDS'+xi,' ',900,1600,2,self.M.systemAndPlanes[3])
        s = 3
        l = 1
        for plane in range(self.M.systemAndPlanes[3]):
               for side in listSipmTypes:
                  if side == 'S': continue
                  tc = h[tag+'signalDS'+xi].cd(l)
                  l+=1
                  if (plane%2==1 or plane==6) and side=='R': continue
                  rc = h[detector+tag+'sig'+side+'_'+str( s*10+plane)+xi]
                  rc.Draw()
                  if () or tag=='T': continue
                  if plane%2==1 or plane==6: plane_label[Xaxis_bin] = "DS "+str(plane//2+1)+" T"
                  else:plane_label[Xaxis_bin] = "DS "+str(plane//2+1)+" "+side
                  med=np.array([0.])
                  rc.GetQuantiles(1,med,q05)
                  signal_attributes[Xaxis_bin]={
                                   "median":med[0],
                                   "std":rc.GetStdDev(),
                                   "max":rc.FindLastBinAbove(0),
                                   "percent_overflow":rc.GetBinContent(rc.GetNbinsX()+1)/rc.Integral()*100. if rc.Integral()>0 else 0.}
                  Xaxis_bin += 1

# summary canvases of the median, maximum and overflow of signals
       ut.bookCanvas(h,detector+'signalsSummary'+xi,' ',1024,768,1,3)
       signal_medians = [ROOT.TGraphErrors(), ROOT.TGraphErrors(), ROOT.TGraphErrors()]
       signal_maxima = [ROOT.TGraphErrors(), ROOT.TGraphErrors(), ROOT.TGraphErrors()]
       signal_overflow = [ROOT.TGraphErrors(), ROOT.TGraphErrors(), ROOT.TGraphErrors()]
       Area = {}
       point_count = {}
       for s in self.M.systemAndPlanes:
         point_count[s-1]=-1
         if s ==1:
           signal_medians[s-1].SetTitle("Median of signal per plane and per side")
           signal_medians[s-1].GetYaxis().SetTitle("median QDC [a.u.]")
           signal_maxima[s-1].SetTitle("Maximal signal per plane and per side")
           signal_maxima[s-1].GetYaxis().SetTitle("maximum QDC [a.u.]")
           signal_overflow[s-1].SetTitle("Overflow/All QDC per plane and per side")
           signal_overflow[s-1].GetYaxis().SetTitle("overflow QDC [%]")
         for item in signal_attributes.keys():
            if s==2 and plane_label[item].find('US')<0 : continue
            if s==3 and plane_label[item].find('DS')<0 : continue
            point_count[s-1] += 1
            signal_medians[s-1].SetPoint(point_count[s-1],item,signal_attributes[item]["median"])
            signal_medians[s-1].SetPointError(point_count[s-1],0,signal_attributes[item]["std"])
            signal_maxima[s-1].SetPoint(point_count[s-1],item,signal_attributes[item]["max"])
            signal_maxima[s-1].SetPointError(point_count[s-1],0,0)
            signal_overflow[s-1].SetPoint(point_count[s-1],item,signal_attributes[item]["percent_overflow"])
            signal_overflow[s-1].SetPointError(point_count[s-1],0,0)
       graph_list = [signal_medians, signal_maxima, signal_overflow]
       for counter, graph in enumerate([item[0] for item in graph_list]):
         h[detector+'signalsSummary'+xi].cd(counter+1)
         ROOT.gPad.SetBottomMargin(0.2)
         ROOT.gPad.SetGrid(0)
         graph.Draw('AP')
         xAxis = graph.GetXaxis()
         # get rid of the original ticks
         xAxis.SetTickLength(0)
         ymin = graph.GetHistogram().GetMinimum()
         ymax = graph.GetHistogram().GetMaximum()
         for index, item in enumerate(signal_attributes.keys()):
           bin_index = xAxis.FindBin(item)
           xAxis.SetBinLabel(bin_index,plane_label[item])
           # Draw custom grid lines for the X axis
           grid = ROOT.TLine(graph.GetPointX(index), ymin, graph.GetPointX(index), ymax)
           grid.SetLineStyle(3)
           grid.DrawClone()
           # Draw ticks
           tick = ROOT.TLine(graph.GetPointX(index), ymin, graph.GetPointX(index), ymin + 0.03*(ymax-ymin))
           tick.DrawClone()         
         # redraw the 1st veto graph and draw the graphs for the US and DS
         graph.Draw('P,same')
         graph_list[counter][1].Draw('P,same')
         graph_list[counter][2].Draw('P,same')
         for s in self.M.systemAndPlanes:
            if s==1: Area[counter]={}
            Area[counter][s-1] = ROOT.TBox(ROOT.TMath.MinElement(graph_list[counter][s-1].GetN(), graph_list[counter][s-1].GetX())-0.5,
                                graph.GetHistogram().GetMinimum(), 
                                ROOT.TMath.MaxElement(graph_list[counter][s-1].GetN(), graph_list[counter][s-1].GetX())+0.5,
                                graph.GetHistogram().GetMaximum())
            Area[counter][s-1].SetLineWidth(0)
            Area[counter][s-1].SetFillStyle(3003)
            Area[counter][s-1].SetFillColor(s+1) 
            Area[counter][s-1].Draw("same")
            graph_list[counter][s-1].SetMarkerStyle(21)
            graph_list[counter][s-1].SetMarkerColor(s+1)
            graph_list[counter][s-1].SetLineColor(s+1)
# end of the summary QDC shifter canvases

       ut.bookCanvas(h,detector+"chanbar"+xi,' ',1800,700,3,1)
       for s in self.M.systemAndPlanes:
            opt = ""
            if s==3: 
               ut.bookCanvas(h,sdict[s]+"chanbar"+xi,' ',1800,1800,12,15)
            else:   
               y = self.M.systemAndPlanes[s]
               ut.bookCanvas(h,sdict[s]+"chanbar"+xi,' ',1800,700,y,self.M.systemAndBars[s])
            h[sdict[s]+"chanbar"+xi].cd(1)
            for l in range(self.M.systemAndPlanes[s]):
               if s==3 and (l==1 or l==3 or l==5 or l==6):continue
               maxN = 0
               for bar in range(self.M.systemAndBars[s]):
                   hname = detector+'chanmult_'+str(s*1000+100*l+bar)+xi
                   nmax = h[hname].GetBinContent(h[hname].GetMaximumBin())
                   if nmax > maxN : maxN = nmax
               for bar in range(self.M.systemAndBars[s]):
                   hname = detector+'chanmult_'+str(s*1000+100*l+bar)+xi
                   h[hname].SetStats(0)
                   # h[hname].SetMaximum(maxN*1.2)
                   h[detector+"chanbar"+xi].cd(s)
                   h[hname].DrawClone(opt)
                   opt="same"
                   i = l+1 + (self.M.systemAndBars[s]-bar-1)*self.M.systemAndPlanes[s]
                   if s==3:
                        ix = bar//15 + 1 + (l//2)*4
                        iy = 15 - bar%15
                        i = ix+(iy-1)*12
                   h[sdict[s]+"chanbar"+xi].cd(i)
                   h[hname].SetMaximum(h[hname].GetBinContent(h[hname].GetMaximumBin())*1.2)
                   h[hname].Draw()
 
# shifter summary plots
       for item in ["Active", "Nfired"]:
         ut.bookCanvas(h,detector+item+'ChannelsPerBarVeto',' ',1024,768,2,3)
         counter = 1
         s = 1
         for l in range(self.M.systemAndPlanes[s]):
           for i in range(2):
             h[detector+item+'ChannelsPerBarVeto'].cd(counter)
             counter += 1
             if l==2 and i==1: continue # vertical planes have readout on the top only
             if i%2==0: 
               h[detector+'chan'+item+'Left_'+str(s*10+l)].Draw("colz")
               h[detector+'chan'+item+'Left_'+str(s*10+l)].SetMinimum(0)
             else:
               h[detector+'chan'+item+'Right_'+str(s*10+l)].Draw("colz")
               h[detector+'chan'+item+'Right_'+str(s*10+l)].SetMinimum(0)
         self.M.myPrint(h[detector+item+'ChannelsPerBarVeto'],detector+item+'ChannelsPerBarUSVeto',subdir='mufilter/shifter')

         ut.bookCanvas(h,detector+item+'ChannelsPerBarUS',' ',1024,768,2,5)
         counter = 1
         s = 2
         for l in range(self.M.systemAndPlanes[s]):
           for i in range(2):
             h[detector+item+'ChannelsPerBarUS'].cd(counter)
             if i%2==0: 
               h[detector+'chan'+item+'Left_'+str(s*10+l)].Draw("colz")
               h[detector+'chan'+item+'Left_'+str(s*10+l)].SetMinimum(0)
             else:
               h[detector+'chan'+item+'Right_'+str(s*10+l)].Draw("colz")
               h[detector+'chan'+item+'Right_'+str(s*10+l)].SetMinimum(0)
             counter += 1
         self.M.myPrint(h[detector+item+'ChannelsPerBarUS'],detector+item+'ChannelsPerBarUSVeto',subdir='mufilter/shifter')

         s=3
         # the DS 'Active' channels plot has the different style than the Veto and US ones
         if item=='Active':
           ut.bookHist(h,detector+'chanActiveDSSummaryHisto', 'DS channel hit multiplicity; ;bar',
                              10, 0, 10,
                              self.M.systemAndBars[s],-0.5,self.M.systemAndBars[s]-0.5)
           h[detector+'chanActiveDSSummaryHisto'].SetStats(0)
           xAxis = h[detector+'chanActiveDSSummaryHisto'].GetXaxis()
           ut.bookCanvas(h,detector+'chanActiveDSSummary',' ',1024,768,1,1)
           counter = 0 # counting histo bins
           for l in range(self.M.systemAndPlanes[s]):
             for i in range(2):# sides left <-> right
               if (l%2==1 or l==6) and i==1: continue# vertical planes have readout on the top only
               counter += 1
               xAxis.SetBinLabel(counter,plane_label[len(plane_label)-10+counter])
               # loop over bars, each DS plane has 60 bars, significant histogram bins start from 1
               for barIndex in range(1,self.M.systemAndBars[s]+1):
                 if i%2==0:
                   h[detector+'chanActiveDSSummaryHisto'].SetBinContent(counter, barIndex,
                                h[detector+'chan'+item+'Left_'+str(s*10+l)].GetBinContent(1, barIndex))
                 else:
                   h[detector+'chanActiveDSSummaryHisto'].SetBinContent(counter, barIndex,
                                h[detector+'chan'+item+'Right_'+str(s*10+l)].GetBinContent(1, barIndex))
           h[detector+'chanActiveDSSummaryHisto'].SetMinimum(0)
           h[detector+'chanActiveDSSummaryHisto'].Draw('colz')
           h[detector+'chanActiveDSSummaryHisto'].SetStats(0)
           self.M.myPrint(h[detector+'chanActiveDSSummary'],detector+'chanActiveDSSummary',subdir='mufilter/shifter')
         # the 'Nfired' channels plot has the same style for all MuFi systems
         else:
           ut.bookCanvas(h,detector+item+'ChannelsPerBarDS',' ',1024,768,3,4)
           counter = 1
           for l in range(self.M.systemAndPlanes[s]):
             for i in range(2):# sides left <-> right
               h[detector+item+'ChannelsPerBarDS'].cd(counter)
               if (l%2==1 or l==6) and i==1: continue# vertical planes have readout on the top only
               if i%2==0: 
                 h[detector+'chan'+item+'Left_'+str(s*10+l)].Draw("colz")
                 h[detector+'chan'+item+'Left_'+str(s*10+l)].SetMinimum(0)
               else:
                 h[detector+'chan'+item+'Right_'+str(s*10+l)].Draw("colz")
                 h[detector+'chan'+item+'Right_'+str(s*10+l)].SetMinimum(0)
               counter += 1
               if l==5 and i==0: counter += 2
           self.M.myPrint(h[detector+item+'ChannelsPerBarDS'],detector+item+'ChannelsPerBarDS',subdir='mufilter/shifter')
#expert plots
       canvas = detector+'signalsSummary'+xi
       self.M.h[canvas].Update()
       if xi!='': self.M.myPrint(self.M.h[canvas],canvas,subdir='mufilter/shifter/'+xi)
       else:     self.M.myPrint(self.M.h[canvas],canvas,subdir='mufilter/shifter')
       for canvas in ['signalUSVeto'+xi,'signalDS'+xi,detector+'LR'+xi,'USBars'+xi,
                     "Vetochanbar"+xi,"USchanbar"+xi,"DSchanbar"+xi,'noise'+xi]:
              h[canvas].Update()
              if x!='': self.M.myPrint(h[canvas],canvas,subdir='mufilter/expert/'+xi)
              else: self.M.myPrint(h[canvas],canvas,subdir='mufilter/expert')
       for canvas in [detector+'hitmaps',detector+'Xhitmaps',detector+'barmaps',detector+'dTScifi',detector+'dTcorScifi']:
              for s in range(1,4):
                  if s<3 and canvas.find('dT')>0: continue
                  h[canvas+sdict[s]+xi].Update()
                  if x!='': self.M.myPrint(h[canvas+sdict[s]+xi],canvas+sdict[s],subdir='mufilter/expert/'+xi)
                  else: self.M.myPrint(h[canvas+sdict[s]+xi],canvas+sdict[s],subdir='mufilter/expert')

# tracking
       ut.bookCanvas(h,"muonDSTracks"+xi,' ',1200,400,3,1)
       tc = h["muonDSTracks"+xi].cd(1)
       h[detector+'slopes'+xi].Draw('colz')
       tc = h["muonDSTracks"+xi].cd(2)
       rc = h[detector+'slopes'+xi].ProjectionX("slopeX"+xi)
       rc.Draw()
       rc.SetTitle('track Y slope')
       tc = h["muonDSTracks"+xi].cd(3)
       rc = h[detector+'slopes'+xi].ProjectionY("slopeY"+xi)
       rc.Draw()
       rc.SetTitle('track Y slope')

       ut.bookCanvas(h,detector+'TtrackPos'+xi,"track position first state",600,1200,1,2)
       h[detector+'TtrackPos'+xi].cd(1)
       rc = h[detector+'trackPosBeam'+xi].Draw('colz')
       h[detector+'TtrackPos'+xi].cd(2)
       rc = h[detector+'trackPos'+xi].Draw('colz')
       if x!='': 
           self.M.myPrint(h["muonDSTracks"+xi],"muonDSTrackdirection"+xi,subdir='mufilter/shifter/'+xi)
           self.M.myPrint(self.M.h[detector+'TtrackPos'+xi],detector+'trackPos'+xi,subdir='mufilter/shifter/'+xi)
       else: 
           self.M.myPrint(h["muonDSTracks"+xi],"muonDSTrackdirection"+xi,subdir='mufilter/shifter')
           self.M.myPrint(self.M.h[detector+'TtrackPos'+xi],detector+'trackPos'+xi,subdir='mufilter/shifter')

# residuals
       # fit all mufi planes in the canvas depending on # veto planes
       nh, nv = self.M.systemAndPlanes[1],15//self.M.systemAndPlanes[1]
       ut.bookCanvas(h,detector+"residualsVsX"+xi,'residualsVsX ',1200,1200, nh, nv)
       ut.bookCanvas(h,detector+"residualsVsY"+xi,'residualsVsY ',1200,1200, nh, nv)
       ut.bookCanvas(h,detector+"residuals"+xi,'residuals',1200,1200, nh, nv)
# veto 2 H planes veto 1 V plane US 5 H planes DS 3 H planes  DS 4 V planes = 15 in total
       for p in ['resX_','resY_']:
          t = detector+"residualsVs"+p.replace('res','').replace('_','')+xi
          i = 1
          for s in range(1,4):
             for l in range(7): 
                if s==1 and l>self.M.systemAndPlanes[1]-1: continue
                if s==2 and l>self.M.systemAndPlanes[2]-1: continue
                tc = h[t].cd(i)
                hname = detector+p+sdict[s]+str(s*10+l)+xi
                h[hname].Draw('colz')
                if p.find('X')>0:
                    tc = h[detector+"residuals"+xi].cd(i)
                    h[hname+'proj']=h[hname].ProjectionX(hname+'proj')
                    rc = h[hname+'proj'].Fit('gaus','SQ')
                    fitResult = rc.Get()
                    if fitResult: 
                        tc.Update()
                        stats = h[hname+'proj'].FindObject('stats')
                        stats.SetOptFit(1111111)
                        stats.SetX1NDC(0.68)
                        stats.SetY1NDC(0.31)
                        stats.SetX2NDC(0.98)
                        stats.SetY2NDC(0.94)
                        h[hname+'proj'].Draw()
                i+=1
       if x!='':
         self.M.myPrint(self.M.h[detector+'residualsVsX'+xi],detector+'residualsVsX',subdir='mufilter/expert/'+xi)
         self.M.myPrint(self.M.h[detector+'residualsVsY'+xi],detector+'residualsVsY',subdir='mufilter/expert/'+xi)
         self.M.myPrint(self.M.h[detector+'residuals'+xi],detector+'residuals',subdir='mufilter/expert/'+xi)
       else:
         self.M.myPrint(self.M.h[detector+'residualsVsX'+xi],detector+'residualsVsX',subdir='mufilter/expert')
         self.M.myPrint(self.M.h[detector+'residualsVsY'+xi],detector+'residualsVsY',subdir='mufilter/expert')
         self.M.myPrint(self.M.h[detector+'residuals'+xi],detector+'residuals',subdir='mufilter/expert')
         
       ut.bookCanvas(self.M.h,'dt'+xi,'',1200,1200,1,2)
       self.M.h['dt'].cd(1)
       self.M.h['deltaTScifiMufiHit_'+str(1)].Draw('hist')
       for s in range(1,6):
          self.M.h['deltaTScifiMufiHit_'+str(s)].SetStats(0)
          self.M.h['deltaTScifiMufiHit_'+str(s)].SetLineColor(s+1)
          self.M.h['deltaTScifiMufiHit_'+str(s)].Draw('samehist')
       self.M.h['dt'].cd(2)
       if 'B2noB1' in self.xing:
        self.M.h['deltaTScifiMufiHit_'+str(1)+'B2noB1'].Draw('hist')
        for s in range(1,6):
          self.M.h['deltaTScifiMufiHit_'+str(s)+'B2noB1'].SetStats(0)
          self.M.h['deltaTScifiMufiHit_'+str(s)+'B2noB1'].SetLineColor(s+1)
          self.M.h['deltaTScifiMufiHit_'+str(s)+'B2noB1'].Draw('samehist')
       self.M.myPrint(self.M.h['dt'+xi],'scifi DS hit difference',subdir='mufilter/expert')

class Mufi_largeVSsmall(ROOT.FairTask):
   """
    make correlation plots of small and large sipms for US and Veto"
   """
   def Init(self,options,monitor):
       self.M = monitor
       sdict = self.M.sdict
       h = self.M.h
       run = ROOT.FairRunAna.Instance()
       for S in [1,2]:
           for l in range(monitor.systemAndPlanes[S]):
              ut.bookHist(h,'SVSl_'+str(l),'QDC large vs small sum',200,0.,200.,200,0.,200.)
              ut.bookHist(h,'sVSl_'+str(l),'QDC large vs small average',200,0.,200.,200,0.,200.)
              for side in ['L','R']:
                   for i1 in range(monitor.systemAndPlanes[1]+monitor.systemAndPlanes[2]):
                      for i2 in range(i1+1,8):
                         tag=''
                         if S==2 and monitor.smallSiPMchannel(i1): tag = 's'+str(i1)
                         else:                              tag = 'l'+str(i1)
                         if S==2 and monitor.smallSiPMchannel(i2): tag += 's'+str(i2)
                         else:                              tag += 'l'+str(i2)
                         ut.bookHist(h,sdict[S]+'cor'+tag+'_'+side+str(l),'QDC channel i vs channel j',200,0.,200.,200,0.,200.)
                         for bar in range(monitor.systemAndBars[S]):
                              ut.bookHist(h,sdict[S]+'cor'+tag+'_'+side+str(l)+str(bar),'QDC channel i vs channel j',200,0.,200.,200,0.,200.)

   def ExecuteEvent(self,event):
      W = self.M.Weight
      M = self.M
      h = self.M.h
      sdict = self.M.sdict
      for aHit in event.Digi_MuFilterHits:
          if not aHit.isValid(): continue
          detID = aHit.GetDetectorID()
          s = detID//10000
          bar = (detID%1000)
          if s>2: continue
          l  = (detID%10000)//1000  # plane number
          sumL,sumS,SumL,SumS = 0,0,0,0
          allChannels = M.map2Dict(aHit,'GetAllSignals',mask=False)
          nS = 0
          nL = 0
          for c in allChannels:
              if s==2 and self.M.smallSiPMchannel(c) : 
                  sumS+= allChannels[c]
                  nS += 1
              else:
                  sumL+= allChannels[c]
                  nL+=1
          if nL>0: SumL=sumL/nL
          if nS>0: SumS=sumS/nS
          rc = h['sVSl_'+str(l)].Fill(SumS,SumL,W)
          rc = h['SVSl_'+str(l)].Fill(sumS/4.,sumL/12.,W)
#
          for side in ['L','R']:
            offset = 0
            if side=='R': offset = 8
            for i1 in range(offset,offset+7):
               if not i1 in allChannels: continue
               qdc1 = allChannels[i1]
               for i2 in range(i1+1,offset+8):
                 if not (i2) in allChannels: continue
                 if s==2 and self.M.smallSiPMchannel(i1): tag = 's'+str(i1-offset)
                 else: tag = 'l'+str(i1-offset)
                 if s==2 and self.M.smallSiPMchannel(i2): tag += 's'+str(i2-offset)
                 else: tag += 'l'+str(i2-offset)
                 qdc2 = allChannels[i2]
                 rc = h[sdict[s]+'cor'+tag+'_'+side+str(l)].Fill(qdc1,qdc2,W)
                 rc = h[sdict[s]+'cor'+tag+'_'+side+str(l)+str(bar)].Fill(qdc1,qdc2,W)
          allChannels.clear()

   def Plot(self):
       h = self.M.h
       sdict = self.M.sdict
       systemAndPlanes = self.M.systemAndPlanes
       ut.bookCanvas(h,'TSL','',1800,1400,3,2)
       ut.bookCanvas(h,'STSL','',1800,1400,3,2)
       S=2
       for l in range(systemAndPlanes[S]):
          tc = h['TSL'].cd(l+1)
          tc.SetLogz(1)
          aHist = h['sVSl_'+str(l)]
          aHist.SetTitle(';small SiPM QCD av:large SiPM QCD av')
          nmax = aHist.GetBinContent(aHist.GetMaximumBin())
          aHist.SetMaximum( 0.1*nmax )
          tc = h['sVSl_'+str(l)].Draw('colz')
       self.M.myPrint(h['TSL'],"largeSiPMvsSmallSiPM",subdir='mufilter/expert')
       for l in range(systemAndPlanes[S]):
          tc = h['STSL'].cd(l+1)
          tc.SetLogz(1)
          aHist = h['SVSl_'+str(l)]
          aHist.SetTitle(';small SiPM QCD sum/2:large SiPM QCD sum/6')
          nmax = aHist.GetBinContent(aHist.GetMaximumBin())
          aHist.SetMaximum( 0.1*nmax )
          tc = h['SVSl_'+str(l)].Draw('colz')
       self.M.myPrint(h['STSL'],"SumlargeSiPMvsSmallSiPM",subdir='mufilter/expert')
       for S in [1,2]:
         for l in range(systemAndPlanes[S]):
          for side in ['L','R']:
             if S == 1 and l == 2 and side == 'R': continue 
             ut.bookCanvas(h,sdict[S]+'cor'+side+str(l),'',1800,1400,7,4)
             k=1
             for i1 in range(7):
                for i2 in range(i1+1,8):
                  tag=''
                  if S==2 and self.M.smallSiPMchannel(i1): tag = 's'+str(i1)
                  else:                              tag = 'l'+str(i1)
                  if S==2 and self.M.smallSiPMchannel(i2): tag += 's'+str(i2)
                  else:                              tag += 'l'+str(i2)
                  tc = h[sdict[S]+'cor'+side+str(l)].cd(k)
                  for bar in range(self.M.systemAndBars[S]):
                      if bar == 0: h[sdict[S]+'cor'+tag+'_'+side+str(l)+str(bar)].Draw('colz')
                      else: h[sdict[S]+'cor'+tag+'_'+side+str(l)+str(bar)].Draw('colzsame')
                  k+=1
             self.M.myPrint(h[sdict[S]+'cor'+side+str(l)],'QDCcor'+side+str(l),subdir='mufilter/expert')

class Veto_Efficiency(ROOT.FairTask):
   " calculate Veto efficiency against Scifi tracks "
   def Init(self,options,monitor):
       self.debug = True
       self.deadTime = 100
       self.M = monitor
       sdict = self.M.sdict
       self.eventBefore={'T':-1,'N':-1,'hits':{2:0,1:0,0:0,'0L':0,'0R':0,'1L':0,'1R':0}}
       h = self.M.h
       run = ROOT.FairRunAna.Instance()
       self.trackTask = run.GetTask('simpleTracking')
       if not self.trackTask: self.trackTask = run.GetTask('houghTransform')
       ioman = ROOT.FairRootManager.Instance()
       self.OT = ioman.GetSink().GetOutTree()
       s = 1
       self.noiseCuts = [1,5,10,12]
       self.zEx = self.M.zPos['Scifi'][10]
       for noiseCut in self.noiseCuts:
        ut.bookHist(h,'timeDiffPrev_'+str(noiseCut),'time diff; [clock cycles] ',100,-0.5,999.5)
        ut.bookHist(h,'XtimeDiffPrev_'+str(noiseCut),'time diff no hits; [clock cycles] ',100,-0.5,999.5)
        ut.bookHist(h,'timeDiffNext_'+str(noiseCut),'time diff next; [clock cycles] ',100,-0.5,999.5)
        ut.bookHist(h,'XtimeDiffNext_'+str(noiseCut),'time diff next no hits; [clock cycles] ',100,-0.5,999.5)
        for c in ['','NoPrev','TiNoFi']:
         for b in ['','beam']:
          nc = 'T'+c+str(noiseCut)+b
          for l in range(monitor.systemAndPlanes[s]):
            ut.bookHist(h,nc+'PosVeto_'+str(l),'track pos at veto'+str(l)+' with hit '+';X [cm]; Y [cm]',110,-55.,0.,110,10.,65.)
            ut.bookHist(h,nc+'XPosVeto_'+str(l),'track pos at veto'+str(l)+' no hit'+str(l)+';X [cm]; Y [cm]',110,-55.,0.,110,10.,65.)
            ut.bookHist(h,nc+'XPosVetoXL_'+str(l),'track pos at veto'+str(l)+' no hit'+str(l)+';X [cm]; Y [cm]',1100,-55.,0.,1100,10.,65.)
            ut.bookHist(h,nc+'PosVeto_111'+str(l),'track pos at veto AND hit'+';X [cm]; Y [cm]',110,-55.,0.,110,10.,65.)
            ut.bookHist(h,nc+'XPosVeto_111'+str(l),'track pos at veto no hit'+';X [cm]; Y [cm]',110,-55.,0.,110,10.,65.)
            if l == 0:
              ut.bookHist(h,nc+'PosVeto_11','track pos at veto AND hit'+';X [cm]; Y [cm]',110,-55.,0.,110,10.,65.)
              ut.bookHist(h,nc+'XPosVeto_11','track pos at veto no hit'+';X [cm]; Y [cm]',110,-55.,0.,110,10.,65.)
          ut.bookHist(h,nc+'PosVeto_000','track pos at veto OR hit'+';X [cm]; Y [cm]',110,-55.,0.,110,10.,65.)
          for x in h:
            if isinstance(h[x], ROOT.TH2) and x.find("PosVeto")>0:
              h[x].SetStats(0)

       ut.bookHist(h,'hitVeto_0','nr hits L vs R;n sipm; n sipm',25,-0.5,24.5,25,-0.5,24.5)
       ut.bookHist(h,'hitVeto_1','nr hits L vs R;n sipm; n sipm',25,-0.5,24.5,25,-0.5,24.5)
       ut.bookHist(h,'hitVeto_2','nr hits T ;n sipm', 25,-0.5,24.5)
       ut.bookHist(h,'hitVeto_01','nr hits 0 vs 1;n sipm; n sipm',25,-0.5,24.5,25,-0.5,24.5)
       ut.bookHist(h,'hitVeto_02','nr hits 0 vs 2;n sipm; n sipm',25,-0.5,24.5,25,-0.5,24.5)
       ut.bookHist(h,'hitVeto_12','nr hits 1 vs 2;n sipm; n sipm',25,-0.5,24.5,25,-0.5,24.5)
       ut.bookHist(h,'scaler','all no prevEvent',25,-0.5,24.5)
       ut.bookHist(h,'deltaT','delta T DS 2 and Scifi 1',100,-20.0,20.)
       ut.bookHist(h,'X/Y','xy matching of scifi DS',100,-20.0,20.,100,-20.0,20.)

   def ExecuteEvent(self,event):
       scifiCorTest = False
       systemAndPlanes = self.M.systemAndPlanes
       sdict = self.M.sdict
       s = 1
       h = self.M.h
       W = self.M.Weight
       nSiPMs = self.M.MuFilter.GetConfParI("MuFilter/VetonSiPMs")
       hits = {2:0,1:0,0:0,'0L':0,'0R':0,'1L':0,'1R':0}
       vetoHitsFromPrev = 0
       if event.EventHeader.GetRunId() < 6204 and event.EventHeader.GetRunId() > 5480: vetoHitsFromPrev = 5
       # special treatment for first 10fb-1 in 2023, wrong time alignment, again!
       N1 = event.GetReadEntry()
       Tcurrent = event.EventHeader.GetEventTime()
       dT = abs(Tcurrent-self.eventBefore['T'])
       prevAdded = False
       for j in [0,-1]:
         if j<0 and N1>0: 
              if dT > vetoHitsFromPrev: continue
              rc = event.GetEvent(N1-1)  # add veto hits from prev event
              prevAdded = True
         for aHit in event.Digi_MuFilterHits:
           if not aHit.isValid(): continue
           Minfo = self.M.MuFilter_PlaneBars(aHit.GetDetectorID())
           s,l,bar = Minfo['station'],Minfo['plane'],Minfo['bar']
           if s>1: continue
           allChannels = self.M.map2Dict(aHit,'GetAllSignals')
           hits[l]+=len(allChannels)
           if l != 2:
             for c in allChannels:
                if  nSiPMs > c:  # left side
                   hits[str(l)+'L']+=1
                else:
                      hits[str(l)+'R']+=1
           allChannels.clear()
       if prevAdded and N1>1:
         rc = event.GetEvent(N1-2)
         Tprevprev = event.EventHeader.GetEventTime()
         dT = abs(Tcurrent-Tprevprev)
       if prevAdded: event.GetEvent(N1)
       prevEvent = False
       tightNoiseFilter = None
       otherAdvTrigger  = None
       otherFastTrigger = None
       if dT < self.deadTime and dT > vetoHitsFromPrev:
           prevEvent = True
           # check type of prev event, if it would pass tight noise filter, run 6568 ++ 
           if event.EventHeader.GetRunId() > 6567 and N1>0:
              tightNoiseFilter, otherFastTrigger, otherAdvTrigger,Nprev,dt = self.checkOtherTriggers(event)

       tmpT = self.eventBefore['T']
       tmpN = self.eventBefore['N']
       self.eventBefore['T'] = Tcurrent
       if (self.M.EventNumber - self.eventBefore['N'] > 1) and self.M.options.postScale < 2:
          print('what is going on?', self.M.EventNumber, self.eventBefore['N'])
       self.eventBefore['N'] = self.M.EventNumber

       rc = h['scaler'].Fill(11)
       if self.M.Reco_MuonTracks.GetEntries()<2: return # require Scifi and DS track
# check that track has scifi cluster in station 1, afterthought: require measurements in all planes
       planes = {}
       for scifiTrack in self.M.Reco_MuonTracks:
           if not scifiTrack.GetUniqueID()==1: continue
           fitStatus = scifiTrack.getFitStatus()
           if not fitStatus.isFitConverged(): continue
           if fitStatus.getNdf() < 5 or fitStatus.getNdf()>12 : continue
           if fitStatus.getChi2()/fitStatus.getNdf() > 80: continue
           for nM in range(scifiTrack.getNumPointsWithMeasurement()):
              M = scifiTrack.getPointWithMeasurement(nM)
              W = M.getRawMeasurement()
              detID = W.getDetId()
              planes[detID//100000] = 1
       rc = h['scaler'].Fill(10)
       scifiOneEff = 10 in planes or 11 in planes or not scifiCorTest
       if not scifiOneEff  and len(planes) < 8: return
       if scifiOneEff and len(planes) < 10: return
       rc = h['scaler'].Fill(0)
       if not prevEvent: rc = h['scaler'].Fill(1)

       for l in range(systemAndPlanes[1]-1):
          rc = h['hitVeto_'+str(l)].Fill(hits[str(l)+'L'],hits[str(l)+'R'])
       rc = h['hitVeto_01'].Fill(hits[0],hits[1])
       if systemAndPlanes[1] > 2 :
         rc = h['hitVeto_02'].Fill(hits[0],hits[2])
         rc = h['hitVeto_12'].Fill(hits[1],hits[2])

       for muTrack in self.M.Reco_MuonTracks:
          if not muTrack.GetUniqueID()==3: continue
          fstate =  muTrack.getFittedState()
          posT,momT  = fstate.getPos(),fstate.getMom()
          lam      = (self.zEx-posT.z())/momT.z()
          yExTag      = posT.y()+lam*momT.y()
          xExTag      = posT.x()+lam*momT.x()
          sstate = scifiTrack.getFittedState()
          pos = sstate.getPos()
          delX = xExTag - pos.x()
          delY = yExTag - pos.y()
          rc = h['X/Y'].Fill(delX,delY)
          if abs(delX)>10 or abs(delY)>10: return
          
       for aTrack in self.M.Reco_MuonTracks:
           if not aTrack.GetUniqueID()==1: continue
           fitStatus = aTrack.getFitStatus()
           if not fitStatus.isFitConverged(): continue
           fstate =  aTrack.getFittedState()
           pos,mom = [fstate.getPos(),fstate.getMom()]
           beam = False
           if abs(mom.x()/mom.z())<0.1 and abs(mom.y()/mom.z())<0.1: beam = True
# extrapolate to veto
# check timing, remove backward tracks
# 1: get early DS time in horizontal plane 2:
           dsHitTimes = []
           for aHit in event.Digi_MuFilterHits:
              if not aHit.isValid(): continue
              detID = aHit.GetDetectorID()
              Minfo = self.M.MuFilter_PlaneBars(detID)
              s,l,bar = Minfo['station'],Minfo['plane'],Minfo['bar']
              if s==3 and l==2:
                   self.M.MuFilter.GetPosition(detID,A,B)
                   timeLeft = aHit.GetTime(0)
                   timeRight = aHit.GetTime(1)
                   if timeLeft>0 and timeRight>0:
                      dL = abs(A[0]-B[0])
                      avTime = self.M.MuFilter.GetCorrectedTime(detID, 0, timeLeft*self.M.TDC2ns,0) + \
                               self.M.MuFilter.GetCorrectedTime(detID, 1, timeRight*self.M.TDC2ns,0)
                      dsHitTimes.append( (avTime-abs(A[0]-B[0])/15)/2) # L/2 / 15cm/ns
           dsHitTimes.sort()
           scifiHitTimes = {1:[],2:[],3:[],4:[],5:[]}
           deltaT = -100
           if len(dsHitTimes)>0:
            for scifiHit in event.Digi_ScifiHits:
              detID = scifiHit.GetDetectorID()
              s = int(scifiHit.GetDetectorID()/1000000)
              if s>1.5: continue
              scifiHitTimes[s].append(self.M.Scifi.GetCorrectedTime(detID,scifiHit.GetTime()*self.M.TDC2ns,0))
            for s in scifiHitTimes:
                if len(scifiHitTimes[s])<1: continue
                scifiHitTimes[s].sort()
                deltaT = dsHitTimes[0] - scifiHitTimes[s][0] - (self.M.zPos['MuFilter'][34]-self.M.zPos['Scifi'][s*10])/u.speedOfLight
           rc = h['deltaT'].Fill(deltaT)
           if deltaT < -10: continue
           #look for previous event time
           T1 = event.EventHeader.GetEventTime()
           N1 = event.EventHeader.GetEventNumber()
           if prevAdded: rc = event.GetEvent(N1-2)
           else:         rc = event.GetEvent(N1-1)
           T0 = event.EventHeader.GetEventTime()
           rc = event.GetEvent(N1+1)
           T2 = event.EventHeader.GetEventTime()
           rc = event.GetEvent(N1)
           if (T1-T0) < 100 and self.M.options.postScale < 2:
               if not prevEvent: print('what is going on?',N1,T1,T0,N1-1,tmpN,tmpT)
               prevEvent = True
           s = 1
           xEx = {}
           yEx = {}
           for l in range(systemAndPlanes[1]):
              zEx = self.M.zPos['MuFilter'][s*10+l]
              lam = (zEx-pos.z())/mom.z()
              xEx[l] = pos.x()+lam*mom.x()
              yEx[l] = pos.y()+lam*mom.y()
           for l in range(systemAndPlanes[1]):
              for noiseCut in self.noiseCuts:
                 c=''
                 if not prevEvent: c='NoPrev'
                 nc = 'T'+c+str(noiseCut)
                 ncL = 'T'+'TiNoFi'+str(noiseCut)
                 if hits[l] > noiseCut or (l==2 and hits[l] > int(noiseCut/2+0.5)):
                      rc = h[nc+'PosVeto_'+str(l)].Fill(xEx[l],yEx[l])
                      if tightNoiseFilter: rc = h[ncL+'PosVeto_'+str(l)].Fill(xEx[l],yEx[l])
                      if beam: rc = h[nc+'beamPosVeto_'+str(l)].Fill(xEx[l],yEx[l])
                 else:                        
                      rc = h[nc+'XPosVeto_'+str(l)].Fill(xEx[l],yEx[l])
                      rc = h[nc+'XPosVetoXL_'+str(l)].Fill(xEx[l],yEx[l])
                      if tightNoiseFilter: h[ncL+'XPosVeto_'+str(l)].Fill(xEx[l],yEx[l])
                      if beam: rc = h[nc+'beamXPosVeto_'+str(l)].Fill(xEx[l],yEx[l])
                 if l==0:
                    if -45<xEx[l] and xEx[l]<-10 and 27<yEx[l] and yEx[l]<54:
                          rc = h['timeDiffPrev_'+str(noiseCut)].Fill(T1-T0)
                          rc = h['timeDiffNext_'+str(noiseCut)].Fill(T2-T1)
                    if ( systemAndPlanes[1]==2 and hits[0] > noiseCut and hits[1] > noiseCut) or \
                       ( systemAndPlanes[1]==3 and hits[0] > noiseCut and hits[1] > noiseCut \
                                               and hits[2] > int(noiseCut/2+0.5) ):
                      for l1 in range(systemAndPlanes[1]):
                        rc = h[nc+'PosVeto_111'+str(l1)].Fill(xEx[l1],yEx[l1])
                        if tightNoiseFilter: 
                          rc = h[ncL+'PosVeto_111'+str(l1)].Fill(xEx[l1],yEx[l1])
                      if beam: rc = h[nc+'beamPosVeto_1110'].Fill(xEx[l],yEx[l])
                    if ( systemAndPlanes[1]==2 and (hits[0] > noiseCut or hits[1] > noiseCut)) or \
                       ( systemAndPlanes[1]==3 and (hits[0] > noiseCut or hits[1] > noiseCut \
                                               or hits[2] > int(noiseCut/2+0.5)) ):
                      rc = h[nc+'PosVeto_000'].Fill(xEx[l],yEx[l])
                      if tightNoiseFilter: h[ncL+'PosVeto_000'].Fill(xEx[l],yEx[l])
                      if beam: rc = h[nc+'beamPosVeto_000'].Fill(xEx[l],yEx[l])
                    else:
                        if -45<xEx[l] and xEx[l]<-10 and 27<yEx[l] and yEx[l]<54:
                          rc = h['XtimeDiffPrev_'+str(noiseCut)].Fill(T1-T0)
                          rc = h['XtimeDiffNext_'+str(noiseCut)].Fill(T2-T1)
                          if not prevEvent or (prevEvent and not tightNoiseFilter):
                            if self.debug: print('no hits',noiseCut,prevEvent,beam,N1,tightNoiseFilter,otherFastTrigger,otherAdvTrigger)
                        for l1 in range(systemAndPlanes[1]):
                          rc = h[nc+'XPosVeto_111'+str(l1)].Fill(xEx[l1],yEx[l1])
                          if tightNoiseFilter: 
                           rc = h[ncL+'XPosVeto_111'+str(l1)].Fill(xEx[l1],yEx[l1])
                        if beam: rc = h[nc+'beamXPosVeto_1110'].Fill(xEx[l],yEx[l])
                    # also for the 2 planea
                    if hits[0] > noiseCut and hits[1] > noiseCut:
                      rc = h[nc+'PosVeto_11'].Fill(xEx[l],yEx[l])
                    if not (hits[0] > noiseCut or hits[1] > noiseCut):
                      rc = h[nc+'XPosVeto_11'].Fill(xEx[l],yEx[l])

   def checkOtherTriggers(self,event,debug=False):
      T0 = event.EventHeader.GetEventTime()
      N = event.EventHeader.GetEventNumber()
      otherFastTrigger = False
      otherAdvTrigger = False
      tightNoiseFilter = False
      Nprev = 1
      if N<Nprev: return otherFastTrigger, otherAdvTrigger, tightNoiseFilter, -1, 0
      rc = event.GetEvent(N-Nprev)
      dt = T0 - event.EventHeader.GetEventTime()
      while dt < self.deadTime and N>Nprev:
         otherFastTrigger = False
         for x in event.EventHeader.GetFastNoiseFilters():
             if debug: print('fast:', x.first, x.second )
             if x.second and not x.first == 'Veto_Total': otherFastTrigger = True
         otherAdvTrigger = False
         for x in event.EventHeader.GetAdvNoiseFilters():
             if debug: print('adv:', x.first, x.second )
             if x.second and not x.first == 'VETO_Planes': otherAdvTrigger = True
         if debug: print('pre event ',Nprev,dt,otherFastTrigger,otherAdvTrigger)
         if otherFastTrigger and otherAdvTrigger:
             rc = event.GetEvent(N)
             return otherFastTrigger, otherAdvTrigger, tightNoiseFilter, Nprev, dt
         Nprev+=1
         rc = event.GetEvent(N-Nprev)
         dt = T0 - event.EventHeader.GetEventTime()
      Nprev = 1
      rc = event.GetEvent(N-Nprev)
      dt = T0 - event.EventHeader.GetEventTime()
      while dt < self.deadTime and Nprev>N:
         hits = {2:0,1:0,0:0}
         for aHit in event.Digi_MuFilterHits:
            Minfo = self.M.MuFilter_PlaneBars(aHit.GetDetectorID())
            s,l,bar = Minfo['station'],Minfo['plane'],Minfo['bar']
            if s>1: continue
            allChannels = aHit.GetAllSignals(False,False)
            hits[l]+=len(allChannels)
         noiseFilter0 = sum(hits)>4.5
         noiseFilter1 = all(p > 0 for p in hits)
         if debug: print('veto hits:',hits)
         if noiseFilter0 and noiseFilter1: 
            tightNoiseFilter = True
            rc = event.GetEvent(N)
            return otherFastTrigger, otherAdvTrigger, tightNoiseFilter, Nprev-1, dt
         Nprev+=1
         rc = event.GetEvent(N-Nprev)
         dt = T0 - event.EventHeader.GetEventTime()
      if Nprev>1: 
            rc = event.GetEvent(N-Nprev+1)
            dt = T0 - event.EventHeader.GetEventTime()
      rc = event.GetEvent(N)
      return otherFastTrigger, otherAdvTrigger, tightNoiseFilter, Nprev-1, dt

   def Plot(self,beamOnly=False):
     h = self.M.h
     nVetoPlanes =self.M.systemAndPlanes[1]
     if nVetoPlanes > 2 : hist_idx = ['0','1','2','000','1110']
     else: hist_idx = ['0','1','000','1110']
     if beamOnly: b='beam'
     else: b=''
     for c in ['','NoPrev']:
      allTracks = h['T'+c+'1PosVeto_0'].Clone('tmp')
      allTracks.Add(h['T'+c+'1XPosVeto_0'])
      for noiseCut in self.noiseCuts:
       nc = 'T'+c+str(noiseCut)+b
       h[nc+'XPosVeto_000']=allTracks.Clone(nc+'XPosVeto_000')
       h[nc+'XPosVeto_000'].Add(h[nc+'PosVeto_000'],-1)
       for l in hist_idx:
           h[nc+'Veto_ineff'+l] = h[nc+'PosVeto_'+l].Clone(nc+'Veto_ineff'+l)
           if l != '2': h[nc+'Veto_ineff'+l].SetTitle('Veto inefficiency '+l+' noise cut='+str(noiseCut))
           else: h[nc+'Veto_ineff'+l].SetTitle('Veto inefficiency '+l+' noise cut='+str(int(noiseCut/2+0.5)))
           h[nc+'Veto_ineff'+l].SetMinimum(0)
           h[nc+'Veto_ineff'+l].SetMaximum(1)
       for ix in range(allTracks.GetNbinsX()):
          for iy in range(allTracks.GetNbinsY()):
              for l in hist_idx:
                 bc = allTracks.GetBinContent(ix,iy)
                 if bc < 100:
                    h[nc+'Veto_ineff'+l].SetBinContent(ix,iy,-1)
                    h[nc+'Veto_ineff'+l].SetBinError(ix,iy,0)
                 else:
                    h[nc+'Veto_ineff'+l].SetBinContent(ix,iy,max(h[nc+'XPosVeto_'+l].GetBinContent(ix+1,iy+1)/bc, 2.7/bc))
                    h[nc+'Veto_ineff'+l].SetBinError(ix,iy,h[nc+'XPosVeto_'+l].GetBinError(ix+1,iy+1)/bc)
       ut.bookCanvas(h,nc+'VetoEff','',1400,1800, nVetoPlanes, 2*nVetoPlanes)
       for p in range(2*nVetoPlanes**2):
         tc = h[nc+'VetoEff'].cd(p+1)
         if p < nVetoPlanes:
           h[nc+'PosVeto_'+str(p%nVetoPlanes)].Draw('colz')
         if p in range(nVetoPlanes, 2*nVetoPlanes):
           h[nc+'PosVeto_111'+str(p%nVetoPlanes)].Draw('colz')
         if p in range(2*nVetoPlanes, 3*nVetoPlanes):
           h[nc+'XPosVeto_'+str(p%nVetoPlanes)].Draw('colz')
         if p in range(3*nVetoPlanes, 4*nVetoPlanes):
           h[nc+'XPosVeto_111'+str(p%nVetoPlanes)].Draw('colz')
       tc = h[nc+'VetoEff'].cd(4*nVetoPlanes+1)
       h[nc+'PosVeto_000'].Draw('colz')

       ut.bookCanvas(h,nc+'VetoInEff','',1800,1400,nVetoPlanes,2)
       for p in range(nVetoPlanes): 
         tc = h[nc+'VetoInEff'].cd(p+1)
         tc.SetLogz(1)
         h[nc+'Veto_ineff'+str(p)].Draw('colz')
       tc = h[nc+'VetoInEff'].cd(nVetoPlanes+1)
       tc.SetLogz(1)
       h[nc+'Veto_ineff1110'].Draw('colz')
       tc = h[nc+'VetoInEff'].cd(nVetoPlanes+2)
       tc.SetLogz(1)
       h[nc+'Veto_ineff000'].Draw('colz')
# make some printout
       Ntot = h[nc+'PosVeto_0'].Clone('Ntot')
       Ntot.Add(h[nc+'XPosVeto_0'])
       ineff0 =  h[nc+'XPosVeto_0'].GetEntries()/(Ntot.GetEntries()+1E-20)
       ineff1 = h[nc+'XPosVeto_1'].GetEntries()/(Ntot.GetEntries()+1E-20)
       ineff2, ineffAND_old, ineffOR_old = 0, 0, 0
       if nVetoPlanes > 2:
         ineff2 = h[nc+'XPosVeto_2'].GetEntries()/(Ntot.GetEntries()+1E-20)
         ineffOR_old =  h[nc+'XPosVeto_11'].GetEntries()/(Ntot.GetEntries()+1E-20)
         ineffAND_old = 1.-h[nc+'PosVeto_11'].GetEntries()/(Ntot.GetEntries()+1E-20)
       ineffOR =  h[nc+'XPosVeto_1110'].GetEntries()/(Ntot.GetEntries()+1E-20)
       ineffAND = 1.-h[nc+'PosVeto_1110'].GetEntries()/(Ntot.GetEntries()+1E-20)
       region = [21,91,34,89]
       xax = h[nc+'PosVeto_0'].GetXaxis()
       yax = h[nc+'PosVeto_0'].GetYaxis()
       Ntot_r = Ntot.Integral(region[0],region[1],region[2],region[3])+1E-20
       ineff0_r = h[nc+'XPosVeto_0'].Integral(region[0],region[1],region[2],region[3])/Ntot_r
       ineff1_r = h[nc+'XPosVeto_1'].Integral(region[0],region[1],region[2],region[3])/Ntot_r
       ineff2_r, ineffAND_old_r, ineffOR_old_r = 0, 0, 0
       if nVetoPlanes > 2:
         ineff2_r = h[nc+'XPosVeto_2'].Integral(region[0],region[1],region[2],region[3])/Ntot_r
         ineffOR_r_old =  h[nc+'XPosVeto_11'].Integral(region[0],region[1],region[2],region[3])/Ntot_r
         ineffAND_r_old = 1.-h[nc+'PosVeto_11'].Integral(region[0],region[1],region[2],region[3])/Ntot_r
       ineffOR_r =  h[nc+'XPosVeto_1110'].Integral(region[0],region[1],region[2],region[3])/Ntot_r
       ineffAND_r = 1.-h[nc+'PosVeto_1110'].Integral(region[0],region[1],region[2],region[3])/Ntot_r
       print('noise cut = ',noiseCut, 'previous event:',c)
       print('global inefficiency veto0: %5.2F%% veto1: %5.2F%% veto2: %5.2F%%'
                                  %(ineff0*100,ineff1*100,ineff2*100),
             'vetoAND: %5.2F%% vetoOR: %5.2F%% veto0AND1: %5.2F%% veto0OR1: %5.2F%%'
                                  %(ineffAND*100,ineffOR*100,ineffAND_old*100,ineffOR_old*100))
       print('region %5.2F < X < %5.2F and %5.2F < Y < %5.2F '%(xax.GetBinCenter(region[0]),
          xax.GetBinCenter(region[1]),yax.GetBinCenter(region[0]),yax.GetBinCenter(region[1])))
       print('veto0: %5.2F%% veto1: %5.2F%% veto2: %5.2F%%'
              %(ineff0_r*100,ineff1_r*100,ineff2_r*100),
             'vetoAND: %5.2F%% vetoOR: %5.2F%% veto0AND1: %5.2F%% veto0OR1: %5.2F%%'
             %(ineffAND_r*100,ineffOR_r*100,ineffAND_old*100,ineffOR_old*100))
#
     for p in range(1,nVetoPlanes):
       if p == 1:
         h['hitVeto_X'] = h['hitVeto_0'+str(p)].ProjectionX('hitVeto_X')
         h['hitVeto_X'].SetStats(0)
         h['hitVeto_X'].SetLineColor(ROOT.kOrange)
         h['hitVeto_X'].SetLineWidth(3)
       h['hitVeto_Y'+str(p)] = h['hitVeto_0'+str(p)].ProjectionY('hitVeto_Y'+str(p))
       h['hitVeto_Y'+str(p)].SetLineColor(ROOT.kBlue-2*p)
       h['hitVeto_Y'+str(p)].SetStats(0)
     ut.bookCanvas(h,'ThitVeto','',900,600,1,1)
     tc = h['ThitVeto'].cd()
     tc.SetLogy(1)
     h['hitVeto_X'].Draw('hist')
     for p in range(1,nVetoPlanes): h['hitVeto_Y'+str(p)].Draw('histsame')

     #save the Veto inefficiency plots to file
     if self.M.options.postScale<2:
        self.M.presenterFile.mkdir('mufilter/VetoIneff')
     for item in h:
       if isinstance(h[item], ROOT.TCanvas) and \
          (item.find('Eff')>0 or item.find('ThitVeto')>0):
              self.M.myPrint(h[item],item,subdir='mufilter/expert/VetoIneff')
