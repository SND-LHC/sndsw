#!/usr/bin/env python
import os
import sys
import ROOT
import numpy as np

import shipunit as u
import shipRoot_conf
import rootUtils as ut
from ShipGeoConfig import ConfigRegistry
from argparse import ArgumentParser

mcEngine     = "TGeant4"
simEngine    = "Pythia8"  # "Genie" # Ntuple
inactivateMuonProcesses  = False

MCTracksWithHitsOnly                  = False  # copy particles which produced a hit and their history
MCTracksWithEnergyCutOnly    = True # copy particles above a certain kin energy cut
MCTracksWithHitsOrEnergyCut = False # or of above, factor 2 file size increase compared to MCTracksWithEnergyCutOnly

parser = ArgumentParser()

parser.add_argument("--H6",   dest="testbeam",   help="use geometry of H8/H6 testbeam setup", action="store_true")
parser.add_argument("--HX",   dest="testbeam2023",   help="use geometry of 2023 testbeam setup", action="store_true")
parser.add_argument("--H4",   dest="testbeam2024",   help="use geometry of 2024 testbeam setup", action="store_true")
parser.add_argument("--target", help="target material for the 2024 testbeam setup", choices=["W","Fe"], default="W", type = str)
parser.add_argument("--Genie",   dest="genie",   help="Genie for reading and processing neutrino interactions (1 standard, 2 FLUKA, 3 Pythia, 4 GENIE geometry driver)", required=False, default = 0, type = int)
parser.add_argument("--Ntuple",  dest="ntuple",  help="Use ntuple as input", required=False, action="store_true")
parser.add_argument("--MuonBack",dest="muonback",  help="Generate events from muon background file, --Cosmics=0 for cosmic generator data", required=False, action="store_true")
parser.add_argument("--Pythia8", dest="pythia8", help="Use Pythia8", required=False, action="store_true")
parser.add_argument("--PG",      dest="pg",      help="Use Particle Gun", required=False, action="store_true")
parser.add_argument("--pID",     dest="pID",     help="id of particle used by the gun (default=22)", required=False, default=22, type=int)
parser.add_argument("--Estart",  dest="Estart",  help="start of energy range of particle gun for muflux detector (default=10 GeV)", required=False, default=10, type=float)
parser.add_argument("--Eend",    dest="Eend",    help="end of energy range of particle gun for muflux detector (default=10 GeV)", required=False, default=10, type=float)

parser.add_argument("--multiplePGSources", help="Multiple particle guns in a x-y plane at a fixed z or in a 3D volume", action="store_true")
parser.add_argument("--EVx", dest="EVx", help="particle gun start xpos", required=False, default=0, type=float)
parser.add_argument("--EVy", dest="EVy", help="particle gun start ypos", required=False, default=0, type=float)
parser.add_argument("--EVz", dest="EVz", help="particle gun start zpos", required=False, default=0, type=float)
parser.add_argument("--Dx", help="size of the full uniform spread of PG xpos", type=float)
parser.add_argument("--Dy", help="size of the full uniform spread of PG ypos", type=float)
parser.add_argument("--nZSlices", help="number of z slices for the PG sources", type=int)
parser.add_argument("--zSliceStep", help="distance between the z slices for the PG sources", type=float)

