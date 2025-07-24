#include "sndSciFiTools.h"

#include <numeric>
#include <algorithm>

#include "TH1F.h"
#include "FairLogger.h"
#include "TClonesArray.h"
#include "sndScifiHit.h"
#include "ROOT/TSeq.hxx"
#include "Scifi.h"
#include "TROOT.h"
#include "ROOT/RRangeCast.hxx"

void snd::analysis_tools::getSciFiHitsPerStation(const TClonesArray *digiHits, std::vector<int> &horizontal_hits,
                                                 std::vector<int> &vertical_hits)
{

   // Clear hits per plane vectors
   std::fill(horizontal_hits.begin(), horizontal_hits.end(), 0);
   std::fill(vertical_hits.begin(), vertical_hits.end(), 0);

   // Add valid hits to hits per plane vectors
   sndScifiHit *hit;
   TIter hitIterator(digiHits);

   while (hit = dynamic_cast<sndScifiHit *>(hitIterator.Next())) {
      if (hit->isValid()) {
         int station = hit->GetStation();
         if (hit->isVertical()) {
            vertical_hits[station - 1]++;
         } else {
            horizontal_hits[station - 1]++;
         }
      }
   }
}

int snd::analysis_tools::getTotalSciFiHits(std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits)
{
   return std::accumulate(horizontal_hits.begin(), horizontal_hits.end(),
                          std::accumulate(vertical_hits.begin(), vertical_hits.end(), 0));
}

int snd::analysis_tools::getTotalSciFiHits(const TClonesArray *digiHits)
{
   std::vector<int> horizontal_hits = std::vector<int>(5);
   std::vector<int> vertical_hits = std::vector<int>(5);

   getSciFiHitsPerStation(digiHits, horizontal_hits, vertical_hits);

   return getTotalSciFiHits(horizontal_hits, vertical_hits);
}

std::vector<float>
snd::analysis_tools::getFractionalHitsPerScifiPlane(std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits)
{

   int total_hits = getTotalSciFiHits(horizontal_hits, vertical_hits);

   std::vector<float> fractional_hits_per_station = std::vector<float>(horizontal_hits.size());

   std::transform(horizontal_hits.begin(), horizontal_hits.end(), vertical_hits.begin(),
                  fractional_hits_per_station.begin(),
                  [&total_hits](const auto &hor, const auto &ver) { return ((float)hor + ver) / total_hits; });

   return fractional_hits_per_station;
}

std::vector<float> snd::analysis_tools::getFractionalHitsPerScifiPlane(const TClonesArray *digiHits)
{
   std::vector<int> horizontal_hits = std::vector<int>(5);
   std::vector<int> vertical_hits = std::vector<int>(5);

   getSciFiHitsPerStation(digiHits, horizontal_hits, vertical_hits);

   return getFractionalHitsPerScifiPlane(horizontal_hits, vertical_hits);
}

int snd::analysis_tools::findScifiStation(std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits,
                                          float threshold)
{

   std::vector<float> frac = getFractionalHitsPerScifiPlane(horizontal_hits, vertical_hits);

   std::vector<float> frac_sum = std::vector<float>(frac.size());

   std::partial_sum(frac.begin(), frac.end(), frac_sum.begin());

   std::vector<float>::iterator station =
      std::find_if(frac_sum.begin(), frac_sum.end(), [&threshold](const auto &f) { return f > threshold; });

   return station - frac_sum.begin() + 1;
}

int snd::analysis_tools::findScifiStation(const TClonesArray *digiHits, float threshold)
{
   std::vector<int> horizontal_hits = std::vector<int>(5);
   std::vector<int> vertical_hits = std::vector<int>(5);

   getSciFiHitsPerStation(digiHits, horizontal_hits, vertical_hits);

   return findScifiStation(horizontal_hits, vertical_hits, threshold);
}

// Auxiliar function to check whether hits are valid or not and whether they are within the same
// station and orientation as the reference hit we are comparing it to
bool validateHit(sndScifiHit *aHit, int ref_station, bool ref_orientation)
{

   if (!(aHit->isValid())) {
      return false;
   }
   if (aHit->GetStation() != ref_station) {
      return false;
   }
   if (aHit->isVertical() != ref_orientation) {
      return false;
   }

   return true;
}

