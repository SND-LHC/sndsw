#pragma once
class Scifi;

#include <map>
#include "TClonesArray.h"
#include "ShipUnit.h"

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
    float peakScifiTiming(const TClonesArray &digiHits, int bins, float min_x, float max_x, bool isMC=false);

    // Function to get the hits in station n and plane X or Y
    // Station and orientation are defined with the same convention as the outputs of
    // sndScifiHit->GetStation() and sndScifiHits->isVertical()
    std::unique_ptr<TClonesArray> getScifiHits(const TClonesArray &digiHits, int station, bool orientation);

    // Function to select the Scifi hits of station n and plane X or Y that are in the range
    // [peakTiming-range_lower; peakTiming+range_upper] of the peak of its time distribution
    // selection_parameters should have the form [bins_x, min_x, max_x, time_lower_range,
    // time_upper_range ] (bins_x is converted to int before creating the histogram)
    // if len(selection_parameters) == 4 => time_lower_range = time_upper_range =
    // selection_parameters[3]
    // make_selection determines whether you want to select the hits for the respective plane
    // or not and skip that step (in case the hits are already curated)
    std::unique_ptr<TClonesArray> selectScifiHits(const TClonesArray &digiHits, int station, bool orientation, int bins_x=52, float min_x=0.0, float max_x=26.0, float time_lower_range=1E9/(2*ShipUnit::snd_freq/ShipUnit::hertz), float time_upper_range=1.2E9/(ShipUnit::snd_freq/ShipUnit::hertz), bool make_selection=true, bool isMC=false);
    std::unique_ptr<TClonesArray> selectScifiHits(const TClonesArray &digiHits, int station, bool orientation, const std::map<std::string, float> &selection_parameters, bool make_selection=true, bool isMC=false);

    // Function to obtain the ScifiHits that are considered to be useful from an event
    // selection_parameters is the number of bins for the histogram, min_x, max_x and
    // range of the selected hits in ns
    // Methods available are as follows:
    // (0) Events within \pm range of the peak of the time distribution for Scifi Hits within
    // each station and orientation
    std::unique_ptr<TClonesArray> filterScifiHits(const TClonesArray &digiHits, const std::map<std::string, float> &selection_parameters, int method=0, std::string setup="TI18", bool isMC=false);
    //Foregoing the selection_parameters option runs with the default values
    std::unique_ptr<TClonesArray> filterScifiHits(const TClonesArray &digiHits, int method=0, std::string setup="TI18", bool isMC=false);

    // Function to calculate the number of the SiPM channel with respect to the whole station
    // referene_SiPM given in the format of sndScifiHit->GetChannelID()
    int calculateSiPMNumber(int reference_SiPM);

    // Function to determine SciFi hit density for channel reference_SiPM around a radius of r
    // SiPM channels
    // If min_check == True the function stops checking the density once it reaches the minimum
    // requirement, and immediately returns -1 in case the minimum density cannot be achieved
    int densityScifi(int reference_SiPM, const TClonesArray &digiHits, int radius, int min_hit_density, bool min_check);

    // Function to check efficiently wether a plane has a required hit density within a radius of r
    // SiPM channels
    bool densityCheck(const TClonesArray &digiHits, int radius=64, int min_hit_density=36, int station=1, bool orientation=false);

    // Function to get the starting wall for a shower in the Scifi
    // Method 0 checks for a minimum Scifi Cluster density to determine whether a shower is
    // propagating on a given Scifi station or not by checking both orientations of the station,
    // and declares the interaction block as being the previous one to pass the check
    // selection_parameters must contain "radius" and "min_hit_density". In case you want the
    // selection to be made relying on a single orientation, then "orientation" can be provided
    // in the for of 0 or 1, for the horizontal and vertical orientations, respectively
    int showerInteractionWall(const TClonesArray &digiHits, const std::map<std::string, float> &selection_parameters, int method=0, std::string setup="TI18");
    // Foregoing the selection_parameters option runs with the default values
    int showerInteractionWall(const TClonesArray &digiHits, int method=0, std::string setup="TI18");

    // Find the Center of Particle Showering on the SciFi plane
    std::pair<double, double> findCentreOfGravityPerStation(const TClonesArray* digiHits, int station, Scifi* ScifiDet);

    // Function to get the hit position vectors for a specific station
    // Returns a pair of vectors containing the x and y positions of the hits in the specified station
    std::pair<std::vector<double>, std::vector<double>> hitPositionVectorsPerStation(const TClonesArray* digiHits, int station, Scifi* ScifiDet);

    // Function to compute the hit density per station
    // Returns a pair of doubles containing the hit density for horizontal and vertical orientations of that station
    std::pair<double,double> hitDensityPerStation(const TClonesArray* digiHits, int station, Scifi* ScifiDet);
  }
}
