'''small script to add information to vertextree as a friend TNtuple'''

import ROOT as r
import fedrarootlogon
import sys

brickID = int(sys.argv[1])

vtxfile = r.TFile.Open("vertextree.root","UPDATE")
vtxtree = vtxfile.Get("vtx")

outputtree = r.TNtuple("brickinfo","Information about the brick","vID:brickID")
#main loop, filling additional information
for vtx in vtxtree:
    outputtree.Fill(vtx.vID, brickID)

vtxfile.cd()
outputtree.Write("",r.TObject.kOverwrite)