// Getting the max for the ScifiHits timing distribution in order to select hits within \pm 3ns
// min_x and max_x should be given in ns
// The code automatically filters for hits within the same station and orientation as the first hit
float snd::analysis_tools::peakScifiTiming(const TClonesArray &digiHits, int bins, float min_x, float max_x, bool isMC)
{

   if (digiHits.GetEntries() <= 0) {
      LOG(warning) << "digiHits has no valid SciFi Hits and as such no maximum for the timing distribution.";
      return -1.;
   }

   TH1F ScifiTiming("Timing", "Scifi Timing", bins, min_x, max_x);

   Scifi *ScifiDet = dynamic_cast<Scifi*> (gROOT->GetListOfGlobals()->FindObject("Scifi") );
   auto* hit = static_cast<sndScifiHit*>(digiHits[0]);
   int refStation = hit->GetStation();
   bool refOrientation = hit->isVertical();
   float hitTime = -1.0;
   float timeConversion = 1.;
   if (!isMC) {
      timeConversion = 1E9 / (ShipUnit::snd_freq / ShipUnit::hertz);
   }

   for (auto *p : digiHits) {
      auto *hit = dynamic_cast<sndScifiHit *>(p);
      if (!validateHit(hit, refStation, refOrientation)) {
         continue;
      }
      hitTime = hit->GetTime() * timeConversion;
      if (!isMC){
        int id_hit = hit ->GetDetectorID();
        hitTime = ScifiDet->GetCorrectedTime(id_hit, hitTime, 0);
      }
      if (hitTime < min_x || hitTime > max_x) {
         continue;
      }
      ScifiTiming.Fill(hitTime);
      hitTime = -1.0;
   }

   float peakTiming = (ScifiTiming.GetMaximumBin() - 0.5) * (max_x - min_x) / bins + min_x;

   return peakTiming;
}

// Getting all the Scifi hits for a specific station and orientation
std::unique_ptr<TClonesArray>
snd::analysis_tools::getScifiHits(const TClonesArray &digiHits, int station, bool orientation)
{

   auto selectedHits = std::make_unique<TClonesArray>("sndScifiHit");

   int i = 0;
   for (auto *p : digiHits) {
      auto *hit = dynamic_cast<sndScifiHit *>(p);
      if (!validateHit(hit, station, orientation)) {
         continue;
      }
      new ((*selectedHits)[i++]) sndScifiHit(*hit);
   }

   return selectedHits;
}