parser.add_argument("--FollowMuon",dest="followMuon", help="Make muonshield active to follow muons", required=False, action="store_true")
parser.add_argument("--FastMuon",  dest="fastMuon",  help="Only transport muons for a fast muon only background estimate", required=False, action="store_true")
parser.add_argument('--eMin', type=float, help="energy cut", dest='ecut', default=-1.)
parser.add_argument('--zMax', type=float, help="max distance to apply energy cut", dest='zmax', default=70000.)
parser.add_argument("--Nuage",     dest="nuage",  help="Use Nuage, neutrino generator of OPERA", required=False, action="store_true")
parser.add_argument("--MuDIS",     dest="mudis",  help="Use muon deep inelastic scattering generator", required=False, action="store_true")
parser.add_argument("-n", "--nEvents",dest="nEvents",  help="Number of events to generate", required=False,  default=100, type=int)
parser.add_argument("-i", "--firstEvent",dest="firstEvent",  help="First event of input file to use", required=False,  default=0, type=int)
parser.add_argument("-s", "--seed",dest="theSeed",  help="Seed for random number. Only for experts, see TRrandom::SetSeed documentation", required=False,  default=0, type=int)
parser.add_argument("-f",        dest="inputFile",       help="Input file if not default file", required=False, default=False)
parser.add_argument("-g",        dest="geofile",       help="geofile for muon shield geometry, for experts only", required=False, default=None)
parser.add_argument("-o", "--output",dest="outputDir",  help="Output directory", required=False,  default=".")
parser.add_argument("--boostFactor", dest="boostFactor",  help="boost mu brems", required=False, type=float,default=0)
parser.add_argument("--enhancePiKaDecay", dest="enhancePiKaDecay",  help="decrease charged pion and kaon lifetime", required=False, type=float,default=0.)
parser.add_argument("--debug",   dest="debug",   help="debugging mode, check for overlaps", required=False, action="store_true")
parser.add_argument("-D", "--display", dest="eventDisplay", help="store trajectories", required=False, action="store_true")
parser.add_argument("--EmuDet","--nuTargetActive",dest="nuTargetPassive",help="activate emulsiondetector", required=False,action="store_false")
parser.add_argument("--NagoyaEmu","--useNagoyaEmulsions",dest="useNagoyaEmulsions",help="use bricks of 57 Nagoya emulsion films instead of 60 Slavich", required=False,action="store_true")
parser.add_argument("-y", dest="year", help="specify the year to generate the respective TI18 detector setup", required=False, type=int, default=2024)

options = parser.parse_args()

# user hook
userTask = False

class MyTask(ROOT.FairTask):
    "user task"

    def Exec(self,opt):
        ioman = ROOT.FairRootManager.Instance()
        MCTracks = ioman.GetObject("MCTrack")
        print('Hello',opt,MCTracks.GetEntries())
        fMC = ROOT.TVirtualMC.GetMC()
        if MCTracks.GetEntries()>100:  fMC.StopRun()

checking4overlaps = False
if options.debug: checking4overlaps = True

if options.pythia8:       simEngine = "Pythia8"
if options.pg:                 simEngine = "PG"
if options.genie:           simEngine = "Genie"
if options.ntuple:         simEngine = "Ntuple"
if options.muonback: simEngine = "MuonBack"
if options.nuage:          simEngine = "Nuage"
if options.mudis:          simEngine = "muonDIS"

if options.inputFile:
  if options.inputFile == "none": options.inputFile = None
  inputFile = options.inputFile
  defaultInputFile = False

if   simEngine == "Genie"  and defaultInputFile: 
   print('GENIE input file missing, exit')
   sys.exit()
if simEngine == "muonDIS" and defaultInputFile:
   print('input file required if simEngine = muonDIS. Example:')
   print("/eos/experiment/sndlhc/MonteCarlo/Pythia6/MuonDIS /muonDis_XXXX.root")
   print("  XXXX = run+cycle*100+k, k: 0 or 1000 for mu+ or mu-,  c: number of cycles: 10 events per incoming muon in each cycle, run: 1...10")
   print(" c = 0 - 2: mu->proton,  5 - 7:  mu->neutron")
   sys.exit()
if simEngine == "Nuage" and not inputFile:
   inputFile = 'Numucc.root'

if (simEngine == "Ntuple") and defaultInputFile :
  print('input file required if simEngine = Ntuple or MuonBack. Examples:')
  print ("crossing angle up:        /eos/experiment/sndlhc/MonteCarlo/FLUKA/muons_up/version1/unit30_Nm.root  (unit30_Pm.root)")
  print ("crossing angle down: /eos/experiment/sndlhc/MonteCarlo/FLUKA/muons_down/muons_VCdown_IR1-LHC.root")
  sys.exit()

