#!/usr/bin/env python
import ROOT,os,sys
import rootUtils as ut
import shipunit as u
import ctypes
from array import array
import numpy as np

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
       if self.M.fsdict or self.M.hasBunchInfo :   self.xing = {'':True,'B1only':False,'B2noB1':False,'noBeam':False}
       else:   self.xing = {'':True}
       self.mat_label = {}
       self.nScifi = self.M.Scifi.GetConfParI("Scifi/nscifi")
       self.nMats = self.M.Scifi.GetConfParI("Scifi/nmats")
       self.nMatsAll = self.nScifi*self.nMats*2
       for xi in self.xing:
        for s in range(2*self.nScifi):
          ut.bookHist(h,detector+'posX_'+str(s)+xi,'x; x [cm]',2000,-100.,100.)
          ut.bookHist(h,detector+'posY_'+str(s)+xi,'y; y[cm]',2000,-100.,100.)
          if s%2==1: ut.bookHist(h,detector+'mult_'+str(s)+xi,'mult vertical station '+str(s//2+1)+'; #hits',100,-0.5,99.5)
          else: ut.bookHist(h,detector+'mult_'+str(s)+xi,'mult horizontal station '+str(s//2+1)+'; #hits',100,-0.5,99.5)
        for mat in range(self.nMatsAll):
          s = mat//6
          p = 'H'
          if mat%6>2: p='V'
          m = mat%3
          ut.bookHist(h,detector+'mat_'+str(mat)+xi,'hit map station '+str(s)+p+' mat '+str(m)+'; #channel',512,-0.5,511.5)
          ut.bookHist(h,detector+'sig_'+str(mat)+xi,'signal '+str(s)+p+' mat '+str(m)+'; QDC [a.u.]',200,-50.0,150.)
          ut.bookHist(h,detector+'tdc_'+str(mat)+xi,'tdc '+str(s)+p+' mat '+str(m)+'; timestamp [LHC clock cycles]',200,-1.,4.)
          if xi=='': self.mat_label[mat] = str(s)+p+' mat '+str(m)
   def ExecuteEvent(self,event):
       h = self.M.h
       W = self.M.Weight
       mult = [0]*10
       for aHit in event.Digi_ScifiHits:
          if not aHit.isValid(): continue
          X =  self.M.Scifi_xPos(aHit.GetDetectorID())
          self.M.fillHist1(detector+'mat_'+str(X[0]*3+X[1]),X[2])
          self.M.fillHist1(detector+'sig_'+str(X[0]*3+X[1]),aHit.GetSignal(0))
          self.M.fillHist1(detector+'tdc_'+str(X[0]*3+X[1]),aHit.GetTime(0))
          self.M.Scifi.GetSiPMPosition(aHit.GetDetectorID(),A,B)
          if aHit.isVertical(): self.M.fillHist1(detector+'posX_'+str(X[0]),A[0])
          else:                 self.M.fillHist1(detector+'posY_'+str(X[0]),A[1])
          mult[X[0]]+=1
       for s in range(10):
          self.M.fillHist1(detector+'mult_'+str(s),mult[s])
   def Plot(self):
      h = self.M.h
      # parameters for the signal median
      q05=np.array([0.5])
      med=np.array([0.])
      signal_attributes={}
      for xi in self.xing:
       if not self.M.fsdict and not self.M.hasBunchInfo and xi!='': continue
       ut.bookCanvas(h,detector+'hitmaps'+xi,' ',1024,768,6,5)
       ut.bookCanvas(h,detector+'signal'+xi,' ',1024,768,6,5)
       ut.bookCanvas(h,detector+'tdc'+xi,' ',1024,768,6,5)
       for mat in range(self.nMatsAll):
           tc = self.M.h[detector+'hitmaps'+xi].cd(mat+1)
           tc.SetLogy(1)
           self.M.h[detector+'mat_'+str(mat)+xi].Draw()
           ROOT.gStyle.SetOptStat(10)
           tc = self.M.h[detector+'signal'+xi].cd(mat+1)
           rc = self.M.h[detector+'sig_'+str(mat)+xi]
           rc.Draw()
           rc.GetQuantiles(1,med,q05)
           signal_attributes[mat]={"median":med[0],
                                   "std":rc.GetStdDev(),
                                   "max":rc.FindLastBinAbove(0),
                                   "percent_overflow":rc.GetBinContent(rc.GetNbinsX()+1)/rc.Integral()*100. if rc.Integral()>0 else 0.}
           tc = self.M.h[detector+'tdc'+xi].cd(mat+1)
           self.M.h[detector+'tdc_'+str(mat)+xi].Draw()

# summary canvases of the median, maximum and overflow of signals
       # overflow is always 0, so skip it
       ut.bookCanvas(h,detector+'signalsSummary'+xi,' ',1024,768,1,1)
       signal_medians, signal_maxima = ROOT.TGraphErrors(), ROOT.TGraphErrors()
       signal_medians.SetTitle("Median of signal per mat")
       signal_medians.GetYaxis().SetTitle("median QDC [a.u.]")
       signal_maxima.SetTitle("Maximal signal per mat")
       signal_maxima.GetYaxis().SetTitle("maximum QDC [a.u.]")
       Area = {}
       for mat in range(self.nMatsAll):
          signal_medians.SetPoint(mat, mat,signal_attributes[mat]["median"])
          signal_medians.SetPointError(mat,0,signal_attributes[mat]["std"])
          signal_maxima.SetPoint(mat, mat,signal_attributes[mat]["max"])
          signal_maxima.SetPointError(mat,0,0)
       graph_list = [signal_medians]
       for counter, graph in enumerate(graph_list):
         h[detector+'signalsSummary'+xi].cd(counter+1)
         ROOT.gPad.SetBottomMargin(0.2)
         ROOT.gPad.SetGrid(0)
         graph.Draw('AP')
         xAxis = graph.GetXaxis()
         # get rid of the original ticks
         xAxis.SetTickLength(0)
         ymin = graph.GetHistogram().GetMinimum()
         ymax = graph.GetHistogram().GetMaximum()
         for mat in range(self.nMatsAll):
           bin_index = xAxis.FindBin(mat)
           xAxis.SetBinLabel(bin_index,self.mat_label[mat])
           # Draw custom grid lines for the X axis
           grid = ROOT.TLine(graph.GetPointX(mat), ymin, graph.GetPointX(mat), ymax)
           grid.SetLineStyle(3)
           grid.DrawClone()
           # Draw ticks
           tick = ROOT.TLine(graph.GetPointX(mat), ymin, graph.GetPointX(mat), ymin + 0.03*(ymax-ymin))
           tick.DrawClone()
         # redraw the graph
         graph.Draw('P,same')
         # draw a bkg 
         Area[counter] = ROOT.TBox(ROOT.TMath.MinElement(graph.GetN(), graph.GetX()),
                          graph.GetHistogram().GetMinimum(), 
                          ROOT.TMath.MaxElement(graph.GetN(), graph.GetX()),
                          graph.GetHistogram().GetMaximum())
         Area[counter].SetLineWidth(0)
         Area[counter].SetFillStyle(3003)
         Area[counter].SetFillColor(ROOT.kBlue) 
         Area[counter].Draw("same")
         graph.SetMarkerStyle(20)
         graph.SetMarkerColor(ROOT.kBlue)
         graph.SetLineColor(ROOT.kBlue)
# end of the summary QDC shifter canvases

       ut.bookCanvas(h,detector+'positions'+xi,' ',2048,768,5,2)
       ut.bookCanvas(h,detector+'mult'+xi,' ',2048,768,5,2)
       for s in range(5):
           tc = self.M.h[detector+'positions'+xi].cd(s+1)
           self.M.h[detector+'posY_'+str(2*s)+xi].Draw()
           tc = self.M.h[detector+'positions'+xi].cd(s+6)
           self.M.h[detector+'posX_'+str(2*s+1)+xi].Draw()

           tc = self.M.h[detector+'mult'+xi].cd(s+1)
           tc.SetLogy(1)
           self.M.h[detector+'mult_'+str(2*s)+xi].Draw()
           tc = self.M.h[detector+'mult'+xi].cd(s+6)
           tc.SetLogy(1)
           self.M.h[detector+'mult_'+str(2*s+1)+xi].Draw()

       for item in h:
          if isinstance(h[item], ROOT.TH2):
            h[item].SetStats(0)
       
       canvas = detector+'signalsSummary'+xi
       self.M.h[canvas].Update()
       if xi!='': self.M.myPrint(self.M.h[canvas],"Scifi-"+canvas,subdir='scifi/shifter/'+xi)
       else:     self.M.myPrint(self.M.h[canvas],"Scifi-"+canvas,subdir='scifi/shifter')

       for canvas in [detector+'hitmaps'+xi,detector+'signal'+xi,detector+'mult'+xi]:
           self.M.h[canvas].Update()
           if canvas.find('hitmaps')>0: role='shifter'
           else : role='expert'
           if xi!='': self.M.myPrint(self.M.h[canvas],"Scifi-"+canvas,subdir='scifi/'+role+'/'+xi)
           else:     self.M.myPrint(self.M.h[canvas],"Scifi-"+canvas,subdir='scifi/'+role)

class Scifi_residuals(ROOT.FairTask):
   " produce residuals for Scifi"
   def Init(self,options,monitor):
       NbinsRes = options.ScifiNbinsRes
       xmin        = options.Scifixmin
       alignPar   = options.ScifialignPar
       self.unbiased = options.ScifiResUnbiased

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
       ut.bookHist(h,'dx','DS track X - scifi X; X[cm]',100,-20.,20.)
       ut.bookHist(h,'dy','DS track Y - scifi Y; Y[cm]',100,-20.,20.)

       ut.bookHist(h,detector+'trackChi2/ndof','track chi2/ndof vs ndof; #chi^{2}/Ndof; Ndof',100,0,100,20,0,20)
# type of crossing, check for b1only,b2nob1,nobeam
       self.xing = {'':True,'B1only':False,'B2noB1':False,'noBeam':False}
       '''
       for xi in self.xing:
          if not self.M.fsdict and not self.M.hasBunchInfo and xi!='': continue
          ut.bookHist(h,detector+'trackSlopes'+xi,'track slope; x/z [mrad]; y/z [mrad]',1000,-100,100,1000,-100,100)
          ut.bookHist(h,detector+'trackSlopesXL'+xi,'track slope; x/z [rad]; y/z [rad]',2200,-1.1,1.1,2200,-1.1,1.1)
          ut.bookHist(h,detector+'trackPos'+xi,'track pos; x [cm]; y [cm]',100,-90,10.,80,0.,80.)
          ut.bookHist(h,detector+'trackPosBeam'+xi,'beam track pos slopes<0.1rad; x [cm]; y [cm]',100,-90,10.,80,0.,80.)
          for item in h:
            if isinstance(h[item], ROOT.TH2) and (item.find('trackPos')>0 or item.find('trackSlopes')>0): 
              h[item].SetTitleOffset(1.1, 'Y')
       '''
       if alignPar:
            for x in alignPar:
               self.M.Scifi.SetConfPar(x,alignPar[x])
               
       self.zExVeto = self.M.zPos['MuFilter'][10]
       self.zEx = self.M.zPos['Scifi'][10]
       self.res = 10.

   def ExecuteEvent(self,event):
       h = self.M.h
       W = self.M.Weight
       nav = self.nav
       if not hasattr(event,"Cluster_Scifi"):
               self.trackTask.scifiCluster()
               clusters = self.trackTask.clusScifi
       else:
               clusters = event.Cluster_Scifi
# overall tracking
       MufiTracks    = []
       ScifiTracks = []
       k = -1
       for theTrack in self.M.Reco_MuonTracks:
          k+=1
          fitStatus = theTrack.getFitStatus()
          if not fitStatus.isFitConverged(): continue
          if theTrack.GetUniqueID()==1:   ScifiTracks.append(k)
          if theTrack.GetUniqueID()==3:   MufiTracks.append(k)
       if len(MufiTracks)==0 or len(ScifiTracks)==0: return
       vetoHits = []
       k = -1
       for aHit in event.Digi_MuFilterHits:
           k+=1
           Minfo = self.M.MuFilter_PlaneBars(aHit.GetDetectorID())
           s,l,bar = Minfo['station'],Minfo['plane'],Minfo['bar']
           if s>1: continue
           X = aHit.GetAllSignals()
           if len(X)<5: continue   # number of fired SiPMs
           vetoHits.append(k)
       xExTag,yExTag = -10000,-10000
       for kMu in MufiTracks:
          theTrack = self.M.Reco_MuonTracks[kMu]
          fstate =  theTrack.getFittedState()
          posT,momT  = fstate.getPos(),fstate.getMom()
          slopeXT = momT.X()/momT.Z()
          slopeYT = momT.Y()/momT.Z()
          if not abs(slopeXT)<0.1 or not abs(slopeYT)<0.1: continue
          lam      = (self.zEx-posT.z())/momT.z()
          yExTag      = posT.y()+lam*momT.y()
          xExTag      = posT.x()+lam*momT.x()
          # eventually require hit in veto to remove ghost tracks
       if xExTag < -9000: return
       ok = False
       for k in vetoHits:
              aHit = event.Digi_MuFilterHits[k]
              self.M.MuFilter.GetPosition(aHit.GetDetectorID(),A,B)
# calculate DOCA
              lam = (self.zExVeto-posT.z())/momT.z()
              xExV,yExV = posT.x()+lam*momT.x(),posT.y()+lam*momT.y()
              pq = A-posT
              uCrossv= (B-A).Cross(momT)
              doca = pq.Dot(uCrossv)/uCrossv.Mag()
              if abs(doca)<self.res: 
                ok=True
                break
       if not ok: return
       # DS track confirmed by Veto hit
           
       theTrack = False
       for theTrack in self.M.Reco_MuonTracks:
          if theTrack.GetUniqueID()==1:
              fitStatus = theTrack.getFitStatus()
              if  fitStatus.isFitConverged():
                 state = theTrack.getFittedState()
                 pos    = state.getPos()
                 mom = state.getMom()
                 slopeX = mom.X()/mom.Z()
                 slopeY = mom.Y()/mom.Z()
                 rc = h[detector+'trackChi2/ndof'].Fill(fitStatus.getChi2()/(fitStatus.getNdf()+1E-10),fitStatus.getNdf())
                 '''
                 self.M.fillHist2(detector+'trackSlopes',slopeX*1000-pos.X()/48.2,slopeY*1000-pos.Y()/48.2)
                 self.M.fillHist2(detector+'trackSlopesXL',slopeX-pos.X()/48200,slopeY-pos.Y()/48200)
                 self.M.fillHist2(detector+'trackPos',pos.X(),pos.Y())
                 if abs(slopeX)<0.1 and abs(slopeY)<0.1:  self.M.fillHist2(detector+'trackPosBeam',pos.X(),pos.Y())
                 '''

       if not theTrack: return

       sortedClusters={}
       for aCl in clusters:
           so = aCl.GetFirst()//100000
           if not so in sortedClusters: sortedClusters[so]=[]
           sortedClusters[so].append(aCl)
# select events with clusters in each plane for making residuals
       if len(sortedClusters)<10: return
       goodEvent = True
       for s in sortedClusters:
          if len(sortedClusters[s])>3:
             goodEvent=False
             break
       if not goodEvent: return

       for s in range(1,6):
            if self.unbiased:
# build trackCandidate
              hitlist = {}
              if self.unbiased or s==1:
                k=0
                Nproj = {0:0,1:0}
                for so in sortedClusters:
                    if so//10 == s and self.unbiased: continue
                    Nproj[so%2]+=1
                    for x in sortedClusters[so]:
                       hitlist[k] = x
                       k+=1
                if Nproj[0]<3 or Nproj[1]<3: continue
                theTrack = self.trackTask.fitTrack(hitlist)
                if not hasattr(theTrack,"getFittedState"): continue
# check residuals
                fitStatus = theTrack.getFitStatus()
                if not fitStatus.isFitConverged(): 
                  theTrack.Delete()
                  continue
# confirm track with DS track
            fstate =  theTrack.getFittedState()
            pos,mom  = fstate.getPos(),fstate.getMom()
            lam      = (self.zExVeto-pos.z())/mom.z()
            yEx      = pos.y()+lam*mom.y()
            xEx      = pos.x()+lam*mom.x()
            dx = xExTag-xEx
            dy = yExTag-yEx
            rc = h['dx'].Fill(dx)
            rc = h['dy'].Fill(dy)
            if abs(dy)>self.res or abs(dx)>self.res:  
               if self.unbiased: continue
               else:             break

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
                   st   = ROOT.getFittedState(theTrack,m)
                   if not st: break
                   Pos = st.getPos()
                   if abs(z-Pos.z())<mZmin:
                      mZmin = abs(z-Pos.z())
                      mClose = m
                if mZmin>10000:
                    print("something wrong here with measurements",mClose,mZmin,theTrack.getNumPointsWithMeasurement())
                    break
                fstate =  theTrack.getFittedState(mClose)
                pos,mom = fstate.getPos(),fstate.getMom()
                rep.setPosMom(state,pos,mom)
                NewPosition = ROOT.TVector3(0., 0., z)   # assumes that plane in global coordinates is perpendicular to z-axis, which is not true for TI18 geometry.
                rep.extrapolateToPlane(state, NewPosition, parallelToZ )
                pos = state.getPos()
                xEx,yEx = pos.x(),pos.y()
                rc = h['track_Scifi'+str(testPlane)].Fill(xEx,yEx,W)
                for aCl in sortedClusters[testPlane]:
                   aCl.GetPosition(A,B)
                   detID = aCl.GetFirst()
                   channel = detID%1000 + ((detID%10000)//1000)*128 + (detID%100000//10000)*512
# calculate DOCA
                   pq = A-pos
                   uCrossv= (B-A).Cross(mom)
                   doca = pq.Dot(uCrossv)/uCrossv.Mag()
                   rc = h['resC'+self.projs[o]+'_Scifi'+str(testPlane)].Fill(doca/u.um,channel,W)
                   rc = h['res'+self.projs[o]+'_Scifi'+str(testPlane)].Fill(doca/u.um,W)
                   rc = h['resX'+self.projs[o]+'_Scifi'+str(testPlane)].Fill(doca/u.um,xEx,W)
                   rc = h['resY'+self.projs[o]+'_Scifi'+str(testPlane)].Fill(doca/u.um,yEx,W)

            if self.unbiased: theTrack.Delete()

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
           self.M.myPrint(self.M.h[canvas],"Scifi-"+canvas,subdir='scifi/expert')
       '''
       for xi in self.xing:
           if not self.M.fsdict and not self.M.hasBunchInfo and xi!='': continue
           tname = detector+'trackDir'+xi
           ut.bookCanvas(h,tname,"track directions",1600,1800,3,2)
           h[tname].cd(1)
           rc = h[detector+'trackSlopes'+xi].Draw('colz')
           h[tname].cd(2)
           rc = h[detector+'trackSlopes'+xi].ProjectionX("slopeX"+xi)
           rc.Draw()
           rc.SetTitle('track X slope')
           h[tname].cd(3)
           rc = h[detector+'trackSlopes'+xi].ProjectionY("slopeY"+xi)
           rc.Draw()
           rc.SetTitle('track Y slope')
           h[tname].cd(4)
           rc = h[detector+'trackSlopesXL'+xi].Draw('colz')
           h[tname].cd(5)
           rc = h[detector+'trackSlopesXL'+xi].ProjectionX("slopeXL"+xi)
           rc.Draw()
           rc.SetTitle('track X slope')
           h[tname].cd(6)
           rc = h[detector+'trackSlopesXL'+xi].ProjectionY("slopeYL"+xi)
           rc.Draw()
           rc.SetTitle('track Y slope')
           if x=='': self.M.myPrint(self.M.h[tname],tname,subdir='scifi/shifter')
           else:     self.M.myPrint(self.M.h[tname],tname,subdir='scifi/shifter/'+xi)
           tname = detector+'TtrackPos'+xi
           ut.bookCanvas(h,tname,"track position first state",600,1200,1,2)
           h[tname].cd(1)
           rc = h[detector+'trackPosBeam'+xi].Draw('colz')
           h[tname].cd(2)
           rc = h[detector+'trackPos'+xi].Draw('colz')
           if x=='': self.M.myPrint(self.M.h[tname],detector+'trackPos'+xi,subdir='scifi/shifter')
           else:     self.M.myPrint(self.M.h[tname],detector+'trackPos'+xi,subdir='scifi/shifter/'+xi)
       '''
           
class Scifi_trackEfficiency(ROOT.FairTask):
   " track efficiency tag with DS track"
   def Init(self,options,monitor):
       self.M = monitor
       self.debug = False
       h = self.M.h
       ut.bookHist(h,'DStag','DS track X/Y at scifi 1; X[cm]; Y[cm]',100,-50,0.,100,10,60)
       ut.bookHist(h,'Xdx','DS track X - scifi X; X[cm]',100,-20.,20.)
       ut.bookHist(h,'Xdy','DS track Y - scifi Y; Y[cm]',100,-20.,20.)
       ut.bookHist(h,'scifiTrack','scifi track X/Y at scifi 1; X[cm]; Y[cm]',100,-50,0.,100,10,60)
       ut.bookHist(h,'USQDC','US QDC for matched tracks; qdc',220,-10,2190.)
       ut.bookHist(h,'clSize','Scifi cluster size for matched tracks; n hits',10,-0.5,9.5)
       ut.bookHist(h,'hitPerPlane','Scifi hits per detector ; n hits',50,-0.5,49.5)
       ut.bookHist(h,'flightDir','flight direction',100,-20.,20.)
       for s in range(6):
           ut.bookHist(h,'XscifiTrack_'+str(s),'scifi track X/Y at scifi 1 missing station '+str(s)+'; X[cm]; Y[cm]',100,-50,0.,100,10,60)
           if s==0: continue
           ut.bookHist(h,'XscifiTrack_0'+str(s),'scifi track X/Y at scifi 1 missing station '+str(s)+'; X[cm]; Y[cm]',100,-50,0.,100,10,60)
           for proj in range(2):
               ut.bookHist(h,'XscifiTrack_'+str(10*s+proj),'scifi track X/Y at scifi 1 missing plane '+str(s*10+proj)+'; X[cm]; Y[cm]',100,-50,0.,100,10,60)
       ut.bookHist(h,'2scifiTrack_1','scifi track X/Y at scifi 1 missing station 1 and 2; X[cm]; Y[cm]',100,-50,0.,100,10,60)
       ut.bookHist(h,'2scifiTrack_01','scifi track X/Y at scifi 1 missing station 1 and 2; X[cm]; Y[cm]',100,-50,0.,100,10,60)
       for proj in range(2):
               ut.bookHist(h,'2scifiTrack_'+str(10*1+proj),'scifi track X/Y at scifi 1 missing plane '+str(proj)+' in 1 and 2; X[cm]; Y[cm]',100,-50,0.,100,10,60)
       self.zEx = self.M.zPos['Scifi'][10]
       self.zExVeto = self.M.zPos['MuFilter'][10]
       self.res = 10.
       self.unbiased = options.ScifiResUnbiased
       self.masked = options.ScifiStationMasked
       self.deadTime = 100
       self.eventBefore={'T':-1,'N':-1,'hits':{1:0,0:0,'0L':0,'0R':0,'1L':0,'1R':0}}
   def ExecuteEvent(self,event):
       vetoHitsFromPrev = 0
       if event.EventHeader.GetRunId() < 6204 and event.EventHeader.GetRunId() > 5480: vetoHitsFromPrev = 5
       # special treatment for first 10fb-1 in 2023, wrong time alignment, again!
       N1 = event.GetReadEntry()
       Tcurrent = event.EventHeader.GetEventTime()
       dT = abs(Tcurrent-self.eventBefore['T'])
       prevAdded = False
       vetoHits = []
       USQDC = 0
       for j in [0,-1]:
         if j<0 and N1>0: 
              if dT > vetoHitsFromPrev: continue
              rc = event.GetEvent(N1-1)  # add veto hits from prev event
              prevAdded = True
         for aHit in event.Digi_MuFilterHits:
           Minfo = self.M.MuFilter_PlaneBars(aHit.GetDetectorID())
           s,l,bar = Minfo['station'],Minfo['plane'],Minfo['bar']
           if s==2 and j==0:  USQDC += aHit.SumOfSignals()['Sum']
           if s>1: continue
           X = aHit.GetAllSignals()
           if len(X)<5: continue   # number of fired SiPMs
           vetoHits.append(aHit.GetDetectorID())
       if prevAdded and N1>1:
         rc = event.GetEvent(N1-2)
         Tprevprev = event.EventHeader.GetEventTime()
         dT = abs(Tcurrent-Tprevprev)
       if prevAdded: event.GetEvent(N1)
       prevEvent = False
       if dT < self.deadTime and dT > vetoHitsFromPrev:
           return
       self.eventBefore['T'] = Tcurrent

       h = self.M.h
       W = self.M.Weight
       MufiTracks    = []
       ScifiTracks = []
       k = -1
       for theTrack in self.M.Reco_MuonTracks:
          k+=1
          fitStatus = theTrack.getFitStatus()
          if not fitStatus.isFitConverged(): continue
          if theTrack.GetUniqueID()==1:   ScifiTracks.append(k)
          if theTrack.GetUniqueID()==3:   MufiTracks.append(k)
       if len(MufiTracks)==0: return
          
       xExTag, yExTag = -10000,-10000
       for kMu in MufiTracks:
          theTrack = self.M.Reco_MuonTracks[kMu]
          fstate =  theTrack.getFittedState()
          posT,momT  = fstate.getPos(),fstate.getMom()
          slopeXT = momT.X()/momT.Z()
          slopeYT = momT.Y()/momT.Z()
          if not abs(slopeXT)<0.1 or not abs(slopeYT)<0.1: continue
          lam      = (self.zEx-posT.z())/momT.z()
          yExTag      = posT.y()+lam*momT.y()
          xExTag      = posT.x()+lam*momT.x()
          # eventually require hit in veto to remove ghost tracks
          ok = False
          for detID in vetoHits:
              self.M.MuFilter.GetPosition(detID,A,B)
# calculate DOCA
              lam = (self.zExVeto-posT.z())/momT.z()
              xExV,yExV = posT.x()+lam*momT.x(),posT.y()+lam*momT.y()
              pq = A-posT
              uCrossv= (B-A).Cross(momT)
              doca = pq.Dot(uCrossv)/uCrossv.Mag()
              if abs(doca)<self.res: 
                ok=True
                break
          if not ok: continue
          rc = h['DStag'].Fill(xExTag,yExTag)
          for kSc in ScifiTracks:
             scifiTrack = self.M.Reco_MuonTracks[kSc]
             fstate =  scifiTrack.getFittedState()
             pos,mom  = fstate.getPos(),fstate.getMom()
             lam      = (self.zEx-pos.z())/mom.z()
             yEx      = pos.y()+lam*mom.y()
             xEx      = pos.x()+lam*mom.x()
             dx = xExTag-xEx
             dy = yExTag-yEx
             rc = h['Xdx'].Fill(dx)
             rc = h['Xdy'].Fill(dy)
             if abs(dy)<self.res and abs(dx)<self.res:
                  rc = h['scifiTrack'].Fill(xExTag,yExTag)
                  rc = h['USQDC'].Fill(USQDC)
             sortedClusters={}
             NscifiTot = 0
             for aCl in self.M.trackTask.clusScifi:
                s = aCl.GetFirst()//100000
                if not (s in sortedClusters): sortedClusters[s]=0
                sortedClusters[s]+=aCl.GetN()
                rc = h['clSize'].Fill(aCl.GetN())
             for s in sortedClusters:
                 rc = h['hitPerPlane'].Fill(sortedClusters[s])
# finished for Scifi track efficiency
# start of Scifi plane inefficiency
       if len(ScifiTracks) < 1: return
       if xExTag< -9000 or not ok: return
       sortedClusters={}
       for aCl in self.M.trackTask.clusScifi:
          so = aCl.GetFirst()//100000
          if not so in sortedClusters: sortedClusters[so]=0
          sortedClusters[so]+=1
       trackClusters = {}
       for x in self.M.trackTask.trackCandidates['Scifi'][0]:
          aCl = self.M.trackTask.trackCandidates['Scifi'][0][x]
          so = aCl.GetFirst()//100000
          if not so in trackClusters: trackClusters[so]=0
          trackClusters[so]+=1

       for s in range(1,6):
       # s is test station
          if self.unbiased:
             # build trackCandidate without s
             self.M.trackTask.maskPlane = s
             self.M.trackTask.fittedTracks.Delete()
             self.M.trackTask.ExecuteTask(option='Scifi')
             if self.M.trackTask.fittedTracks.GetEntries()==0: continue
             theTrack = self.M.trackTask.fittedTracks[0]
             if not hasattr(theTrack,"getFittedState"): continue
             if not theTrack.getFitStatus().isFitConverged():
                self.M.trackTask.fittedTracks.Delete()
                continue
          else:
         # check that test station not required for making track
             nproj = {0:0,1:0}
             for aCl in trackClusters:
                sq = aCl//10
                if sq==s or self.masked.find(str(sq))>-0.5: continue
                nproj[aCl%10]+=1
             if nproj[0]<3 or nproj[1]<3: continue
             theTrack = self.M.Reco_MuonTracks[ScifiTracks[0]]
             
          flightDir = self.M.trackTask.trackDir(theTrack)
          rc = h['flightDir'].Fill(flightDir[0])
          if flightDir[0] < -100:
              if self.unbiased: self.M.trackTask.fittedTracks.Delete()
              continue
          fstate   = theTrack.getFittedState()
          pos,mom  = fstate.getPos(),fstate.getMom()
          lam      = (self.zEx-pos.z())/mom.z()
          yEx      = pos.y()+lam*mom.y()
          xEx      = pos.x()+lam*mom.x()
          dx = xExTag-xEx
          dy = yExTag-yEx
          slopeX = mom.X()/mom.Z()
          slopeY = mom.Y()/mom.Z()
          if not abs(slopeX)<0.1 or not abs(slopeY)<0.1: 
              if self.unbiased: self.M.trackTask.fittedTracks.Delete()
              continue
          if abs(dy)>self.res or abs(dx)>self.res:
              if self.unbiased: self.M.trackTask.fittedTracks.Delete()
              continue
          testPlane = 10*s
          z = self.M.zPos['Scifi'][testPlane]
          lam      = (z-pos.z())/mom.z()
          yEx      = pos.y()+lam*mom.y()
          xEx      = pos.x()+lam*mom.x()
          rc = h['XscifiTrack_0'+str(s)].Fill(xEx,yEx)
          if not ( (s*10 in sortedClusters) or ( (s*10+1)  in sortedClusters) ):
              rc = h['XscifiTrack_'+str(s)].Fill(xEx,yEx)
              if -39<xEx and xEx<-16 and 22<yEx and yEx<47 and self.debug:
                        print('inefficient scifi detector ',s,self.M.EventNumber,xEx,yEx,len(vetoHits))
          if not (s*10 in sortedClusters):
              rc = h['XscifiTrack_'+str(s*10)].Fill(xEx,yEx)
          if not ((s*10+1) in sortedClusters):
              rc = h['XscifiTrack_'+str(s*10+1)].Fill(xEx,yEx)
          if s==1:
             nproj = {0:0,1:0}
             for aCl in trackClusters:
                sq = aCl//10
                if sq==2 or sq==1: continue
                nproj[aCl%10]+=1
             if nproj[0]<3 or nproj[1]<3: continue
             rc = h['2scifiTrack_0'+str(s)].Fill(xEx,yEx)
             if not ( (s*10 in sortedClusters) or ( (s*10+1)  in sortedClusters) or ( (2*10)  in sortedClusters) or ( (2*10+1)  in sortedClusters) ):
                 rc = h['2scifiTrack_'+str(s)].Fill(xEx,yEx)
             if not (s*10 in sortedClusters or 2*10 in sortedClusters ):
               rc = h['2scifiTrack_'+str(s*10)].Fill(xEx,yEx)
             if not ((s*10+1) in sortedClusters or (2*10+1) in sortedClusters):
               rc = h['2scifiTrack_'+str(s*10+1)].Fill(xEx,yEx)
       if self.unbiased:
# restore original track
           self.M.trackTask.maskPlane = -1
           self.M.trackTask.fittedTracks.Delete()
# analysis and plots 
   def Plot(self):
       h = self.M.h
       ut.bookCanvas(h,'dxdy','',1200,1200,2,1)
       tc = h['dxdy'].cd(1)
       h['Xdx'].Draw()
       tc = h['dxdy'].cd(2)
       h['Xdy'].Draw()
       self.M.myPrint(h['dxdy'],'ScifiMufiPulls',subdir='scifi/expert')
       ut.bookCanvas(h,'scifiEff','',1600,800,3,1)
       tc = h['scifiEff'].cd(1)
       h['DStag'].Draw('colz')
       tc = h['scifiEff'].cd(2)
       h['scifiTrack'].Draw('colz')
       h['eff']=h['scifiTrack'].Clone('eff')
       h['eff'].Divide(h['DStag'])
       tc = h['scifiEff'].cd(3)
       h['eff'].DrawCopy('colz')
       self.M.myPrint(h['scifiEff'],'ScifiTrackEfficiency',subdir='scifi/expert')
       limits = {1:{'X':[-44,-8],'Y':[16,50]},2:{'X':[-40,-12],'Y':[18,47]}}
       bins = {}
       for l in limits :
         bins[l] = []
         for p in limits[l]:
           for x in limits[l][p]:
             bins[l].append(eval('h["DStag"].Get'+p+'axis().FindBin(x)'))
         e = h['scifiTrack'].Integral(bins[l][0],bins[l][1],bins[l][2],bins[l][3])/h['DStag'].Integral(bins[l][0],bins[l][1],bins[l][2],bins[l][3])*100
         print('average efficiency: %5.2F<X<%5.2F %5.2F<Y<%5.2F = %5.2F%%'%(limits[l]['X'][0],limits[l]['X'][1],limits[l]['Y'][0],limits[l]['Y'][1],e))
       # station inefficiency
       ut.bookCanvas(h,'Tsineff','',1200,900,3,2)
       sRef=1
       border = [14.964849051657234, 54.00431913184336, -46.21974599493218, -7.146496817384938]  # scifi 1 boundaries
       h['TlineTop'+str(sRef)] = ROOT.TLine(border[2],border[1],border[3],border[1])
       h['TlineBot'+str(sRef)] = ROOT.TLine(border[2],border[0],border[3],border[0])
       h['TlineLef'+str(sRef)] = ROOT.TLine(border[2],border[0],border[2],border[1])
       h['TlineRig'+str(sRef)] = ROOT.TLine(border[3],border[0],border[3],border[1])
       for x in ['TlineTop'+str(sRef),'TlineBot'+str(sRef),'TlineLef'+str(sRef),'TlineRig'+str(sRef)]: 
           h[x].SetLineWidth(2)
           h[x].SetLineColor(ROOT.kRed)
       for s in range(1,6):
          tc = h['Tsineff'].cd(s)
          tc.SetRightMargin(0.1)
          tc.SetLogz(1)
          h['sineff'+str(s)] = h['XscifiTrack_'+str(s)].Clone('sineff'+str(s))
          h['sineff'+str(s)].Divide(h['XscifiTrack_0'])
          h['sineff'+str(s)].SetStats(0)
          h['sineff'+str(s)].SetMaximum(0.1)
          h['sineff'+str(s)].DrawCopy('colz')
          for x in ['TlineTop'+str(sRef),'TlineBot'+str(sRef),'TlineLef'+str(sRef),'TlineRig'+str(sRef)]: h[x].Draw('same')
          tc.Update()
          for l in limits :
            e = h['XscifiTrack_'+str(s)].Integral(bins[l][0],bins[l][1],bins[l][2],bins[l][3])/h['scifiTrack'].Integral(bins[l][0],bins[l][1],bins[l][2],bins[l][3])
            print('average efficiency station: %2i %5.2F<X<%5.2F %5.2F<Y<%5.2F = %5.2G'%(s,limits[l]['X'][0],limits[l]['X'][1],limits[l]['Y'][0],limits[l]['Y'][1],e))
       self.M.myPrint(h['Tsineff'],'ScifiStationInEfficiency',subdir='scifi/expert')
       ut.bookCanvas(h,'Tqdc','',1200,900,1,1)
       tc = h['Tqdc'].cd()
       h['USQDC'].Draw()
       self.M.myPrint(h['Tqdc'],'US QDC for muon track',subdir='mufilter/expert')
