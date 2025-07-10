#!/usr/bin/env python
import ROOT
import argparse

ROOT.gROOT.SetBatch(True)

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Extract muon track data at a specific z-plane.")
parser.add_argument("--zplane", type=float, required=True,
                    help="Fixed z-plane value for extrapolation.")
parser.add_argument("--inputfile", type=str, required=True,
                    help="Input ROOT file containing muon tracks.")
parser.add_argument("--outputfile", type=str, default="extracted_muon_tracks.root",
                    help="Output ROOT file to save the extracted data.")
args = parser.parse_args()

# Get the z-plane value and file paths from user input
z_plane = args.zplane
input_file = args.inputfile
output_file_name = args.outputfile

# Set up the TChain for the ROOT file
track_chain = ROOT.TChain("rawConv")  # Ensure the TTree name matches
track_chain.Add(input_file)  # Add the specified input file to the TChain

# Prepare output ROOT file and TTree
output_file = ROOT.TFile(output_file_name, "RECREATE")
ntuple = ROOT.TNtuple("muon_tracks", "Extrapolated Muon Tracks",
                      "event:x:y:z:theta:phi")

# Loop over each event and track
for i_event, event in enumerate(track_chain):
    for track in event.Reco_MuonTracks:
        if track.getTrackType() != 11 or track.getTrackFlag() != 1:
            continue  # Skip non-Scifi tracks or bad quality tracks

        # Extract position and momentum
        pos = track.getStart()  # pos.x(), pos.y(), pos.z()
        mom = track.getTrackMom()  # mom.x(), mom.y(), mom.z()

        # Calculate lambda for z-plane intersection
        lam = (z_plane - pos.z()) / mom.z()
        extrap_x = pos.x() + lam * mom.x()
        extrap_y = pos.y() + lam * mom.y()

        # Calculate theta and phi angles
        theta = ROOT.TMath.ATan(ROOT.TMath.Sqrt((mom.x() ** 2 + mom.y() ** 2) / mom.z() ** 2))
        phi = ROOT.TMath.ATan2(mom.y(), mom.x())

        # Store in the ntuple
        ntuple.Fill(i_event, extrap_x, extrap_y, z_plane, theta, phi)

# Write to the output file and close it
output_file.Write()
output_file.Close()
print(f"Extraction completed. Results are saved in '{output_file_name}'. Used z-plane: {z_plane}")