// Getting all the Scifi hits for a specific station and orientation, taking into account a filter
// of \pm range around the peak of the timing distribution for said Scifi plane
// selection_parameters is the number of bins for the histogram, min_x, max_x and time window for
// the Scifi hits in ns
// make_selection == true allows you to first perform a selection of the relevant hits, and only
// afterwards apply the filter. This is recomended for when the selection has not been made
// previously
std::unique_ptr<TClonesArray> snd::analysis_tools::selectScifiHits(const TClonesArray &digiHits, int station,
                                                                   bool orientation, int bins_x, float min_x,
                                                                   float max_x, float time_lower_range,
                                                                   float time_upper_range, bool make_selection,
								   bool isMC)
{

   if (bins_x < 1) {
      LOG(FATAL) << "bins_x in selection_parameters cannot be <1. Consider using the default value of 52 instead.";
   }
   if (min_x > max_x) {
      LOG(warning) << "In selection_parameters min_x > max_x. Values will be swapped.";
      float aux_float = min_x;
      min_x = max_x;
      max_x = aux_float;
   }
   if (min_x < 0.0) {
      LOG(warning) << "In selection_parameters min_x < 0.0. Consider using the default value of 0.0.";
   }
   if (max_x < 0.0) {
      LOG(FATAL) << "In selection_parameters max_x < 0.0. Consider using the default value of 26.0.";
   }
   if (time_lower_range <= 0.0) {
      LOG(FATAL) << "In selection_parameters time_lower_range <= 0.0. Value should always be positive, in ns.";
   }
   if (time_upper_range <= 0.0) {
      LOG(FATAL) << "In selection_parameters time_upper_range <= 0.0. Value should always be positive, in ns.";
   }

   auto filteredHits = std::make_unique<TClonesArray>("sndScifiHit", digiHits.GetEntries());

   float peakTiming = -1.0;

   float timeConversion = 1.;
   if (!isMC) {
      timeConversion = 1E9 / (ShipUnit::snd_freq / ShipUnit::hertz);
   }

   if (make_selection) {

      auto selectedHits = getScifiHits(digiHits, station, orientation);

      peakTiming = peakScifiTiming(*selectedHits, bins_x, min_x, max_x, isMC);

      int i = 0;
      for (auto *p : *selectedHits) {
         auto *hit = dynamic_cast<sndScifiHit *>(p);
         if (!validateHit(hit, station, orientation)) {
            continue;
         }
         if ((peakTiming - time_lower_range > hit->GetTime() * timeConversion) ||
             (hit->GetTime() * timeConversion > peakTiming + time_upper_range)) {
            continue;
         }
         new ((*filteredHits)[i++]) sndScifiHit(*hit);
      }

   } else {
      // Does not create selectedHits and just uses digiHits (not unique_ptr)

      peakTiming = peakScifiTiming(digiHits, bins_x, min_x, max_x, isMC);

      int i = 0;
      for (auto *p : digiHits) {
         auto *hit = dynamic_cast<sndScifiHit *>(p);
         if (!validateHit(hit, station, orientation)) {
            continue;
         }
         if ((peakTiming - time_lower_range > hit->GetTime() * timeConversion) ||
             (hit->GetTime() * timeConversion > peakTiming + time_upper_range)) {
            continue;
         }
         new ((*filteredHits)[i++]) sndScifiHit(*hit);
      }
   }

   return filteredHits;
}

// Takes a map with 4 or 5 arguments as an input {"bins_x", "min_x", "max_x", "time_lower_range",
//  "time_upper_range"}
// User may foreit "time_upper_range" and function will assume a symmetric time interval
std::unique_ptr<TClonesArray>
snd::analysis_tools::selectScifiHits(const TClonesArray &digiHits, int station, bool orientation,
                                     const std::map<std::string, float> &selection_parameters, bool make_selection,
				     bool isMC)
{

   if ((selection_parameters.find("bins_x") == selection_parameters.end()) ||
       (selection_parameters.find("min_x") == selection_parameters.end()) ||
       (selection_parameters.find("max_x") == selection_parameters.end()) ||
       (selection_parameters.find("time_lower_range") == selection_parameters.end())) {
      LOG(FATAL) << "In order to use method 0 please provide the correct selection_parameters. Consider the default = "
                    "{{\"bins_x\", 52.}, {\"min_x\", 0.}, {\"max_x\", 26.}, {\"time_lower_range\", "
                    "1E9/(2*ShipUnit::snd_freq/ShipUnit::hertz))}, {\"time_upper_range\", "
                    "2E9/(ShipUnit::snd_freq/ShipUnit::hertz)}}";
   }

   float time_upper_range = -1.;

   if (selection_parameters.find("time_upper_range") == selection_parameters.end()) {
      time_upper_range = selection_parameters.at("time_lower_range");
   } else {
      time_upper_range = selection_parameters.at("time_upper_range");
   }

   return selectScifiHits(digiHits, station, orientation, int(selection_parameters.at("bins_x")),
                          selection_parameters.at("min_x"), selection_parameters.at("max_x"),
                          selection_parameters.at("time_lower_range"), time_upper_range, make_selection,
			  isMC);
}