print("SND@LHC setup for",simEngine,"to produce",options.nEvents,"events")

ROOT.gRandom.SetSeed(options.theSeed)  # this should be propagated via ROOT to Pythia8 and Geant4VMC
shipRoot_conf.configure(0)     # load basic libraries, prepare atexit for python

if options.testbeam:
  snd_geo = ConfigRegistry.loadpy("$SNDSW_ROOT/geometry/sndLHC_H6geom_config.py")
elif options.testbeam2023:
  snd_geo = ConfigRegistry.loadpy("$SNDSW_ROOT/geometry/sndLHC_HXgeom_config.py",
                                   tb_2023_mc = options.testbeam2023)
elif options.testbeam2024:
  snd_geo = ConfigRegistry.loadpy("$SNDSW_ROOT/geometry/sndLHC_H4geom_config.py",
                                   tb_2024_mc = options.testbeam2024,
                                   target_material = options.target)
else:
  snd_geo = ConfigRegistry.loadpy("$SNDSW_ROOT/geometry/sndLHC_TI18geom_config.py",
                                   nuTargetPassive = options.nuTargetPassive,
                                   useNagoyaEmulsions = options.useNagoyaEmulsions,
                                   year=options.year)

if simEngine == "PG": tag = simEngine + "_"+str(options.pID)+"-"+mcEngine
else: tag = simEngine+"-"+mcEngine

if not os.path.exists(options.outputDir):
  os.makedirs(options.outputDir)
if options.boostFactor>1:
   tag+='_boost'+str(options.boostFactor)
outFile = "%s/sndLHC.%s.root" % (options.outputDir, tag)

# rm older files !!! 
for x in os.listdir(options.outputDir):
  if not x.find(tag)<0: os.system("rm %s/%s" % (options.outputDir, x) )
# Parameter file name
parFile="%s/ship.params.%s.root" % (options.outputDir, tag)

# In general, the following parts need not be touched, except for user task
# ========================================================================

# -----Timer--------------------------------------------------------
timer = ROOT.TStopwatch()
timer.Start()
# ------------------------------------------------------------------------
# -----Create simulation run----------------------------------------
run = ROOT.FairRunSim()
run.SetName(mcEngine)  # Transport engine
run.SetSink(ROOT.FairRootFileSink(outFile))  # Output file
run.SetUserConfig("g4Config.C") # user configuration file default g4Config.C 
rtdb = run.GetRuntimeDb() 
# add user task
if userTask:
  userTask   = MyTask()
  run.AddTask(userTask)

# -----Create geometry----------------------------------------------
import shipLHC_conf as sndDet_conf
modules = sndDet_conf.configure(run,snd_geo)

# -----Create PrimaryGenerator--------------------------------------
primGen = ROOT.FairPrimaryGenerator()

