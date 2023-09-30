#!/usr/bin/env python
'''Conversion tools from FEDRA to SNDSW'''

from xml.sax.handler import feature_external_ges
import ROOT as r
import fedrarootlogon
import numpy as np

#getting class instance
emureader = r.EmulsionDet()

def importgeofile(geofile):
    '''importing geometry file, can be replaced with sndsw config function if needed'''
    r.gGeoManager.Import(geofile)


def convertseg(seg,brickID,refplate = 60):
 '''convert position and angles from an EdbSegP into the global system
        Seg: EdbSegP instance;
        brickID: number of the brick the track was reconstructed in (ex. 31);
        returns a 6-values array (x,y,z,Vx,Vy,Vz)
        refplate: which is our reference plate
 '''
 detID = int(brickID * 1e3 + refplate)
 

 localarr = np.array([seg.X(), seg.Y(), seg.Z()])
 globalarr = np.zeros(3)

 emureader.GetPosition(detID,localarr,globalarr)

 anglocalarr = np.array([seg.TX(), seg.TY(), 1.])
 angglobalarr = np.zeros(3)

 emureader.GetAngles(detID,anglocalarr,angglobalarr)


 return np.concatenate([globalarr,angglobalarr])

def converttrack(track,brickID, refplate=60, fittedsegments = False):
 '''convert all segments from a volumetrack:
        track: EdbTrackP instance;
        brickID: number of the brick the track was reconstructed in (ex. 31);
        fittedsegments: used fitted segments instead of true segments (default False)
        refplate: which is our reference plate
 '''

 nseg = track.N()
 globaltracklist = []
 for iseg in range(nseg):
        myseg = track.GetSegment(iseg)
        if fittedsegments: #replace true segment with fittedsegment
            myseg = track.GetSegmentF(iseg)
        globalsegarr = convertseg(myseg,brickID,refplate)
        globaltracklist.append(globalsegarr)

 globaltrackarr = np.array(globaltracklist)
 return globaltrackarr

def convertvertex(vertex, brickID, refplate=60, fittedsegments=False):
 '''Converting EdbVertex position into SNDSW system
         vertex: EdbVertex instance;
         brickID: number of the brick the vertex was reconstructed in (ex. 31)
         fittedsegments: used fitted segments instead of true segments (default False)
         refplate: which is our reference plate
 '''
 detID = int(brickID * 1e3 + refplate)
 #vertex position
 localvarr = np.array([vertex.VX(),vertex.VY(),vertex.VZ()])
 globalvarr = np.zeros(3)
 emureader.GetPosition(detID,localvarr,globalvarr)
 #track info
 ntracks = vertex.N()
 globaltracklist = []
 for itrack in range(ntracks):
    track = vertex.GetTrack(itrack)
    globaltrackarr = converttrack(track,brickID,refplate,fittedsegments)
    globaltracklist.append(globaltrackarr)

 globaltracksarr = np.array(globaltracklist)
 return globalvarr, globaltracksarr

def convertmcpoint(emupoint, MCEventID, EmulsionID = 0, weight = 70):
  ''' Converting EmuPoint from MCEventID into EdbSegP'''

  detID = emupoint.GetDetectorID()
  momentum = r.TMath.Sqrt(pow(emupoint.GetPx(),2) + pow(emupoint.GetPy(),2) + pow(emupoint.GetPz(),2))
  #first, convert positions
  globalpos = np.array([emupoint.GetX(),emupoint.GetY(),emupoint.GetZ()])
  localpos = np.zeros(3)

  emureader.GetLocalPosition(detID, globalpos, localpos)

  xem = localpos[0]
  yem = localpos[1]

  #second, convert angles
  globalang = np.array([emupoint.GetPx(),emupoint.GetPy(),emupoint.GetPz()])
  localang = np.zeros(3)

  emureader.GetLocalAngles(detID, globalang, localang)
  #angles in TX, TY format
  tx = localang[0]/localang[2] #px/pz
  ty = localang[1]/localang[2] #py/pz
  #MCTruth info
  pdgcode = emupoint.PdgCode()
  trackID = emupoint.GetTrackID()

  #ready to write the segment
  emuseg = r.EdbSegP()
  emuseg.Set(EmulsionID, xem, yem, tx, ty, weight, 1)
  emuseg.SetMC(MCEventID, emupoint.GetTrackID())
  emuseg.SetP(momentum)
  emuseg.SetVid(pdgcode,0)

  emuseg.SetZ(localpos[2]) #0 is the center of the volume (center of plastic base in emulsion film)

  return emuseg
