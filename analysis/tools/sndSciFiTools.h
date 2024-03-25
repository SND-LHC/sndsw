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
    float peakScifiTiming(TClonesArray * digiHits, int bins, float min_x, float max_x); 

    // Function to get the hits in station n and plane X or Y
    // Station and orientation are defined with the same convention as the outputs of
    // sndScifiHit->GetStation() and sndScifiHits->isVertical()
    std::shared_ptr<TClonesArray> getScifiHits(TClonesArray * digiHits, int station, bool orientation);

    // Function to get the Scifi hits of station n and plane X or Y that are in the range
    // [peakTiming-range_lower; peakTiming+range_upper] of the peak of its time distribution
    // When only using "window_range", interval is symmetric around peakTiming (\pm window_range)
    // selection_parameters should have the form [bins_x, min_x, max_x, range_lower, range_upper]
    // (bins_x is converted to int before creating the histogram)
    // if len(selection_parameters) == 4 => range_lower = range upper = selection_parameters[3]
    // make_selection determines whether you want to select the hits for the respective plane
    // or not and skip that step (in case the hits are already curated)
    std::shared_ptr<TClonesArray> getScifiHits(TClonesArray * digiHits, int station, bool orientation, int bins_x, float min_x, float max_x, float range_lower, float range_upper, bool make_selection=true);
    std::shared_ptr<TClonesArray> getScifiHits(TClonesArray * digiHits, int station, bool orientation, int bins_x, float min_x, float max_x, float window_range, bool make_selection=true);
    std::shared_ptr<TClonesArray> getScifiHits(TClonesArray * digiHits, int station, bool orientation, std::vector<float> &selection_parameters, bool make_selection=true);

    // Function to obtain the ScifiHits that are considered to be useful from an event
    // selection_parameters is the number of bins for the histogram, min_x, max_x and
    // range of the selected hits in ns
    // Methods available are as follows:
    // (0) Events within \pm range of the peak of the time distribution for Scifi Hits within
    // each station and orientation
    std::shared_ptr<TClonesArray> filterScifiHits(TClonesArray * digiHits, std::vector<float> &selection_parameters, int method=0, std::string setup="TI18");

    // Function to determine SciFi hit density for channel reference_SiPM around a radius of r 
    // SiPM channels
    // If min_check == True the function stops checking the density once it reaches the minimum
    // requirement, and immediately returns -1 in case the minimum density cannot be achieved
    int densityScifi(int reference_SiPM, TClonesArray * digiHits, int radius, int min_hit_density, bool min_check);

    // Function to check efficiently wether a plane has a required hit density within a radius of r
    // SiPM channels
    bool densityCheck(TClonesArray * digiHits, int radius, int min_hit_density, bool orientation);

  }
}