# -----Particle Gun-----------------------
if simEngine == "PG": 
  myPgun = ROOT.FairBoxGenerator(options.pID,1)
  myPgun.SetPRange(options.Estart,options.Eend)
  myPgun.SetPhiRange(0, 360) # // Azimuth angle range [degree]
  myPgun.SetThetaRange(0,0) # // Polar angle in lab system range [degree]
  if options.multiplePGSources:
    # multiple PG sources in the x-y plane; z is always the same!
    myPgun.SetBoxXYZ(options.EVx*u.cm,
                     options.EVy*u.cm,
                     (options.EVx+options.Dx)*u.cm,
                     (options.EVy+options.Dy)*u.cm,
                     options.EVz*u.cm)
  else:
     # point source
     myPgun.SetXYZ(options.EVx*u.cm, options.EVy*u.cm, options.EVz*u.cm)
  primGen.AddGenerator(myPgun)
  # To generate particle guns along the z axis, create z *target* layers with a set step
  # For an **unknown** reason simply setting target z thickness doesn't produce the expected result
  if options.multiplePGSources:
    targetZpos = np.array(np.arange(options.nZSlices)*options.zSliceStep*u.cm, dtype='d')
    primGen.SetMultTarget(len(targetZpos), targetZpos, 0*u.cm) # dummy thickness set to 0
  print('===> Setting particle gun sources starting at (x1,y1,z1)='
        f'({options.EVx},{options.EVy},{options.EVz})[cm × cm × cm] \n'
        f'with a uniform x-y spread of (Dx,Dy)=({options.Dx},{options.Dy})[cm × cm]'
        f' and {options.nZSlices} z slices in steps of {options.zSliceStep}[cm].')
  run.SetPythiaDecayer('DecayConfigPy8.C')
  ROOT.FairLogger.GetLogger().SetLogScreenLevel("WARNING") # otherwise stupid printout for each event
# -----muon DIS Background------------------------
if simEngine == "muonDIS":
   ut.checkFileExists(inputFile)
   primGen.SetTarget(0., 0.) 
   DISgen = ROOT.MuDISGenerator()
   mu_start, mu_end = (-3.7-2.0)*u.m , -0.3*u.m # tunnel wall -30cm in front of SND
   DISgen.SetPositions(0, mu_start, mu_end)
   DISgen.Init(inputFile,options.firstEvent) 
   primGen.AddGenerator(DISgen)
   options.nEvents = min(options.nEvents,DISgen.GetNevents())
   inactivateMuonProcesses = True # avoid unwanted hadronic events of "incoming" muon flying backward
   run.SetPythiaDecayer('DecayConfigPy8.C')
   print('MuDIS position info input=',mu_start, mu_end)
   print('Generate ',options.nEvents,' with DIS input', ' first event',options.firstEvent)

# -----neutrino interactions from nuage------------------------
if simEngine == "Nuage":
   primGen.SetTarget(0., 0.)
   Nuagegen = ROOT.NuageGenerator()
   Nuagegen.EnableExternalDecayer(1) #with 0 external decayer is disable, 1 is enabled
   Nuagegen.SetPositions(0., -snd_geo.EmulsionDet.zdim/2, snd_geo.EmulsionDet.zdim/2, -snd_geo.EmulsionDet.xdim/2, snd_geo.EmulsionDet.xdim/2, -snd_geo.EmulsionDet.ydim/2, snd_geo.EmulsionDet.ydim/2)
   ut.checkFileExists(inputFile)
   Nuagegen.Init(inputFile,options.firstEvent)
   primGen.AddGenerator(Nuagegen)
   options.nEvents = min(options.nEvents,Nuagegen.GetNevents())
   run.SetPythiaDecayer("DecayConfigNuAge.C")
   print('Generate ',options.nEvents,' with Nuage input', ' first event',options.firstEvent)

# -----neutrino interactions from GENIE------------------------
if simEngine=="Genie":
   ut.checkFileExists(inputFile)
   primGen.SetTarget(0., 0.) # do not interfere with GenieGenerator
   Geniegen = ROOT.GenieGenerator()
   Geniegen.SetGenerationOption(options.genie - 1) # 0 standard, 1 FLUKA,2 Pythia
   Geniegen.Init(inputFile,options.firstEvent)
   Geniegen.SetCrossingAngle(150e-6) #used only in option 2

   # Neutrino vertex generation range in z:
   # Tolerance for neutrino vertex generation range. Mostly to account for tilt in geometry alignment. Take difference in z coordinate of vertical fibres of around 0.5 cm over the fibre length, 39 cm. Assume maximum difference in z is 1 m * 0.5/39.
   tolerance_vtx_z = 1*u.m * 0.5/39
   # From first veto bar
   neutrino_vtx_start_z = snd_geo.MuFilter.Veto1Dy - snd_geo.MuFilter.VetoBarZ/2. - tolerance_vtx_z
   # To last Scifi plane
   neutrino_vtx_end_z = snd_geo.Scifi.Ypos4 + snd_geo.Scifi.zdim/2. + tolerance_vtx_z

   Geniegen.SetPositions(-480*u.m, neutrino_vtx_start_z, neutrino_vtx_end_z)

   Geniegen.SetDeltaE_Matching_FLUKAGenie(10.) #energy range for the search of a GENIE interaction with similar energy of FLUKA neutrino
   primGen.AddGenerator(Geniegen)
   options.nEvents = min(options.nEvents,Geniegen.GetNevents())
   run.SetPythiaDecayer('DecayConfigPy8.C')
   print('Generate ',options.nEvents,' with Genie input for Ship@LHC', ' first event',options.firstEvent)

