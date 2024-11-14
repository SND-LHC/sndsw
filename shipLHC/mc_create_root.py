#!/usr/bin/env python
import ROOT
import numpy as np
import os

ROOT.gROOT.SetBatch(True)

# Define the fixed z coordinate where we will extract x, y positions
z_plane = 250.0  # adjust as needed

# Set up the TChain for the ROOT files
track_chain = ROOT.TChain("rawConv")  # Ensure the TTree name matches

# Directory containing ROOT files
root_files_dir = "/eos/experiment/sndlhc/users/sii/4705/"
file_pattern = "sndsw_raw-{}_{}_4705_muonReco.root"

# Adding all ROOT files to the TChain
for part in range(10, 15):
    for it in range(10):
        file_path = os.path.join(root_files_dir, file_pattern.format(it, part))
        track_chain.Add(file_path)

# Prepare output ROOT file and TTree
output_file = ROOT.TFile("extracted_muon_tracks.root", "RECREATE")
ntuple = ROOT.TNtuple("muon_tracks", "Extrapolated Muon Tracks",
                      "event:x:y:slopes_xz:slopes_yz")

# Loop over each event and track
for i_event, event in enumerate(track_chain):
    for track in event.Reco_MuonTracks:
        if track.getTrackType() != 11 or track.getTrackFlag() != 1:
            continue  # Skip non-Scifi tracks or bad quality tracks

        # Extrapolate position and calculate slopes
        pos = track.getStart()
        mom = track.getTrackMom()

        # Calculate lamda for z-plane intersection
        lam = (z_plane - pos.z()) / mom.z()
        extrap_x = pos.x() + lam * mom.x()
        extrap_y = pos.y() + lam * mom.y()

        # Slopes
        slope_xz = mom.x() / mom.z()
        slope_yz = mom.y() / mom.z()

        # Store in the ntuple
        ntuple.Fill(i_event, extrap_x, extrap_y, slope_xz, slope_yz)

# Write to the output file and close it
output_file.Write()
output_file.Close()
print("Extraction completed. Results are saved in 'extracted_muon_tracks.root'.")