std::unique_ptr<TClonesArray>
snd::analysis_tools::filterScifiHits(const TClonesArray &digiHits,
                                     const std::map<std::string, float> &selection_parameters, int method,
                                     std::string setup, bool isMC)
{
   TClonesArray supportArray("sndScifiHit", 0);
   auto filteredHits = std::make_unique<TClonesArray>("sndScifiHit", digiHits.GetEntries());
   int filteredHitsIndex = 0;
   int ScifiStations = 5;
   if (setup == "H8") {
      ScifiStations = 4;
   } else {
      LOG(info) << "\"TI18\" setup will be used by default, please provide \"H8\" for the Testbeam setup.";
   }

   sndScifiHit *hit;
   TIter hitIterator(&supportArray);

   if (method == 0) {

      if ((selection_parameters.find("bins_x") == selection_parameters.end()) ||
          (selection_parameters.find("min_x") == selection_parameters.end()) ||
          (selection_parameters.find("max_x") == selection_parameters.end()) ||
          (selection_parameters.find("time_lower_range") == selection_parameters.end())) {
         LOG(FATAL) << "In order to use method 0 please provide the correct selection_parameters. Consider the default "
                       "= {{\"bins_x\", 52.}, {\"min_x\", 0.}, {\"max_x\", 26.}, {\"time_lower_range\", "
                       "1E9/(2*ShipUnit::snd_freq/ShipUnit::hertz)}, {\"time_upper_range\", "
                       "1.2E9/(ShipUnit::snd_freq/ShipUnit::hertz)}}";
      }

      // This is overwriting the previous arrays with the newest one
      for (auto station : ROOT::MakeSeq(1, ScifiStations + 1)) {
         for (auto orientation : {false, true}) {

            auto supportArray = selectScifiHits(digiHits, station, orientation, selection_parameters, true, isMC);
            for (auto *p : *supportArray) {
               auto *hit = dynamic_cast<sndScifiHit *>(p);
               if (hit->isValid()) {
                  new ((*filteredHits)[filteredHitsIndex++]) sndScifiHit(*hit);
               }
            }
         }
      }
   } else {
      LOG(error) << "Please provide a valid time filter method from:";
      LOG(error) << "(0): Events within \\mp time_lower_range time_upper_range of the peak of the time distribution "
                    "for Scifi Hits within each station and orientation";
      LOG(FATAL) << "selection_parameters = {bins_x, min_x, max_x, time_lower_range, time_upper_range}.";
   }
   return filteredHits;
}

std::unique_ptr<TClonesArray>
snd::analysis_tools::filterScifiHits(const TClonesArray &digiHits, int method, std::string setup, bool isMC)
{

   std::map<std::string, float> selection_parameters;

   if (method == 0) {

      selection_parameters["bins_x"] = 52.0;
      selection_parameters["min_x"] = 0.0;
      selection_parameters["max_x"] = 26.0;
      selection_parameters["time_lower_range"] = 1E9 / (2 * ShipUnit::snd_freq / ShipUnit::hertz);
      selection_parameters["time_upper_range"] = 1.2E9 / (ShipUnit::snd_freq / ShipUnit::hertz);

   } else {
      LOG(FATAL) << "Please use method=0. No other methods implemented so far.";
   }

   return filterScifiHits(digiHits, selection_parameters, method, setup, isMC);
}

// Caculate the number of the SiPM channel in the whole station by inputing its number in the
// sndScifiHit->GetChannelID() format
int snd::analysis_tools::calculateSiPMNumber(int reference_SiPM)
{

   int ref_matAux = reference_SiPM % 100000;
   int ref_mat = ref_matAux / 10000;
   int ref_arrayAux = reference_SiPM % 10000;
   int ref_array = ref_arrayAux / 1000;
   int ref_channel = reference_SiPM % 1000;
   int referenceChannel = ref_mat * 4 * 128 + ref_array * 128 + ref_channel;

   return referenceChannel;
}