if simEngine == "Ntuple":
   ut.checkFileExists(inputFile)
   Ntuplegen = ROOT.NtupleGenerator_FLUKA()
   Ntuplegen.SetZ(snd_geo.Floor.z)
   Ntuplegen.Init(inputFile,options.firstEvent)
   primGen.AddGenerator(Ntuplegen)
   options.nEvents = min(options.nEvents,Ntuplegen.GetNevents())
   run.SetPythiaDecayer('DecayConfigPy8.C')

if simEngine == "MuonBack":
# reading muon tracks from FLUKA
 fileType = ut.checkFileExists(inputFile)
 if fileType == 'tree':
 # 2018 background production 
  primGen.SetTarget(snd_geo.target.z0+70.845*u.m,0.)
 else:
  primGen.SetTarget(snd_geo.target.z0+50*u.m,0.)
 #
 MuonBackgen = ROOT.MuonBackGenerator()
 # MuonBackgen.FollowAllParticles() # will follow all particles after hadron absorber, not only muons
 MuonBackgen.Init(inputFile,options.firstEvent,options.phiRandom)
 primGen.AddGenerator(MuonBackgen)
 options.nEvents = min(options.nEvents,MuonBackgen.GetNevents())
 MCTracksWithHitsOnly = True # otherwise, output file becomes too big
 print('Process ',options.nEvents,' from input file, with Phi random=',options.phiRandom, ' with MCTracksWithHitsOnly',MCTracksWithHitsOnly)

if options.ecut > 0:   
   modules['Floor'].SetEmin(options.ecut)
   modules['Floor'].SetZmax(options.zmax)

#
run.SetGenerator(primGen)
# ------------------------------------------------------------------------
if options.followMuon :  
    if 'Veto' in modules:
        options.fastMuon = True
        modules['Veto'].SetFollowMuon()
    if 'Floor' in modules:
        modules['Floor'].MakeSensitive()
        print('make floor sensitive')
if options.fastMuon :
     if 'Veto' in modules:       modules['Veto'].SetFastMuon()
     elif 'Floor' in modules: 
           modules['Floor'].SetFastMuon()
           modules['Floor'].SetZmax(options.zmax)
           print('transport only-muons up to z=',options.zmax)
# ------------------------------------------------------------------------
#---Store the visualiztion info of the tracks, this make the output file very large!!
#--- Use it only to display but not for production!
if options.eventDisplay: run.SetStoreTraj(ROOT.kTRUE)
else:            run.SetStoreTraj(ROOT.kFALSE)



# -----Initialize simulation run------------------------------------
run.Init()
gMC = ROOT.TVirtualMC.GetMC()
fStack = gMC.GetStack()
if MCTracksWithHitsOnly:
 fStack.SetMinPoints(1)
 fStack.SetEnergyCut(-100.*u.MeV)
elif MCTracksWithEnergyCutOnly:
 fStack.SetMinPoints(-1)
 fStack.SetEnergyCut(100.*u.MeV)
elif MCTracksWithHitsOrEnergyCut: 
 fStack.SetMinPoints(1)
 fStack.SetEnergyCut(100.*u.MeV)
