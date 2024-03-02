#!/usr/bin/env python
import ROOT
import shipRoot_conf
from ShipGeoConfig import ConfigRegistry
from argparse import ArgumentParser

mcEngine     = "TGeant4"

parser = ArgumentParser()

parser.add_argument("-c",   dest="config",   help="configuration file", required=True)
parser.add_argument("-g",   dest="geofile",   help="geo file output name", required=True)
parser.add_argument("-y",   dest="year",   help="specify the year to generate the respective TI18 detector setup", required=True)
options = parser.parse_args()

shipRoot_conf.configure(0)     # load basic libraries, prepare atexit for python

snd_geo = ConfigRegistry.loadpy(options.config, year=options.year)

# -----Create simulation run----------------------------------------
run = ROOT.FairRunSim()
run.SetName(mcEngine)  # Transport engine
run.SetSink(ROOT.FairRootFileSink(ROOT.TMemFile('output', 'recreate')))  # output file
run.SetUserConfig("g4Config.C") # user configuration file default g4Config.C 
rtdb = run.GetRuntimeDb() 

# -----Create geometry----------------------------------------------
import shipLHC_conf as sndDet_conf
modules = sndDet_conf.configure(run,snd_geo)

# -----Create PrimaryGenerator--------------------------------------
primGen = ROOT.FairPrimaryGenerator()

# -----Particle Gun-----------------------
myPgun = ROOT.FairBoxGenerator(13,1)
primGen.AddGenerator(myPgun)

run.SetGenerator(primGen)
# -----Initialize simulation run------------------------------------
run.Init()

# -----Start run----------------------------------------------------
# run.Run(1)
# ------------------------------------------------------------------------
run.CreateGeometryFile(options.geofile)
# save detector parameters dictionary in geofile
import saveBasicParameters
saveBasicParameters.execute(options.geofile,snd_geo)
