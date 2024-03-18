#pragma once

#include "TClonesArray.h"

namespace snd {
  namespace analysis_tools {

    // Function to get the number of hits per station in SciFi
    void getSciFiHitsPerStation(TClonesArray * digiHits, std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits);

    // Function to get the total number of SciFi hits
    int getTotalSciFiHits(std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits);
    int getTotalSciFiHits(TClonesArray * digiHits);

    // Function to get the SciFi fractional hits per plane
    std::vector<float> getFractionalHitsPerScifiPlane(std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits);
    std::vector<float> getFractionalHitsPerScifiPlane(TClonesArray * digiHits);

    // Function to find the station where the interaction ocurred by checking the station at which the cumulative hit fraction exceeds a threshold
    int findScifiStation(std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits, float threshold);
    int findScifiStation(TClonesArray * digiHits, float threshold);

    // Function to get the peak of the Scifi hit timing distribution for any given plane
    float peakScifiTiming(TClonesArray * digiHits, int bins, double minX, double maxX); 

    // Function to get the hits in station n and plane X or Y
    // Station and orientation are defined with the same convention as the outputs of
    // sndScifiHit->GetStation() and sndScifiHits->isVertical()
    TClonesArray getScifiHits(TClonesArray * digiHits, int station, bool orientation);

    // Function to get the Scifi hits of station n and plane X or Y that are in \pm range
    // of the peak of its time distribution
    // option determines whether you want to select the hits for the respective plane or not and
    // skip that step (in case the hits are already curated)
    TClonesArray getScifiHits(TClonesArray * digiHits, int station, bool orientation, float range, bool option=true);

    // Still being implemented
    // Function to obtain the ScifiHits that are considered to be usefull from an event
    // Input is the time range of the events in ns
    // Methods available are as follows:
    // (0) Events within \pm range of the peak of the time distribution for Scifi Hits within
    // each station and orientation
    TClonesArray filterScifiHits(TClonesArray * digiHits, int method=0, float range=1E9/(2*160.316E6), std::string setup="TI18");

    // Function to determine SciFi hit density for channel reference_SiPM around a radius of r 
    // SiPM channels
    // If min_check == True the function stops checking the density once it reaches the minimum
    // requirement, and immediately returns -1 in case the minimum density cannot be achieved
    int densityScifi(int reference_SiPM, TClonesArray * digiHits, int radius, int min_hit_density, bool min_check);

    // Function to check efficiently wether a plane has a required hit density within a radius of r
    // SiPM channels
    //int getElementN(std::set<int>&, int n);
    bool densityCheck(TClonesArray * digiHits, int radius, int min_hit_density, bool orientation);

  }
}