elif options.deepCopy: 
 fStack.SetMinPoints(0)
 fStack.SetEnergyCut(0.*u.MeV)

#
if options.boostFactor > 1:
 ROOT.gROOT.ProcessLine('#include "Geant4/G4ProcessTable.hh"')
 ROOT.gROOT.ProcessLine('#include "Geant4/G4MuBremsstrahlung.hh"')
 ROOT.gROOT.ProcessLine('#include "Geant4/G4GammaConversionToMuons.hh"')
 ROOT.gROOT.ProcessLine('#include "Geant4/G4MuPairProduction.hh"')
 ROOT.gROOT.ProcessLine('#include "Geant4/G4AnnihiToMuPair.hh"')
 ROOT.gROOT.ProcessLine('#include "Geant4/G4MuonToMuonPairProduction.hh"')
 ROOT.gROOT.ProcessLine('#include "Geant4/G4MuonPlus.hh"')
 ROOT.gROOT.ProcessLine('#include "Geant4/G4MuonMinus.hh"')

 gProcessTable = ROOT.G4ProcessTable.GetProcessTable()
 # only muon interaction
 # procBrems        = gProcessTable.FindProcess(ROOT.G4String('muBrems'),ROOT.G4String('mu+'))
 # muPairProd = gProcessTable.FindProcess(ROOT.G4String('muPairProd'),ROOT.G4String('mu+'))
 # muPairProd.SetCrossSectionBiasingFactor(options.boostFactor)
 # procBrems.SetCrossSectionBiasingFactor(options.boostFactor)
 # muon pair production
 gammaToMuPair = gProcessTable.FindProcess(ROOT.G4String('GammaToMuPair'),ROOT.G4String('gamma'))
 gammaToMuPair.SetCrossSecFactor(options.boostFactor) 
 AnnihiToMuPair = gProcessTable.FindProcess(ROOT.G4String('AnnihiToMuPair'),ROOT.G4String('e+'))
 AnnihiToMuPair.SetCrossSecFactor(options.boostFactor)
 MuonToMuonPair = gProcessTable.FindProcess(ROOT.G4String('muToMuonPairProd'),ROOT.G4String('mu+'))
 MuonToMuonPair.SetCrossSectionBiasingFactor(options.boostFactor)

 mygMC = ROOT.TGeant4.GetMC()
 if options.debug:
   mygMC.ProcessGeantCommand("/run/particle/dumpOrderingParam")
   mygMC.ProcessGeantCommand("/particle/select mu+")
   mygMC.ProcessGeantCommand("/particle/process/dump")
   mygMC.ProcessGeantCommand("/particle/select gamma")
   mygMC.ProcessGeantCommand("/particle/process/dump")
   mygMC.ProcessGeantCommand("/particle/select e+")
   mygMC.ProcessGeantCommand("/particle/process/dump")
#
if options.enhancePiKaDecay:
  ROOT.gROOT.ProcessLine('#include "Geant4/G4ParticleTable.hh"')
  ROOT.gROOT.ProcessLine('#include "Geant4/G4DecayTable.hh"')
  ROOT.gROOT.ProcessLine('#include "Geant4/G4PhaseSpaceDecayChannel.hh"')
  pt = ROOT.G4ParticleTable.GetParticleTable()
  for pid in [211,-211,321,-321]:
      particleG4  = pt.FindParticle(pid)
      lt = particleG4.GetPDGLifeTime()
      particleG4.SetPDGLifeTime(lt/options.enhancePiKaDecay)
  print('### pion kaon lifetime decreased by the factor:',options.enhancePiKaDecay)

