#!/usr/bin/env python
import ROOT,os,sys
import rootUtils as ut
import shipunit as u
import ctypes
from array import array

A,B  = ROOT.TVector3(),ROOT.TVector3()
parallelToZ = ROOT.TVector3(0., 0., 1.)
detector = "scifi-"
class Scifi_hitMaps(ROOT.FairTask):
   " produce hitmaps for Scifi"
   def Init(self,options,monitor):
       self.M = monitor
       h = self.M.h
       ioman = ROOT.FairRootManager.Instance()
       self.OT = ioman.GetSink().GetOutTree()
       
       for s in range(10):
          ut.bookHist(h,detector+'posX_'+str(s),'x; x [cm]',2000,-100.,100.)
          ut.bookHist(h,detector+'posY_'+str(s),'y; y[cm]',2000,-100.,100.)
          if s%2==1: ut.bookHist(h,detector+'mult_'+str(s),'mult vertical station '+str(s//2+1)+'; #hits',100,-0.5,99.5)
          else: ut.bookHist(h,detector+'mult_'+str(s),'mult horizontal station '+str(s//2+1)+'; #hits',100,-0.5,99.5)
       for mat in range(30):
          ut.bookHist(h,detector+'mat_'+str(mat),'hit map / mat; #channel',512,-0.5,511.5)
          ut.bookHist(h,detector+'sig_'+str(mat),'signal / mat; QDC [a.u.]',200,-50.0,150.)
          ut.bookHist(h,detector+'tdc_'+str(mat),'tdc / mat; timestamp [LHC clock cycles]',200,-1.,4.)
   def ExecuteEvent(self,event):
       h = self.M.h
       mult = [0]*10
       for aHit in event.Digi_ScifiHits:
          if not aHit.isValid(): continue
          X =  self.M.Scifi_xPos(aHit.GetDetectorID())
          rc = h[detector+'mat_'+str(X[0]*3+X[1])].Fill(X[2])
          rc = h[detector+'sig_'+str(X[0]*3+X[1])].Fill(aHit.GetSignal(0))
          rc = h[detector+'tdc_'+str(X[0]*3+X[1])].Fill(aHit.GetTime(0))
          self.M.Scifi.GetSiPMPosition(aHit.GetDetectorID(),A,B)
          if aHit.isVertical(): rc = h[detector+'posX_'+str(X[0])].Fill(A[0])
          else:                     rc = h[detector+'posY_'+str(X[0])].Fill(A[1])
          mult[X[0]]+=1
       for s in range(10):
          rc = h[detector+'mult_'+str(s)].Fill(mult[s])
   def Plot(self):
       h = self.M.h
       ut.bookCanvas(h,detector+'hitmaps',' ',1024,768,6,5)
       ut.bookCanvas(h,detector+'signal',' ',1024,768,6,5)
       ut.bookCanvas(h,detector+'tdc',' ',1024,768,6,5)
       for mat in range(30):
           tc = self.M.h[detector+'hitmaps'].cd(mat+1)
           A = self.M.h[detector+'mat_'+str(mat)].GetSumOfWeights()/512.
           if self.M.h[detector+'mat_'+str(mat)].GetMaximum()>10*A: self.M.h[detector+'mat_'+str(mat)].SetMaximum(10*A)
           self.M.h[detector+'mat_'+str(mat)].Draw()
           tc = self.M.h[detector+'signal'].cd(mat+1)
           self.M.h[detector+'sig_'+str(mat)].Draw()
           tc = self.M.h[detector+'tdc'].cd(mat+1)
           self.M.h[detector+'tdc_'+str(mat)].Draw()

       ut.bookCanvas(h,detector+'positions',' ',2048,768,5,2)
       ut.bookCanvas(h,detector+'mult',' ',2048,768,5,2)
       for s in range(5):
           tc = self.M.h[detector+'positions'].cd(s+1)
           self.M.h[detector+'posY_'+str(2*s)].Draw()
           tc = self.M.h[detector+'positions'].cd(s+6)
           self.M.h[detector+'posX_'+str(2*s+1)].Draw()

           tc = self.M.h[detector+'mult'].cd(s+1)
           tc.SetLogy(1)
           self.M.h[detector+'mult_'+str(2*s)].Draw()
           tc = self.M.h[detector+'mult'].cd(s+6)
           tc.SetLogy(1)
           self.M.h[detector+'mult_'+str(2*s+1)].Draw()

       for canvas in [detector+'hitmaps',detector+'signal',detector+'mult']:
           self.M.h[canvas].Update()
           self.M.myPrint(self.M.h[canvas],"Scifi-"+canvas,subdir='scifi')

class Scifi_residuals(ROOT.FairTask):
   " produce residuals for Scifi"
   def Init(self,options,monitor):
       NbinsRes = options.ScifiNbinsRes
       xmin        = options.Scifixmin
       alignPar   = options.ScifialignPar

       self.M = monitor
       h = self.M.h
       self.projs = {1:'V',0:'H'}
       self.parallelToZ = ROOT.TVector3(0., 0., 1.)
       run = ROOT.FairRunAna.Instance()
       ioman = ROOT.FairRootManager.Instance()
       self.OT = ioman.GetSink().GetOutTree()
       self.nav = ROOT.gGeoManager.GetCurrentNavigator()
       self.trackTask = self.M.FairTasks['simpleTracking']
       if not self.trackTask: self.trackTask = run.GetTask('houghTransform')

       for s in range(1,6):
          for o in range(2):
             for p in self.projs:
               proj = self.projs[p]
               xmax = -xmin
               ut.bookHist(h,'res'+proj+'_Scifi'+str(s*10+o),'residual '+proj+str(s*10+o)+'; [#mum]',NbinsRes,xmin,xmax)
               ut.bookHist(h,'resX'+proj+'_Scifi'+str(s*10+o),'residual '+proj+str(s*10+o)+'; [#mum]',NbinsRes,xmin,xmax,100,-50.,0.)
               ut.bookHist(h,'resY'+proj+'_Scifi'+str(s*10+o),'residual '+proj+str(s*10+o)+'; [#mum]',NbinsRes,xmin,xmax,100,10.,60.)
               ut.bookHist(h,'resC'+proj+'_Scifi'+str(s*10+o),'residual '+proj+str(s*10+o)+'; [#mum]',NbinsRes,xmin,xmax,128*4*3,-0.5,128*4*3-0.5)
               ut.bookHist(h,'track_Scifi'+str(s*10+o),'track x/y '+str(s*10+o)+'; x [cm]; y [cm]',80,-70.,10.,80,0.,80.)
       ut.bookHist(h,detector+'trackChi2/ndof','track chi2/ndof vs ndof; #chi^{2}/Ndof; Ndof',100,0,100,20,0,20)
       ut.bookHist(h,detector+'trackSlopes','track slope; x/z [mrad]; y/z [mrad]',1000,-100,100,1000,-100,100)
       ut.bookHist(h,detector+'trackSlopesXL','track slope; x/z [rad]; y/z [rad]',120,-1.1,1.1,120,-1.1,1.1)
       ut.bookHist(h,detector+'trackPos','track pos; x [cm]; y [cm]',100,-90,10.,80,0.,80.)
       ut.bookHist(h,detector+'trackPosBeam','beam track pos slopes<0.1rad; x [cm]; y [cm]',100,-90,10.,80,0.,80.)

       if alignPar:
            for x in alignPar:
               self.M.Scifi.SetConfPar(x,alignPar[x])

   def ExecuteEvent(self,event):
       h = self.M.h
       nav = self.nav
       if not hasattr(event,"Cluster_Scifi"):
               self.trackTask.scifiCluster()
               clusters = self.OT.Cluster_Scifi
       else:
               clusters = event.Cluster_Scifi

# overall tracking
       self.trackTask.ExecuteTask("Scifi")
       for aTrack in self.OT.Reco_MuonTracks:
          if not aTrack.GetUniqueID()==1: continue
          fitStatus = aTrack.getFitStatus()
          if not fitStatus.isFitConverged(): continue
          state = aTrack.getFittedState()
          pos    = state.getPos()
          mom = state.getMom()
          slopeX = mom.X()/mom.Z()
          slopeY = mom.Y()/mom.Z()
          rc = h[detector+'trackChi2/ndof'].Fill(fitStatus.getChi2()/(fitStatus.getNdf()+1E-10),fitStatus.getNdf() )
          rc = h[detector+'trackSlopes'].Fill(slopeX*1000,slopeY*1000)
          rc = h[detector+'trackSlopesXL'].Fill(slopeX,slopeY)
          rc = h[detector+'trackPos'].Fill(pos.X(),pos.Y())
          if abs(slopeX)<0.1 and abs(slopeY)<0.1:  rc = h[detector+'trackPosBeam'].Fill(pos.X(),pos.Y())

       sortedClusters={}
       for aCl in clusters:
           so = aCl.GetFirst()//100000
           if not so in sortedClusters: sortedClusters[so]=[]
           sortedClusters[so].append(aCl)
# select events with clusters in each plane
       if len(sortedClusters)<10: return
       for s in range(1,6):
# build trackCandidate
            hitlist = {}
            k=0
            for so in sortedClusters:
                    if so//10 == s: continue
                    for x in sortedClusters[so]:
                       hitlist[k] = x
                       k+=1
            theTrack = self.trackTask.fitTrack(hitlist)
            if not hasattr(theTrack,"getFittedState"): continue
# check residuals
            fitStatus = theTrack.getFitStatus()
            if not fitStatus.isFitConverged(): 
                 theTrack.Delete()
                 continue
# test plane
            for o in range(2):
                testPlane = s*10+o
                z = self.M.zPos['Scifi'][testPlane]
                rep     = ROOT.genfit.RKTrackRep(13)
                state  = ROOT.genfit.StateOnPlane(rep)
# find closest track state
                mClose = 0
                mZmin = 999999.
                for m in range(0,theTrack.getNumPointsWithMeasurement()):
                   st   = theTrack.getFittedState(m)
                   Pos = st.getPos()
                   if abs(z-Pos.z())<mZmin:
                      mZmin = abs(z-Pos.z())
                      mClose = m
                fstate =  theTrack.getFittedState(mClose)
                pos,mom = fstate.getPos(),fstate.getMom()
                rep.setPosMom(state,pos,mom)
                NewPosition = ROOT.TVector3(0., 0., z)   # assumes that plane in global coordinates is perpendicular to z-axis, which is not true for TI18 geometry.
                rep.extrapolateToPlane(state, NewPosition, parallelToZ )
                pos = state.getPos()
                xEx,yEx = pos.x(),pos.y()
                rc = h['track_Scifi'+str(testPlane)].Fill(xEx,yEx)
                for aCl in sortedClusters[testPlane]:
                   aCl.GetPosition(A,B)
                   detID = aCl.GetFirst()
                   channel = detID%1000 + ((detID%10000)//1000)*128 + (detID%100000//10000)*512
# calculate DOCA
                   pq = A-pos
                   uCrossv= (B-A).Cross(mom)
                   doca = pq.Dot(uCrossv)/uCrossv.Mag()
                   rc = h['resC'+self.projs[o]+'_Scifi'+str(testPlane)].Fill(doca/u.um,channel)
                   rc = h['res'+self.projs[o]+'_Scifi'+str(testPlane)].Fill(doca/u.um)
                   rc = h['resX'+self.projs[o]+'_Scifi'+str(testPlane)].Fill(doca/u.um,xEx)
                   rc = h['resY'+self.projs[o]+'_Scifi'+str(testPlane)].Fill(doca/u.um,yEx)

            theTrack.Delete()

# analysis and plots 
   def Plot(self):
       h = self.M.h
       P = {'':'','X':'colz','Y':'colz','C':'colz'}
       Par = {'mean':1,'sigma':2}
       h['globalPos']   = {'meanH':ROOT.TGraphErrors(),'sigmaH':ROOT.TGraphErrors(),'meanV':ROOT.TGraphErrors(),'sigmaV':ROOT.TGraphErrors()}
       h['globalPosM'] = {'meanH':ROOT.TGraphErrors(),'sigmaH':ROOT.TGraphErrors(),'meanV':ROOT.TGraphErrors(),'sigmaV':ROOT.TGraphErrors()}
       for x in h['globalPosM']:
            h['globalPos'][x].SetMarkerStyle(21)
            h['globalPos'][x].SetMarkerColor(ROOT.kBlue)
            h['globalPosM'][x].SetMarkerStyle(21)
            h['globalPosM'][x].SetMarkerColor(ROOT.kBlue)
       globalPos = h['globalPos']
       for proj in P:
           ut.bookCanvas(h,'scifiRes'+proj,'',1600,1900,2,5)
           k=1
           j = {0:0,1:0}
           for s in range(1,6):
               for o in range(2):
                  so = s*10+o
                  tc = h['scifiRes'+proj].cd(k)
                  k+=1
                  hname = 'res'+proj+self.projs[o]+'_Scifi'+str(so)
                  h[hname].Draw(P[proj])
                  if proj == '':
                     rc = h[hname].Fit('gaus','SQ')
                     fitResult = rc.Get()
                     if not fitResult: continue
                     for p in Par:
                          globalPos[p+self.projs[o]].SetPoint(s-1,s,fitResult.Parameter(Par[p]))
                          globalPos[p+self.projs[o]].SetPointError(s-1,0.5,fitResult.ParError(1))
                  if proj == 'C':
                       for m in range(3):
                             h[hname+str(m)] = h[hname].ProjectionX(hname+str(m),m*512,m*512+512)
                             rc = h[hname+str(m)].Fit('gaus','SQ0')
                             fitResult = rc.Get()
                             if not fitResult: continue
                             for p in Par:
                                 h['globalPosM'][p+self.projs[o]].SetPoint(j[o], s*10+m,   fitResult.Parameter(Par[p]))
                                 h['globalPosM'][p+self.projs[o]].SetPointError(j[o],0.5,fitResult.ParError(1))
                             j[o]+=1
       
       S  = ctypes.c_double()
       M = ctypes.c_double()
       h['alignPar'] = {}
       alignPar = h['alignPar']
       for p in globalPos:
           ut.bookCanvas(h,p,p,750,750,1,1)
           tc = h[p].cd()
           globalPos[p].SetTitle(p+';station; offset [#mum]')
           globalPos[p].Draw("ALP")
           if p.find('mean')==0:
               for n in range(globalPos[p].GetN()):
                  rc = globalPos[p].GetPoint(n,S,M)
                  print("station %i: offset %s =  %5.2F um"%(S.value,p[4:5],M.value))
                  s = int(S.value*10)
                  if p[4:5] == "V": s+=1
                  alignPar["Scifi/LocD"+str(s)] = M.value

       ut.bookCanvas(h,'mean&sigma',"mean and sigma",1200,1200,2,2)
       k=1
       for p in h['globalPosM']:
           ut.bookCanvas(h,p+'M',p,750,750,1,1)
           tc = h[p+'M'].cd()
           h['globalPosM'][p].SetTitle(p+';mat ; offset [#mum]')
           h['globalPosM'][p].Draw("ALP")
           tc = h['mean&sigma'].cd(k)
           h['globalPosM'][p].Draw("ALP")
           k+=1
           if p.find('mean')==0:
              for n in range(h['globalPosM'][p].GetN()):
                 rc = h['globalPosM'][p].GetPoint(n,S,M)
                 print("station %i: offset %s =  %5.2F um"%(S.value,p[4:5],M.value))
                 s = int(S.value*10)
                 if p[4:5] == "V": s+=1
                 alignPar["Scifi/LocM"+str(s)] = M.value
       T = ['mean&sigma']
       for proj in P: T.append('scifiRes'+proj)
       for canvas in T:
           self.M.myPrint(self.M.h[canvas],"Scifi-"+canvas,subdir='scifi')
       ut.bookCanvas(h,detector+'trackDir',"track directions",1600,1800,3,2)
       h[detector+'trackDir'].cd(1)
       rc = h[detector+'trackSlopes'].Draw('colz')
       h[detector+'trackDir'].cd(2)
       rc = h[detector+'trackSlopes'].ProjectionX("slopeX").Draw()
       h[detector+'trackDir'].cd(3)
       rc = h[detector+'trackSlopes'].ProjectionY("slopeY").Draw()
       h[detector+'trackDir'].cd(4)
       rc = h[detector+'trackSlopesXL'].Draw('colz')
       h[detector+'trackDir'].cd(5)
       rc = h[detector+'trackSlopesXL'].ProjectionX("slopeXL").Draw()
       h[detector+'trackDir'].cd(6)
       rc = h[detector+'trackSlopesXL'].ProjectionY("slopeYL").Draw()
       self.M.myPrint(self.M.h[detector+'trackDir'],detector+'trackDir',subdir='scifi')
       ut.bookCanvas(h,detector+'TtrackPos',"track position first state",1200,800,1,2)
       h[detector+'TtrackPos'].cd(1)
       rc = h[detector+'trackPosBeam'].Draw('colz')
       h[detector+'TtrackPos'].cd(2)
       rc = h[detector+'trackPos'].Draw('colz')
       self.M.myPrint(self.M.h[detector+'TtrackPos'],detector+'trackPos',subdir='scifi')