int snd::analysis_tools::densityScifi(int reference_SiPM, const TClonesArray &digiHits, int radius, int min_hit_density,
                                      bool min_check)
{

   int hit_density = 0;

   bool orientation = false;
   if (int(reference_SiPM / 100000) % 10 == 1) {
      orientation = true;
   }
   int ref_station = reference_SiPM / 1000000;
   int referenceChannel = calculateSiPMNumber(reference_SiPM);

   for (auto *p : digiHits) {
      auto *hit = dynamic_cast<sndScifiHit *>(p);
      if (!validateHit(hit, ref_station, orientation)) {
         continue;
      }
      int hitChannel = calculateSiPMNumber(hit->GetChannelID());
      if (radius == -1) {
         hit_density++;
      } else {
         if (hitChannel > referenceChannel + radius) {
            break;
         }
         if (abs(referenceChannel - hitChannel) <= radius) {
            hit_density++;
         }
      }

      if (min_check && (hit_density >= min_hit_density)) {
         break;
      }
   }

   return hit_density;
}

// Perform a density check for a specific station and orientation (dafault is horizontal)
bool snd::analysis_tools::densityCheck(const TClonesArray &digiHits, int radius, int min_hit_density, int station,
                                       bool orientation)
{

   if (digiHits.GetEntries() <= 0) {
      return false;
   }

   if (radius <= 0) {
      LOG(FATAL) << "Radius<=0. Please provide a radius bigger than 0.";
      return false;
   }

   if (min_hit_density < 0) {
      LOG(FATAL) << "Min_hit_density < 0. Please provide a min_hit_density >= 0.";
      return false;
   }

   if (min_hit_density > 2 * radius) {
      LOG(warning) << "Warning! Radius of density check does not allow for the required minimum density!";
      return false;
   }

   // Creating a vector that stores the fired SiPM channels (should already be ordered but we sort
   // afterwards to make sure)
   std::vector<int> fired_channels{};

   // Only fill the set with hits from the same station and with the same orientation as given in
   // argument (false==horizontal and true==vertical)
   for (auto *p : digiHits) {
      auto *hit = dynamic_cast<sndScifiHit *>(p);
      if (!validateHit(hit, station, orientation)) {
         continue;
      }
      fired_channels.push_back(calculateSiPMNumber(hit->GetChannelID()));
   }

   int n_fired_channels = fired_channels.size();

   if (n_fired_channels < min_hit_density) {
      return false;
   }

   // Looping over the ordered hits, checking whether within an interval of "min_hit_density" the
   // difference between the channel IDs is smaller than "radius". If so, then we have the required
   // density. Else, we check the next combination until we meet the criterion, or end the loop,
   // returning false
   std::sort(fired_channels.begin(), fired_channels.end());
   for (int i = 0; i < n_fired_channels - min_hit_density; i++) {

      if (fired_channels[i + min_hit_density - 1] - fired_channels[i] <= radius * 2) {
         return true;
      }
   }
   return false;
}

// Retrieve the target block where the shower starts
// Method 0 checks for a minimum number of hits in a specific range (2*radius) in a station to
// consider that a shower as started before said station. User must provide a map
//"selection_parameters" with at least 2 elements "radius" and "min_hit_density"
// In order to only require 1 orientation to pass the criterion, "selection_parameters" may also
// include "orientation", which can be 0. and 1. for the horizontal and vertical orientations respec.
int snd::analysis_tools::showerInteractionWall(const TClonesArray &digiHits,
                                               const std::map<std::string, float> &selection_parameters, int method,
                                               std::string setup)
{

   int totalScifiStations = 5;
   if (setup == "H8") {
      totalScifiStations = 4;
   } else {
      LOG(info) << "\"TI18\" setup will be used by default, please provide \"H8\" for the Testbeam setup.";
   }

   // There is always 1 more Scifi Station than a target block. As such, showerStart == totalScifiStations
   // means that the shower did not start developing in the target, before the last Scifi Station
   int showerStart = totalScifiStations;

   if (method == 0) {

      if ((selection_parameters.find("radius") == selection_parameters.end()) ||
          (selection_parameters.find("min_hit_density") == selection_parameters.end())) {
         LOG(FATAL)
            << "Argument of select_parameters is incorrect. Please provide a map with the arguments \"radius\" and "
               "\"min_hit_density\". Consider using the default values of {{\"radius\",64}, {\"min_hit_density\",36}}.";
      }

      for (int scifiStation = 1; scifiStation <= totalScifiStations; scifiStation++) {

         // For each ScifiStation we check whether we see clusters with the desired parameters
         // By default, we require passing the check on both the horizontal and vertical mats
         // In case selection_parameters["orientation"] was provided, we set the NOT CHOSEN orientation
         // as true by default, so as to only need to pass the chosen orientation check
         bool horizontalCheck = false;
         bool verticalCheck = false;
         if (selection_parameters.find("orientation") != selection_parameters.end()) {
            if (int(selection_parameters.at("orientation")) == 0) {
               verticalCheck = true;
            }
            if (int(selection_parameters.at("orientation")) == 1) {
               horizontalCheck = true;
            }
         }
         horizontalCheck = (densityCheck(digiHits, int(selection_parameters.at("radius")),
                                         int(selection_parameters.at("min_hit_density")), scifiStation, false) ||
                            horizontalCheck);
         verticalCheck = (densityCheck(digiHits, int(selection_parameters.at("radius")),
                                       int(selection_parameters.at("min_hit_density")), scifiStation, true) ||
                          verticalCheck);
         if ((horizontalCheck) && (verticalCheck)) {
            showerStart = scifiStation - 1;
            return showerStart;
         }
      }
   }

   return showerStart;
}

