#pragma once

#include "TClonesArray.h"

namespace snd {
  namespace analysis_tools {

    // Function to get the number of hits per station in SciFi
    void getSciFiHitsPerStation(const TClonesArray * digiHits, std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits);

    // Function to get the total number of SciFi hits
    int getTotalSciFiHits(std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits);
    int getTotalSciFiHits(const TClonesArray * digiHits);

    // Function to get the SciFi fractional hits per plane
    std::vector<float> getFractionalHitsPerScifiPlane(std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits);
    std::vector<float> getFractionalHitsPerScifiPlane(const TClonesArray * digiHits);

    // Function to find the station where the interaction ocurred by checking the station at which the cumulative hit fraction exceeds a threshold
    int findScifiStation(std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits, float threshold);
    int findScifiStation(const TClonesArray * digiHits, float threshold);

    // Function to get the peak of the Scifi hit timing distribution for any given plane
    float peakScifiTiming(const TClonesArray &digiHits, int bins, float min_x, float max_x); 

    // Function to get the hits in station n and plane X or Y
    // Station and orientation are defined with the same convention as the outputs of
    // sndScifiHit->GetStation() and sndScifiHits->isVertical()
    std::unique_ptr<TClonesArray> getScifiHits(const TClonesArray &digiHits, int station, bool orientation);

    // Function to select the Scifi hits of station n and plane X or Y that are in the range
    // [peakTiming-range_lower; peakTiming+range_upper] of the peak of its time distribution
    // selection_parameters should have the form [bins_x, min_x, max_x, range_lower, range_upper]
    // (bins_x is converted to int before creating the histogram)
    // if len(selection_parameters) == 4 => range_lower = range upper = selection_parameters[3]
    // make_selection determines whether you want to select the hits for the respective plane
    // or not and skip that step (in case the hits are already curated)
    std::unique_ptr<TClonesArray> selectScifiHits(const TClonesArray &digiHits, int station, bool orientation, int bins_x=52, float min_x=0.0, float max_x=26.0, float range_lower=1E9/(2*160.316E6), float range_upper=2E9/(160.316E6), bool make_selection=true);
    std::unique_ptr<TClonesArray> selectScifiHits(const TClonesArray &digiHits, int station, bool orientation, const std::vector<float> &selection_parameters, bool make_selection=true);

    // Function to obtain the ScifiHits that are considered to be useful from an event
    // selection_parameters is the number of bins for the histogram, min_x, max_x and
    // range of the selected hits in ns
    // Methods available are as follows:
    // (0) Events within \pm range of the peak of the time distribution for Scifi Hits within
    // each station and orientation
    std::unique_ptr<TClonesArray> filterScifiHits(const TClonesArray &digiHits, const std::vector<float> &selection_parameters, int method=0, std::string setup="TI18");
    //Foregoing the selection_parameters option runs with the default values
    std::unique_ptr<TClonesArray> filterScifiHits(const TClonesArray &digiHits, int method=0, std::string setup="TI18");

    // Function to determine SciFi hit density for channel reference_SiPM around a radius of r 
    // SiPM channels
    // If min_check == True the function stops checking the density once it reaches the minimum
    // requirement, and immediately returns -1 in case the minimum density cannot be achieved
    int densityScifi(int reference_SiPM, const TClonesArray &digiHits, int radius, int min_hit_density, bool min_check);

    // Function to check efficiently wether a plane has a required hit density within a radius of r
    // SiPM channels
    bool densityCheck(const TClonesArray &digiHits, int radius=64, int min_hit_density=50, int station=1, bool orientation=false);

    // Function to get the starting block for a shower in the Scifi
    // Method 0 uses the Scifi Cluster density to determine whether a shower is propagating on a
    // given Scifi station or not, and declares the interaction block as being the previous one
    // to fulfill the desired condition
    int showerInteractionBlock(const TClonesArray &digiHits, const std::vector<float> &selection_parameters, int method=0, std::string setup="TI18");
    //Foregoing the selection_parameters option runs with the default values
    int showerInteractionBlock(const TClonesArray &digiHits, int method=0, std::string setup="TI18");

  }
}
