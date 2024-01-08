import ROOT
from array import array
import shipunit as u
A,B = ROOT.TVector3(),ROOT.TVector3()

ROOT.gInterpreter.Declare("""
#include <KalmanFitterInfo.h>
#include <Track.h>
#include <MeasuredStateOnPlane.h>
#include <stddef.h>     

const genfit::MeasuredStateOnPlane& getFittedState(genfit::Track* theTrack, int nM){
      try{
        return theTrack->getFittedState(nM);
      }
      catch(genfit::Exception& e){
        std::cerr<<"Exception "<< e.what() <<std::endl;
        const genfit::MeasuredStateOnPlane* state(NULL);
        return *state;
      }
}
""")

class Tracking(ROOT.FairTask):
 " Tracking "
 def Init(self,online=False):
   geoMat = ROOT.genfit.TGeoMaterialInterface()
   bfield = ROOT.genfit.ConstField(0,0,0)   # constant field of zero
   fM = ROOT.genfit.FieldManager.getInstance()
   fM.init(bfield)
   ROOT.genfit.MaterialEffects.getInstance().init(geoMat)
   ROOT.genfit.MaterialEffects.getInstance().setNoEffects()
   lsOfGlobals  = ROOT.gROOT.GetListOfGlobals()
   self.scifiDet = lsOfGlobals.FindObject('Scifi')
   self.mufiDet = lsOfGlobals.FindObject('MuFilter')
   
   # internal storage of clusters
   self.clusScifi   = ROOT.TObjArray(100)
   self.DetID2Key = {}
   self.clusMufi  = ROOT.TObjArray(100)
   
   self.fitter = ROOT.genfit.KalmanFitter()
   self.fitter.setMaxIterations(50)
   #internal storage of fitted tracks
   # output is in genfit::track or sndRecoTrack format
   # default is genfit::Track
   if hasattr(self, "genfitTrack"): pass
   else: self.genfitTrack = True
   
   if self.genfitTrack:
        self.fittedTracks = ROOT.TObjArray(10)
   else:
        self.fittedTracks = ROOT.TClonesArray("sndRecoTrack", 10)
   self.sigmaScifi_spatial = 2*150.*u.um
   self.sigmaMufiUS_spatial = 2.*u.cm
   self.sigmaMufiDS_spatial = 0.3*u.cm
   self.scifi_vsignal = 15.*u.cm/u.ns
   self.firstScifi_z = 300*u.cm
   self.dZ = 13.   # distance between scifi stations = 13cm
   self.multipleTrackStore = {}
   self.Debug = False
   self.ioman = ROOT.FairRootManager.Instance()
   self.sink = self.ioman.GetSink()
   # online mode:         raw data in, converted data in output
   # offline read only:   converted data in, no output
   # offline read/write:  converted data in, converted data out

   # for scifi tracking
   self.nPlanes = 3
   self.nClusters = 5
   self.sigma=150*u.um
   self.maxRes=50
   self.maskPlane=-1
   # for DS tracking
   self.DSnPlanes = 3
   self.DSnHits = 5
   self.nDSPlanesVert  = self.mufiDet.GetConfParI("MuFilter/NDownstreamPlanes")
   self.nDSPlanesHor = self.nDSPlanesVert-1

   if online:
      self.event = self.sink.GetOutTree()
   else: 
      self.event = self.ioman.GetInChain()     # should contain all digis, but not necessarily the tracks and scifi clusters

   self.systemAndPlanes  = {1:2,2:5,3:7}
   return 0
 
 def SetTrackClassType(self,genfitTrack):
     self.genfitTrack = genfitTrack
 
 def FinishEvent(self):
  pass

 def ExecuteTask(self,option='ScifiDS'):
    self.trackCandidates = {}
    if not option.find('DS')<0:
           self.clusMufi.Delete()
           self.dsCluster()
           self.trackCandidates['DS'] = self.DStrack()
    if not option.find('Scifi')<0:
           self.clusScifi.Delete()
           self.scifiCluster()
           self.trackCandidates['Scifi'] = self.Scifi_track()
    i_muon = -1
    for x in self.trackCandidates:
      for aTrack in self.trackCandidates[x]:
           rc = self.fitTrack(aTrack)
           if type(rc)==type(1):
                print('trackfit failed',rc,aTrack)
           else:
                i_muon += 1
                if x=='DS':   rc.SetUniqueID(3)
                if x=='Scifi': rc.SetUniqueID(1)
                # add the tracks
                if self.genfitTrack:
                    self.fittedTracks.Add(rc)
                else:
                    # Load items into snd track class object
                    #if not rc.getFitStatus().isFitConverged(): continue
                    this_track = ROOT.sndRecoTrack(rc)
                    pointTimes = ROOT.std.vector(ROOT.std.vector('float'))()
                    if x=='DS':
                       for pnt in rc.getPointsWithMeasurement():
                           hitID = pnt.getRawMeasurement().getHitId()
                           aCl = self.clusMufi[hitID]
                           pointTimes.push_back([aCl.GetTime()])
                    if x=='Scifi':
                       for pnt in rc.getPointsWithMeasurement():
                           hitID = pnt.getRawMeasurement().getHitId()
                           aCl = self.clusScifi[hitID]
                           pointTimes.push_back([aCl.GetTime()])
                    this_track.setRawMeasTimes(pointTimes)
                    this_track.setTrackType(rc.GetUniqueID())
                    # Store the track in sndRecoTrack format
                    self.fittedTracks[i_muon] = this_track
            

 def DStrack(self):
    event = self.event
    trackCandidates = []
    clusters = self.clusMufi

    stations = {}
    clusPerStation = {}
    planesPerProjection = {0:0,1:0}
    s = 3
    for p in range(self.systemAndPlanes[s]+1): 
          stations[s*10+p] = {}
          clusPerStation[s*10+p] = 0
    k=-1
    for aCl in clusters:
         k+=1
         detID = aCl.GetFirst()
         if detID//10000 < 3: continue
         p = (detID//1000)%10
         bar = detID%1000
         plane = s*10+p
         if bar<60: 
           plane = s*10+2*p  # even numbers horizontal planes, odd numbers vertical planes
         else:  
           plane = s*10+2*p+1
         stations[plane][k] = aCl
         clusPerStation[plane] +=1
    for p in clusPerStation:
       if clusPerStation[p]>0:
          planesPerProjection[p%2]+=1

    failed = False
    if planesPerProjection[1]<self.DSnPlanes or planesPerProjection[0]<self.DSnPlanes: return trackCandidates
    if self.DSnPlanes==2 and (planesPerProjection[1]==2 or planesPerProjection[0]==2):
    # require max 1 cluster per plane if only 2 planes hit per projection
       for p in clusPerStation:
         if clusPerStation[p]>1:
            failed = True
            break
       if failed: return trackCandidates
       hitlist = {}
       for p in stations:
         for k in stations[p]:  hitlist[k] = stations[p][k]
       trackCandidates.append(hitlist)
       return trackCandidates

    for p in clusPerStation:
         if clusPerStation[p]>self.DSnHits: return trackCandidates

# require one plane with 1 cluster as seed

# proj = 0, horizontal, max 3 planes
# proj = 1, vertex,     max 4 planes
    seed = -1
    combinations = {}
    hitlist = {}
    for proj in range(2):
      for plane in range(self.systemAndPlanes[s]+1):
          if not plane%2==proj: continue
          if clusPerStation[s*10+plane]==1:
             seed = s*10+plane
             break
      if seed < 0: return trackCandidates
      combinations[proj] = {}
      for kA in stations[seed]:
         clA = stations[seed][kA]
         clA.GetPosition(A,B)
         posA = (A+B)/2.
         clInPlane = {}
         for p2 in range(self.systemAndPlanes[s]+1):
            if not p2%2==proj: continue
            planeB = s*10+p2
            if planeB == seed: continue
            l = len(stations[planeB])
            if l>0: clInPlane[planeB] = l
         srt = sorted(clInPlane)
         if len(stations[srt[0]])+len(stations[srt[1]]) > self.DSnHits+1: return trackCandidates
         planeB = srt[0]
         for kB in stations[planeB]:
                clB = stations[planeB][kB]
                clB.GetPosition(A,B)
                posB = (A+B)/2.
                delBA = posB-posA
                if proj==0: 
                   lam = delBA[1]/delBA[2]
                   b = posA[1]-lam*posA[2]
                else: 
                   lam = delBA[0]/delBA[2]
                   b = posA[0]-lam*posA[2]
                for p3 in range(self.systemAndPlanes[s]+1):
                   if not p3%2==proj: continue
                   planeC = s*10+p3
                   if planeC == seed or planeC == planeB: continue
                   for kC in stations[planeC]:
                      clC = stations[planeC][kC]
                      clC.GetPosition(A,B)
                      posC = (A+B)/2.
                      eX = posC[2]*lam+b
                      if proj==0: res = abs(eX-posC[1])
                      else:       res = abs(eX-posC[0])
                      if proj==0: combinations[proj][res] = [[seed,kA],[planeB,kB],[planeC,kC]]
                      else:
                         for p4 in range(self.systemAndPlanes[s]+1):
                           if not p4%2==proj: continue
                           planeD = s*10+p4
                           if planeD == seed or planeD == planeB or planeD == planeC: continue
                           if len(stations[planeD])==0:
                               combinations[proj][res] = [[seed,kA],[planeB,kB],[planeC,kC]]
                           for kD in stations[planeD]:
                             clD = stations[planeD][kD]
                             clD.GetPosition(A,B)
                             posD = (A+B)/2.
                             eX = posD[2]*lam+b
                             if proj==0: res+= abs(eX-posD[1])
                             else:       res+= abs(eX-posD[0])
                             combinations[proj][res] = [[seed,kA],[planeB,kB],[planeC,kC],[planeD,kD]]
      # find combination with smallest residual
      srt = sorted(combinations[proj])[0]
      for x in combinations[proj][srt]:
         hitlist[x[1]] = stations[x[0]][x[1]]
    trackCandidates.append(hitlist)
    return trackCandidates

 def Scifi_track(self):
# check for low occupancy and enough hits in Scifi
        event = self.event
        trackCandidates = []
        clusters = self.clusScifi
        stations = {}
        projClusters = {0:{},1:{}}
        for s in range(1,6):
           for o in range(2):
              stations[s*10+o] = []
        k=0      
        for cl in clusters:
            detID = cl.GetFirst()
            s  = detID//1000000
            o = (detID//100000)%10
            if self.maskPlane != s:
                stations[s*10+o].append(detID)
                projClusters[o][detID] = [cl,k]
                k+=1
        nclusters = 0
        check = {}
        ignore = []
        for o in range(2):
            check[o]={}
            for s in range(1,6):
                if len(stations[s*10+o]) > self.nClusters: 
                  ignore.append(s*10+o)
                elif len(stations[s*10+o]) > 0 : 
                  check[o][s] = 1
                  nclusters+=len(stations[s*10+o])
        
        if len(check[0])<self.nPlanes or len(check[1])<self.nPlanes: return trackCandidates
# build trackCandidate
# PR logic, fit straight line in x/y projection, remove outliers. Ignore tilt.
        hitlist = {}
        sortedClusters = {}
        check[0]=0
        check[1]=0
        for o in range(2):
           sortedClusters[o]=sorted(projClusters[o])
           g = ROOT.TGraph()
           n = 0
           mean = {}
           points = {}
           for detID in sortedClusters[o]:
               s  = detID//1000000
               if  (s*10+o) in ignore: continue
               if not s in mean: 
                  mean[s]=[0,0,0]
               projClusters[o][detID][0].GetPosition(A,B)
               z = (A[2]+B[2])/2.
               if o==0: y = (A[1]+B[1])/2.
               else: y = (A[0]+B[0])/2.
               points[detID] = [z,y]
               mean[s][0]+=z
               mean[s][1]+=y
               mean[s][2]+=1
           for s in mean:
              for n in range(2):
                 mean[s][n]=mean[s][n]/mean[s][2]
              g.AddPoint(mean[s][0],mean[s][1])
           rc = g.Fit('pol1','SQ')
           fun = g.GetFunction('pol1')
           for detID in points:
               z = points[detID][0]
               y = points[detID][1]
               res = abs(y-fun.Eval(z))/self.sigma
               if res < self.maxRes:
                 k = projClusters[o][detID][1]
                 hitlist[k] = projClusters[o][detID][0]
                 check[o]+=1
        if check[0]>2 and check[1]>2:
              trackCandidates.append(hitlist)
        return trackCandidates

 def scifiCluster(self):
       clusters = []
       self.DetID2Key.clear()
       hitDict = {}
       for k in range(self.event.Digi_ScifiHits.GetEntries()):
            d = self.event.Digi_ScifiHits[k]
            if not d.isValid(): continue 
            hitDict[d.GetDetectorID()] = k
       hitList = list(hitDict.keys())
       if len(hitList)>0:
              hitList.sort()
              tmp = [ hitList[0] ]
              cprev = hitList[0]
              ncl = 0
              last = len(hitList)-1
              hitvector = ROOT.std.vector("sndScifiHit*")()
              for i in range(len(hitList)):
                   if i==0 and len(hitList)>1: continue
                   c=hitList[i]
                   neighbour = False
                   if (c-cprev)==1:    # does not account for neighbours across sipms
                        neighbour = True
                        tmp.append(c)
                   if not neighbour  or c==hitList[last]:
                        first = tmp[0]
                        N = len(tmp)
                        hitvector.clear()
                        for aHit in tmp: hitvector.push_back( self.event.Digi_ScifiHits[hitDict[aHit]])
                        aCluster = ROOT.sndCluster(first,N,hitvector,self.scifiDet,False)
                        clusters.append(aCluster)
                        if c!=hitList[last]:
                             ncl+=1
                             tmp = [c]
                        elif not neighbour :   # save last channel
                            hitvector.clear()
                            hitvector.push_back( self.event.Digi_ScifiHits[hitDict[c]])
                            aCluster = ROOT.sndCluster(c,1,hitvector,self.scifiDet,False)
                            clusters.append(aCluster)
                   cprev = c
       self.clusScifi.Delete()            
       for c in clusters:  
            self.clusScifi.Add(c)
# map clusters to hit keys
            for nHit in range(self.event.Digi_ScifiHits.GetEntries()):
              if self.event.Digi_ScifiHits[nHit].GetDetectorID()==c.GetFirst():
                 self.DetID2Key[c.GetFirst()] = nHit

 def dsCluster(self):
       clusters = []
       hitDict = {}
       for k in range(self.event.Digi_MuFilterHits.GetEntries()):
            d = self.event.Digi_MuFilterHits[k]
            if (d.GetDetectorID()//10000)<3 or (not d.isValid()): continue
            hitDict[d.GetDetectorID()] = k
       hitList = list(hitDict.keys())
       if len(hitList)>0:
              hitList.sort()
              tmp = [ hitList[0] ]
              cprev = hitList[0]
              ncl = 0
              last = len(hitList)-1
              hitvector = ROOT.std.vector("MuFilterHit*")()
              for i in range(len(hitList)):
                   if i==0 and len(hitList)>1: continue
                   c=hitList[i]
                   neighbour = False
                   if (c-cprev)==1 or (c-cprev)==2:    # allow for one missing channel
                        neighbour = True
                        tmp.append(c)
                   if not neighbour  or c==hitList[last] or c%1000==59:
                        first = tmp[0]
                        N = len(tmp)
                        hitvector.clear()
                        for aHit in tmp: hitvector.push_back( self.event.Digi_MuFilterHits[hitDict[aHit]])
                        aCluster = ROOT.sndCluster(first,N,hitvector,self.mufiDet,False)
                        clusters.append(aCluster)
                        if c!=hitList[last]:
                             ncl+=1
                             tmp = [c]
                        elif not neighbour :   # save last channel
                            hitvector.clear()
                            hitvector.push_back( self.event.Digi_MuFilterHits[hitDict[c]])
                            aCluster = ROOT.sndCluster(c,1,hitvector,self.mufiDet,False)
                            clusters.append(aCluster)
                   cprev = c
       self.clusMufi.Delete()
       for c in clusters:  self.clusMufi.Add(c)

 def patternReco(self):
# very simple for the moment, take all scifi clusters
    trackCandidates = []
    hitlist = {}
    ScifiStations = {}
    for k in range(len(self.event.Cluster_Scifi)):
           hitlist[k] = self.event.Cluster_Scifi[k]
           ScifiStations[hitlist[k].GetFirst()//1000000] = 1
# take fired muonFilter bars if more than 2 SiPMs have fired
    nMin = 1
    MuFiPlanes = {}
    for k in range(self.event.Digi_MuFilterHits.GetEntries()):
         aHit = self.event.Digi_MuFilterHits[k]
         if not aHit.isValid(): continue
         detID = aHit.GetDetectorID()
         sy    = detID//10000
         l       = (detID%10000)//1000  # plane number
         bar = (detID%1000)
         nSiPMs = aHit.GetnSiPMs()
         nSides  = aHit.GetnSides()
         nFired = 0
         for i in range(nSides*nSiPMs):
              if aHit.GetSignal(i) > 0: nFired+=1
         if nMin > nFired: continue
         hitlist[k*1000] = self.event.Digi_MuFilterHits[k]
         MuFiPlanes[sy*100+l] = 1
    if (len(ScifiStations) == 5 or len(MuFiPlanes)>4) and len(hitlist)<20:
           trackCandidates.append(hitlist)
    return trackCandidates

 def fitTrack(self,hitlist):
# hitlist:  clusterID: [A,B] endpoints of scifiCluster
    hitPosLists={}
    trID = 0

    posM    = ROOT.TVector3(0, 0, 0.)
    momM = ROOT.TVector3(0,0,100.)  # default track with high momentum

# approximate covariance
    covM = ROOT.TMatrixDSym(6)
    for k in hitlist:
      aCl = hitlist[k]
      if hasattr(aCl,"GetFirst"):
        detID = aCl.GetFirst()
      else:
        detID = aCl.GetDetectorID()
      if detID<40000: res = self.sigmaMufiDS_spatial
      else:  res = self.sigmaScifi_spatial
      break

    for  i in range(3):   covM[i][i] = res*res
    for  i in range(3,6): covM[i][i] = ROOT.TMath.Power(res / (4.*2.) / ROOT.TMath.Sqrt(3), 2)
    rep = ROOT.genfit.RKTrackRep(13)

# start state
    state = ROOT.genfit.MeasuredStateOnPlane(rep)
    rep.setPosMomCov(state, posM, momM, covM)

# create track
    seedState = ROOT.TVectorD(6)
    seedCov   = ROOT.TMatrixDSym(6)
    rep.get6DStateCov(state, seedState, seedCov)
    theTrack = ROOT.genfit.Track(rep, seedState, seedCov)

# make measurements sorted in z
    unSortedList = {}
    tmpList = {}
    A,B = ROOT.TVector3(),ROOT.TVector3()
    for k in hitlist:
        aCl = hitlist[k]
        if hasattr(aCl,"GetFirst"):
            detID = aCl.GetFirst()
            aCl.GetPosition(A,B)
            detSys  = 1
            if detID<40000: detSys=3
        else:
            detID = aCl.GetDetectorID()
            detSys  = 1
            if detID<40000: detSys=3
            if detSys==3: self.mufiDet.GetPosition(detID,A,B)
            if detSys==1: self.scifiDet.GetSiPMPosition(detID,A,B)
        distance = 0
        tmp = array('d',[A[0],A[1],A[2],B[0],B[1],B[2],distance])
        unSortedList[A[2]] = [ROOT.TVectorD(7,tmp),detID,k,detSys]
    sorted_z=list(unSortedList.keys())
    sorted_z.sort()
    for z in sorted_z:
        tp = ROOT.genfit.TrackPoint() # note how the point is told which track it belongs to
        hitCov = ROOT.TMatrixDSym(7)
        detSys = unSortedList[z][3]
        if detSys==3:      
              res = self.sigmaMufiDS_spatial
              maxDis = 1.0
        elif detSys==2:  
              res = self.sigmaMufiUS_spatial
              maxDis = 5.0
        else:         
              res = self.sigmaScifi_spatial
              maxDis = 0.1
        hitCov[6][6] = res*res
        measurement = ROOT.genfit.WireMeasurement(unSortedList[z][0],hitCov,1,6,tp) # the measurement is told which trackpoint it belongs to
        measurement.setMaxDistance(maxDis)
        measurement.setDetId(unSortedList[z][1])
        measurement.setHitId(unSortedList[z][2])
        tp.addRawMeasurement(measurement) # package measurement in the TrackPoint                                          
        theTrack.insertPoint(tp)  # add point to Track
    if not theTrack.checkConsistency():
        print("track not consistent")
        theTrack.Delete()
        return -2
# do the fit
    self.fitter.processTrack(theTrack) # processTrackWithRep(theTrack,rep,True)
    fitStatus   = theTrack.getFitStatus()
    if self.Debug: print("Fit result: converged chi2 Ndf",fitStatus.isFitConverged(),fitStatus.getChi2(),fitStatus.getNdf())
    if not fitStatus.isFitConverged() and 0>1:
        theTrack.Delete()
        return -1
    if self.Debug: 
        chi2 = fitStatus.getChi2()/(fitStatus.getNdf()+1E-15)
        fittedState = theTrack.getFittedState()
        P = fittedState.getMomMag()
        print("track fitted Ndf #Meas P",fitStatus.getNdf(), theTrack.getNumPointsWithMeasurement(),P)
        for p in theTrack.getPointsWithMeasurement():
            rawM = p.getRawMeasurement()
            info = p.getFitterInfo()
            if not info: continue
            detID = rawM.getDetId()
            print(detID,"weights",info.getWeights()[0],info.getWeights()[1],fitStatus.getNdf())
    return theTrack

 def trackDir(self,theTrack):
      if theTrack.GetUniqueID()>1: return False # for the moment, only the scifi is time calibrated
      fitStatus   = theTrack.getFitStatus()
      if not fitStatus.isFitConverged() : return [100,-100]
      state = ROOT.getFittedState(theTrack,0)
      pos = state.getPos()
      mom = state.getMom()
      lam = (self.firstScifi_z-pos.z())/mom.z()
      # nominal first position
      pos1 = ROOT.TVector3(pos.x()+lam*mom.x(),pos.y()+lam*mom.y(),self.firstScifi_z)

      self.Tline = ROOT.TGraph()
      meanT = 0
      for nM in range(theTrack.getNumPointsWithMeasurement()):
            state = ROOT.getFittedState(theTrack,nM)
            if not state: continue
            posM   = state.getPos()
            M = theTrack.getPointWithMeasurement(nM)
            W = M.getRawMeasurement()
            detID = W.getDetId()
            clkey = W.getHitId()
            aCl = self.clusScifi[clkey]
            aHit = self.event.Digi_ScifiHits[ self.DetID2Key[detID] ]
            self.scifiDet.GetSiPMPosition(detID,A,B)
            if aHit.isVertical(): X = B-posM
            else: X = A-posM
            L = X.Mag()/self.scifi_vsignal
         # need to correct for signal propagation along fibre
            corTime = self.scifiDet.GetCorrectedTime(detID, aCl.GetTime(), 0)
            trajLength = (posM-pos1).Mag()
            T = corTime - L - trajLength/u.speedOfLight
            self.Tline.AddPoint(trajLength,T)
      rc = self.Tline.Fit('pol1','SQ')
      fitResult =  rc.Get()
      slope = fitResult.Parameter(1)
      return [slope,slope/(fitResult.ParError(1)+1E-13),fitResult.Parameter(0)]
      
 def multipleTrackCandidates(self,planesWithClusters=10,nMaxCl=8,dGap=0.2,dMax=0.8,dMax3=0.8,ovMax = 1,doublet=True,debug=False):
       A,B = ROOT.TVector3(),ROOT.TVector3()
       h = self.multipleTrackStore
       h['trackCand'] = {0:{},1:{}}
       h['sortedClusters'] = {}
       sortedClusters = h['sortedClusters']
       self.scifiCluster()
       clusters = self.clusScifi
       for aCl in clusters:
           so = aCl.GetFirst()//100000
           if not so in sortedClusters: 
              sortedClusters[so]=[]
           aCl.GetPosition(A,B)
           if aCl.GetFirst()//100000%10 == 0: pos = A[1]
           else:                              pos = A[0]
           sortedClusters[so].append([pos,A[2],aCl])
# select events with clusters in each plane
       if len(sortedClusters)<planesWithClusters: return
#
       h['mergedClusters'] = {}
       mergedClusters = h['mergedClusters']
       for so in sortedClusters:
          pos={}
          mergedClusters[so]=[]
          for k in range(len(sortedClusters[so])):
             pos[k] = sortedClusters[so][k][0]
          sorted_pos = sorted(pos.items(), key=lambda x: x[1])
          merged = -1
          for i in range(len(sorted_pos)):
              x = sorted_pos[i]
              aClobj = sortedClusters[so][x[0]]
              if merged < 0:
                 merged+=1
                 mergedClusters[so].append( [aClobj[0],aClobj[1],[aClobj[2]]] )
              else:
                 prevPos = mergedClusters[so][merged][0]
                 pos     = aClobj[0]
                 if pos-prevPos > dGap: 
                   merged+=1
                   mergedClusters[so].append( [aClobj[0],aClobj[1],[aClobj[2]]] )
                 else:
                   N = len(mergedClusters[so][merged][2])
                   newPos = (prevPos*N+pos)/(N+1)
                   mergedClusters[so][merged][0]=newPos
                   mergedClusters[so][merged][2].append(aClobj[2])
# not more than nMaxCl in a plane and at least 2
       for so in mergedClusters:
          if len(mergedClusters[so]) > nMaxCl: return
          if len(mergedClusters[so]) < 2: return
       for p in range(2):
         if debug: print('-------- p=',p)
         if doublet:
# method using doublets
          h['doublet'] = {}
          for j1 in range(1,5):
             h['doublet'][j1] = {}
             for k1 in range(len(mergedClusters[j1*10+p])):
                aCl1 = mergedClusters[j1*10+p][k1]
                h['doublet'][j1][k1] = []
                j2 = j1+1
                for k2 in range(len(mergedClusters[j2*10+p])):
                    aCl2 = mergedClusters[j2*10+p][k2]
                    D = aCl2[0]-aCl1[0]
                    if debug: print(j1,j2,'x1',aCl1[0],'x2',aCl2[0],'D',D)
                    if abs(D ) > dMax: continue
                    h['doublet'][j1][k1].append(10*j2+k2)
                if len(h['doublet'][j1][k1]) == 0 and j2<5:   # allow one missing plane
                   j2 = j1+2
                   for k2 in range(len(mergedClusters[j2*10+p])):
                     aCl2 = mergedClusters[j2*10+p][k2]
                     D = aCl2[0]-aCl1[0]
                     if debug: print(j1,j2,'x1',aCl1[0],'x2',aCl2[0],'D',D)
                     if abs(D ) > dMax: continue
                     h['doublet'][j1][k1].append(10*j2+k2)
          h['trackCand'][p] = {}
          for j1 in [1,2]:
           for k1 in h['doublet'][j1]:
            if j1==2:
               alreadyUsed = False
               for k0 in h['doublet'][1]:
                 for sk0 in h['doublet'][1][k0]:
                    if k1== (sk0%10) :
                       alreadyUsed = True
                       break
               if alreadyUsed: continue
            trackId = (k1+1)*10**(j1-1)
            if debug: print(j1,k1,trackId)
            for sk2 in h['doublet'][j1][k1]:
              j2 = sk2//10
              k2 = sk2%10
              trackId = (k1+1)*10**(j1-1) + (k2+1)*10**(j2-1)
              if debug: print(j2,k2,trackId)
              for sk3 in h['doublet'][j2][k2]:
               j3 = sk3//10
               k3 = sk3%10
               if j3>4: continue
               trackId = (k1+1)*10**(j1-1) + (k2+1)*10**(j2-1) + (k3+1)*10**(j3-1)
               if debug: print(j3,k3,trackId)
               for sk4 in h['doublet'][j3][k3]:
                 j4 = sk4//10
                 k4 = sk4%10
                 trackId = (k1+1)*10**(j1-1) + (k2+1)*10**(j2-1) + (k3+1)*10**(j3-1) + (k4+1)*10**(j4-1)
                 if debug: print(j4,k4,trackId)
                 found = False
                 if j4<5:
                    for sk5 in h['doublet'][j4][k4]:
                       found = True
                       j5 = sk5//10
                       k5 = sk5%10
                       trackId = (k1+1)*10**(j1-1) + (k2+1)*10**(j2-1) + (k3+1)*10**(j3-1) + (k4+1)*10**(j4-1) + (k5+1)*10**(j5-1)
                       h['trackCand'][p][trackId] = ROOT.TGraph()
                 if not found:
                    h['trackCand'][p][trackId] = ROOT.TGraph()
          for trackId in h['trackCand'][p]:
              N = -1
              for s in range(1,6):
                k = (trackId//10**(s-1))%10-1
                if not k<0:
                   N+=1
                   aCl = mergedClusters[s*10+p][k]
                   h['trackCand'][p][trackId].SetPoint(N,aCl[1],aCl[0])
# 5 plane combinatoric
         else:
          for k1 in range(len(mergedClusters[10+p])):
            aCl1 = mergedClusters[10+p][k1]
            for k2 in range(len(mergedClusters[20+p])):
               aCl2 = mergedClusters[20+p][k2]
               D = aCl2[0]-aCl1[0]
               if debug: print(2,'x1',aCl1[0],'x2',aCl2[0],'D',D)
               if abs(D) > dMax: continue
               for k3 in range(len(mergedClusters[30+p])):
                  aCl3 = mergedClusters[30+p][k3]
                  D3 = 2*D+aCl1[0]-aCl3[0]
                  if debug: print(3,'x1',aCl1[0],'x2',aCl2[0],'x3',aCl3[0],'D',D,'D3',D3)
                  if abs(D3) > dMax3: continue
                  trackId = 1000+100*k3+10*k2+k1
                  trackId3 = trackId
                  h['trackCand'][p][trackId]=ROOT.TGraph()
                  h['trackCand'][p][trackId].SetPoint(0,aCl1[1],aCl1[0])
                  h['trackCand'][p][trackId].SetPoint(1,aCl2[1],aCl2[0])
                  h['trackCand'][p][trackId].SetPoint(2,aCl3[1],aCl3[0])
                  extr4 = h['trackCand'][p][trackId].Eval(aCl3[1]+self.dZ)
                  for k4 in range(len(mergedClusters[40+p])):
                    aCl4 = mergedClusters[40+p][k4]
                    D4 = aCl4[0]-extr4
                    if debug: print(4,'D4',D4,'x4',aCl4[0],'extr',extr4)
                    if abs(D4) > dMax3: continue
                    trackId = 10000+1000*k4+100*k3+10*k2+k1
                    trackId4 = trackId
                    if debug: print(4,trackId)
                    h['trackCand'][p][trackId] = h['trackCand'][p][trackId3].Clone()
                    h['trackCand'][p][trackId].SetPoint(3,aCl4[1],aCl4[0])
                    extr5 = h['trackCand'][p][trackId].Eval(aCl4[1]+self.dZ)
                    for k5 in range(len(mergedClusters[50+p])):
                       aCl5 = mergedClusters[50+p][k5]
                       D5 = aCl5[0]-extr5
                       if debug: print(5,'D5',D5,'x5',aCl5[0],'extr',extr5)
                       if abs(D5) > dMax3: continue
                       trackId = 100000+10000*k5+1000*k4+100*k3+10*k2+k1
                       h['trackCand'][p][trackId] = h['trackCand'][p][trackId4].Clone()
                       h['trackCand'][p][trackId].SetPoint(4,aCl5[1],aCl5[0])
                       if debug: print('final',trackId)
       '''
clonekiller
if more than 2 clusters are shared by a track, take the track with better chi2
       '''
       h['cloneCand'] = {0:[],1:[]}
       discarded = h['cloneCand']
       for p in range(2):
         keys = list(h['trackCand'][p])
         for k1 in range( len(keys)-1):
            if keys[k1] < 100000 and not doublet: continue
            if keys[k1] in discarded[p]: continue
            if doublet: test1 = str(keys[k1]).zfill(5)
            else:       test1 = str(keys[k1]%100000).zfill(5)
            for k2 in range(k1+1,len(keys) ):
               if keys[k2] < 100000 and not doublet: continue
               if keys[k2] in discarded[p]: continue
               if doublet: test2 = str(keys[k2]).zfill(5)
               else:       test2 = str(keys[k2]%100000).zfill(5)
               ov = 0
               for i in range(5): 
                 if test1[i]==test2[i]: ov+=1
               if ov>ovMax:
                 if doublet and test1.find('0')<0 and not test2.find('0')<0: discarded[p].append(keys[k2])
                 elif doublet and test2.find('0')<0 and not test1.find('0')<0: discarded[p].append(keys[k1])
                 else:
                    rc1 = h['trackCand'][p][keys[k1]].Fit('pol1','QS')
                    rc2 = h['trackCand'][p][keys[k2]].Fit('pol1','QS')
                    if rc1.Get().Chi2() <  rc2.Get().Chi2(): discarded[p].append(keys[k2])
                    else:                                    discarded[p].append(keys[k1])