int snd::analysis_tools::showerInteractionWall(const TClonesArray &digiHits, int method, std::string setup)
{

   if (method != 0) {
      LOG(FATAL) << "Please use method=0. No other methods implemented so far.";
   }

   std::map<std::string, float> selection_parameters = {{"radius", 64.}, {"min_hit_density", 36.}};

   return showerInteractionWall(digiHits, selection_parameters, method, setup);
}

double computeMean(const std::vector<double>& values)
{
   double sum = std::accumulate(values.begin(), values.end(), 0.0);
   double mean = sum / values.size();
   return mean;
}

std::pair<double, double>
snd::analysis_tools::findCentreOfGravityPerStation(const TClonesArray* digiHits, int station, Scifi* ScifiDet)
{
   if (!digiHits) {
      LOG(ERROR) << "Error: digiHits is null in findCentreOfGravityPerStation";
   }
   
   auto [x_positions, y_positions] = snd::analysis_tools::hitPositionVectorsPerStation(digiHits, station, ScifiDet);

   if (x_positions.empty()) {
      LOG(ERROR) << "Error: No hits enter.";
   }
   double meanX = computeMean(x_positions);
   if (y_positions.empty()) {
      LOG(ERROR) << "Error: No hits enter.";
   }
   double meanY = computeMean(y_positions);
   return {meanX, meanY};
}

std::pair<std::vector<double>,std::vector<double>> snd::analysis_tools::hitPositionVectorsPerStation(const TClonesArray *digiHits, int station, Scifi* ScifiDet){
     /*This function returns the hit vector position for the digiHits in both orientations
       Arguments:
         digiHits: A TClonesArray containing the hits in the SciFi detector.
         station: The station number for which to retrieve the hit positions.
         ScifiDet: A pointer to the Scifi detector object to retrieve SiPM positions.
       Returns:
         A pair of vectors containing the x and y positions of the hits in the specified station.
       If no hits are found, it returns empty vectors and logs an error message.
     */

     if (!digiHits) {
       LOG(ERROR) << "Error: digiHits is null";
    }
    std::vector<double> x_positions;
    std::vector<double> y_positions;

    TVector3 A, B;
    for (auto* hit : ROOT::RRangeCast<sndScifiHit*, false, decltype(*digiHits)>(*digiHits)) {
      if (!hit || !hit->isValid()) {
         continue;
      }
      if (hit->GetStation() != station) {
         continue;
      }
      ScifiDet->GetSiPMPosition(hit->GetDetectorID(), A, B);
      if (hit->isVertical()) {
         x_positions.push_back((A.X() + B.X()) * 0.5);
      }
      else {
         y_positions.push_back((A.Y() + B.Y()) * 0.5);
      }
    }
    if (x_positions.empty()) {
       LOG(ERROR) << "Error: No hits enter.";
    }
    if (y_positions.empty()) {
       LOG(ERROR) << "Error: No hits enter.";
    }
    return {x_positions, y_positions};
}
  