if inactivateMuonProcesses :
 ROOT.gROOT.ProcessLine('#include "Geant4/G4ProcessTable.hh"')
 mygMC = ROOT.TGeant4.GetMC()
 mygMC.ProcessGeantCommand("/process/inactivate muPairProd")
 mygMC.ProcessGeantCommand("/process/inactivate muBrems")
 #mygMC.ProcessGeantCommand("/process/inactivate muIoni")  Temporary fix for DIS Simulations (incoming and outgoing muon hits)
 mygMC.ProcessGeantCommand("/process/inactivate muonNuclear")
 mygMC.ProcessGeantCommand("/particle/select mu+")
 mygMC.ProcessGeantCommand("/particle/process/dump")
 gProcessTable = ROOT.G4ProcessTable.GetProcessTable()
 procmu = gProcessTable.FindProcess(ROOT.G4String('muIoni'),ROOT.G4String('mu+'))
 procmu.SetVerboseLevel(2)

if options.debug:  ROOT.fair.Logger.SetConsoleSeverity("debug")
# -----Start run----------------------------------------------------
run.Run(options.nEvents)
# -----Runtime database---------------------------------------------
kParameterMerged = ROOT.kTRUE
parOut = ROOT.FairParRootFileIo(kParameterMerged)
parOut.open(parFile)
rtdb.setOutput(parOut)
rtdb.saveOutput()
rtdb.printParamContexts()
getattr(rtdb,"print")()
# ------------------------------------------------------------------------
geoFile = "%s/geofile_full.%s.root" % (options.outputDir, tag)
run.CreateGeometryFile(geoFile)
# save detector parameters dictionary in geofile
import saveBasicParameters
saveBasicParameters.execute(geoFile,snd_geo)

# ------------------------------------------------------------------------
# If using GENIE option 4 (geometry driver) copy GST TTree to the 
# output file. This will make it easy to access the FLUKA variables for
# each neutrino event.
if options.genie == 4 :

    f_input = ROOT.TFile(inputFile)
    gst = f_input.gst

    selection_string = "(Entry$ >= "+str(options.firstEvent)+")"
    if (options.firstEvent + options.nEvents) < gst.GetEntries() :
        selection_string += "&&(Entry$ < "+str(options.firstEvent + options.nEvents)+")"
    
    # Reopen output file
    f_output = ROOT.TFile(outFile, "UPDATE")

    # Copy only the events used in this run
    gst_copy = gst.CopyTree(selection_string)
    gst_copy.Write()

    f_input.Close()
    f_output.Close()

# -----Finish-------------------------------------------------------
timer.Stop()
rtime = timer.RealTime()
ctime = timer.CpuTime()
print(' ') 
print("Macro finished succesfully.") 

print("Output file is ",  outFile) 
print("Geometry file is ",geoFile)
print("Real time ",rtime, " s, CPU time ",ctime,"s")

# ------------------------------------------------------------------------
def checkOverlaps(removeScifi=False):
 sGeo = ROOT.gGeoManager
 if removeScifi:
    for n in range(1,6):
       Hscifi = sGeo.FindVolumeFast('ScifiVolume'+str(n))
       removalList = []
       for x in Hscifi.GetNodes():
             if x.GetName().find('Scifi')==0: removalList.append(x)
       for x in removalList: Hscifi.RemoveNode(x)
 sGeo.SetNmeshPoints(10000)
 sGeo.CheckOverlaps(0.1)  # 1 micron takes 5minutes
 sGeo.PrintOverlaps()
# check subsystems in more detail
 for x in sGeo.GetTopNode().GetNodes(): 
   x.CheckOverlaps(0.0001)
   sGeo.PrintOverlaps()

def checkOverlapsWithGeant4():
 # after /run/initialize, but prints warning messages, problems with TGeo volume
 mygMC = ROOT.TGeant4.GetMC()
 mygMC.ProcessGeantCommand("/geometry/test/recursion_start 0")
 mygMC.ProcessGeantCommand("/geometry/test/recursion_depth 2")
 mygMC.ProcessGeantCommand("/geometry/test/run")

# checking for overlaps
if checking4overlaps:
      checkOverlaps()
