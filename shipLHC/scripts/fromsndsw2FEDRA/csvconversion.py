import fedra2scipy_utils
import sys
import pandas as pd
import ROOT as r

def applyconversion(nbrick, simfile, simbkgfile):
 '''convert couples ROOT files into a csv'''

 df = fedra2scipy_utils.builddataframe(nbrick)
 df = fedra2scipy_utils.addtrueMCinfo(df,simfile, simbkgfile)
 df = fedra2scipy_utils.addtrackindex(df,"linked_tracks.root")
 dfvertices = fedra2scipy_utils.addvertexindex(df,"vertextree.root")

 return df, dfvertices 

#the two steps can now be done together, without an intermediate file
nbrick = int(sys.argv[1])
simfile = r.TFile.Open(sys.argv[2]) #neutrino signal simulation
simbkgfile = r.TFile.Open(sys.argv[3]) #muon background simulation
#starting all conversion steps
df,dfvertices = applyconversion(nbrick, simfile, simbkgfile)

#df = df.drop(columns = ["P","Flag"])
df.to_csv('brick{}.csv'.format(nbrick),index=False)
dfvertices.to_csv('brick{}_vertices.csv'.format(nbrick),index=False)