double hitWeightComputation(std::vector<double> hit_position) {
    /* This function returns the summation of the hit weights 
    where a hit weight is the number of neighbouring hits within 1 cm position (default width).

    Arguments: 
        hit_position: A vector containing the positions of the hits in a specific SciFi station.
    
    Returns: 
        The sum of the weights of the hits in the vector.
        Returns 0 if the vector is empty or if the sum of weights is 0.
    */

    if (hit_position.empty()) {
        LOG(INFO) << "Warning: The hit position vector is empty." << std::endl;
        return 0; // Return 0 if the vector is empty
    }

    int n_hits = hit_position.size();
    double width = 1.0; // Width around the hit to consider as a neighbour, in cm
    std::vector<double> weights; // Vector to store the weights of each hit

    // Sorting the hits for efficient neighbour counting
    std::sort(hit_position.begin(), hit_position.end());

    // Initialize pointers for the sliding window.
    // Both pointers will only move forward (or stay put) across the outer loop iterations.
    int left_ptr = 0; 
    int right_ptr = 0; 

    // Loop through each hit position to calculate its neighbour count
    for (int i = 0; i < n_hits; i++) {
        double specific_hit = hit_position[i];
        
        // Move right_ptr forward to include all hits within (specific_hit + width).
        // It will point to the first element *just outside* the right boundary of the window.
        while (right_ptr < n_hits && hit_position[right_ptr] <= specific_hit + width) {
            right_ptr++;
        }
        
        // Move left_ptr forward to exclude all hits *less than* (specific_hit - width).
        // It will point to the first element *just inside* or at the left boundary of the window.
        while (left_ptr < n_hits && hit_position[left_ptr] < specific_hit - width) {
            left_ptr++;
        }
        
        // Calculate neighbouring hits within the window:
        // The number of hits in the current window is (right_ptr - left_ptr).
        // This count includes the 'specific_hit' itself.
        double count_in_window = right_ptr - left_ptr;
        
        // Subtract 1 to exclude the 'specific_hit' itself from the neighbour count.
        // We must ensure that 'specific_hit' (hit_position[i]) is actually within the window
        // defined by left_ptr and right_ptr. Since the array is sorted and we are iterating
        // through 'i', hit_position[i] will always be between hit_position[left_ptr] and 
        // hit_position[right_ptr-1] (if left_ptr <= i < right_ptr).
        double neighbour_no_of_hits = count_in_window - 1; 

        // Ensure the neighbour count is non-negative
        if (neighbour_no_of_hits < 0) {
            neighbour_no_of_hits = 0; 
        }
        
        weights.push_back(neighbour_no_of_hits);
    }

    // Calculate the sum of all computed weights
    double sum_weights = std::accumulate(weights.begin(), weights.end(), 0.0);

    return sum_weights;
}

std::pair<double,double> snd::analysis_tools::hitDensityPerStation(const TClonesArray *digiHits, int station, Scifi* ScifiDet){
    /* 
     This function returns the hit density which is described as the summation of the hit weights 
     where a hit weigth is the number of neighbouring hits within 1 cm postion (default width)
     arguments: hit position vector : vector ontaining the positions of the hits in a specific SciFi station
     returns: the sum of the weights of the hits in the vector
     If the vector is empty, it returns 0.
     If the sum of weights is 0, it returns 0.
     */
    std::pair<std::vector<double>, std::vector<double>> hit_position_vec = hitPositionVectorsPerStation(digiHits, station, ScifiDet);
    std::vector<double> hit_position_x = hit_position_vec.first; // Assuming we are interested in X positions
    std::vector<double> hit_position_y = hit_position_vec.second; // Assuming we are interested in Y positions

    if (hit_position_x.size()==0 && hit_position_y.size()==0) {
        LOG(INFO)<< "Warning: The hit position vector is empty." << std::endl;
        return {0.0,0.0}; // Check if the vector is empty
    }

    double sum_weights_x = hitWeightComputation(hit_position_x);
    double sum_weights_y = hitWeightComputation(hit_position_y);

    return {sum_weights_x, sum_weights_y};
}

