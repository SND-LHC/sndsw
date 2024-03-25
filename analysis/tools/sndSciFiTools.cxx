#include "sndSciFiTools.h"

#include <numeric>
#include <algorithm>
#include <set>

#include "TH1D.h"
#include "FairLogger.h"
#include "TClonesArray.h"
#include "sndScifiHit.h"

void snd::analysis_tools::getSciFiHitsPerStation(TClonesArray * digiHits, std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits){

  // Clear hits per plane vectors
  std::fill(horizontal_hits.begin(), horizontal_hits.end(), 0);
  std::fill(vertical_hits.begin(), vertical_hits.end(), 0);

  // Add valid hits to hits per plane vectors
  sndScifiHit * hit;
  TIter hitIterator(digiHits);

  while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
    if (hit->isValid()){
      int station = hit->GetStation();
      if (hit->isVertical()){
	vertical_hits[station-1]++;
      } else {
	horizontal_hits[station-1]++;
      }
    }
  }
}

int snd::analysis_tools::getTotalSciFiHits(std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits){
  return std::accumulate(horizontal_hits.begin(),
			 horizontal_hits.end(),
			 std::accumulate(vertical_hits.begin(),
					 vertical_hits.end(), 0));
}


int snd::analysis_tools::getTotalSciFiHits(TClonesArray * digiHits){
  std::vector<int> horizontal_hits = std::vector<int>(5);
  std::vector<int> vertical_hits = std::vector<int>(5);
    
  getSciFiHitsPerStation(digiHits, horizontal_hits, vertical_hits);

  return getTotalSciFiHits(horizontal_hits, vertical_hits);
}

std::vector<float> snd::analysis_tools::getFractionalHitsPerScifiPlane(std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits){
    
  int total_hits = getTotalSciFiHits(horizontal_hits, vertical_hits);
    
  std::vector<float> fractional_hits_per_station = std::vector<float>(horizontal_hits.size());
    
  std::transform(horizontal_hits.begin(), horizontal_hits.end(),
		 vertical_hits.begin(),
		 fractional_hits_per_station.begin(),
		 [&total_hits](const auto& hor, const auto& ver){
		   return ((float) hor + ver)/total_hits;
		 });
    
  return fractional_hits_per_station;
    
}

std::vector<float> snd::analysis_tools::getFractionalHitsPerScifiPlane(TClonesArray * digiHits){
  std::vector<int> horizontal_hits = std::vector<int>(5);
  std::vector<int> vertical_hits = std::vector<int>(5);
    
  getSciFiHitsPerStation(digiHits, horizontal_hits, vertical_hits);
    
  return getFractionalHitsPerScifiPlane(horizontal_hits, vertical_hits);
}
  
  
int snd::analysis_tools::findScifiStation(std::vector<int> &horizontal_hits, std::vector<int> &vertical_hits, float threshold){

  std::vector<float> frac = getFractionalHitsPerScifiPlane(horizontal_hits, vertical_hits);
    
  std::vector<float> frac_sum = std::vector<float>(frac.size());
    
  std::partial_sum(frac.begin(), frac.end(), frac_sum.begin());

  std::vector<float>::iterator station = std::find_if(frac_sum.begin(), frac_sum.end(),
						      [&threshold](const auto& f) {
							return f > threshold;
						      });
    
  return station - frac_sum.begin() + 1;
    
}

int snd::analysis_tools::findScifiStation(TClonesArray * digiHits, float threshold){
  std::vector<int> horizontal_hits = std::vector<int>(5);
  std::vector<int> vertical_hits = std::vector<int>(5);
    
  getSciFiHitsPerStation(digiHits, horizontal_hits, vertical_hits);

  return findScifiStation(horizontal_hits, vertical_hits, threshold);
}

// Auxiliar function to check whether hits are valid or not and whether they are within the same
// station and orientation as the reference hit we are comparing it to
bool validateHit(sndScifiHit * aHit, int ref_station, bool ref_orientation){

  if (!(aHit->isValid())){return false;}
  if (aHit->GetStation() != ref_station){return false;}
  if (aHit->isVertical() != ref_orientation){return false;}

  return true;

}


// Getting the max for the ScifiHits timing distribution in order to select hits within \pm 3ns
// min_x and max_x should be given in ns
// The code automatically filters for hits within the same station and orientation as the first hit
float snd::analysis_tools::peakScifiTiming(TClonesArray * digiHits, int bins, float min_x, float max_x){

  if (digiHits == NULL || digiHits->GetEntries() <= 0){return -1.0;}

  TH1D ScifiTiming("Timing", "Scifi Timing", bins, min_x, max_x);

  sndScifiHit * hit;
  TIter hitIterator(digiHits);

  int refStation = ((sndScifiHit*)digiHits->At(0))->GetStation();
  bool refOrientation = ((sndScifiHit*)digiHits->At(0))->isVertical();
  float hitTime = -1.0;

  while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
    if (!validateHit(hit, refStation, refOrientation)){continue;}
    hitTime = hit->GetTime()*1E9/160.316E6;
    if (hitTime < min_x || hitTime > max_x){continue;}
    ScifiTiming.Fill(hitTime);
    hitTime = -1.0;
  }

  float peakTiming = (ScifiTiming.GetMaximumBin()-0.5)*(max_x-min_x)/bins + min_x;
	 
  return peakTiming;
}

// Getting all the Scifi hits for a specific station and orientation
std::shared_ptr<TClonesArray> snd::analysis_tools::getScifiHits(TClonesArray * digiHits, int station, bool orientation){

  sndScifiHit * hit;
  TIter hitIterator(digiHits);

  int length = digiHits->GetEntries();

  std::shared_ptr<TClonesArray> selectedHits = std::make_shared<TClonesArray>("sndScifiHit", length);

  int i = 0;
  while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
    if (!validateHit(hit, station, orientation)){continue;}
    (*selectedHits)[i] = hit->Clone();
    i++;  
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
std::shared_ptr<TClonesArray> snd::analysis_tools::getScifiHits(TClonesArray * digiHits, int station, bool orientation, int bins_x, float min_x, float max_x, float range_lower, float range_upper, bool make_selection){

  if (bins_x < 1){
    LOG (warning) << "bins_x in selection_parameters cannot be <1. Default value of 52 used instead.\n";
    bins_x = 52;
  }
  if (min_x < 0.0){
    LOG (warning) << "In selection_parameters min_x < 0.0. Default value of 0.0 will be set. \n";
    min_x = 0.0;
  }
  if (max_x < 0.0){
    LOG (warning) << "In selection_parameters max_x < 0.0. Default value of 26.0 will be set. \n";
    max_x = 26.0;
  }
  if (min_x > max_x){
    LOG (warning) << "In selection_parameters min_x > max_x. Values will be swapped. \n";
    float aux_float = min_x;
    min_x = max_x;
    max_x = aux_float;
  }
  if (range_lower <= 0.0){
    LOG (warning) << "In selection_parameters range_upper <= 0.0. Default value of 0.5*1E9/160.316E6 will be set. \n";
    range_lower = 0.5*1E9/160.316E6;
  }
  if (range_upper <= 0.0){
    LOG (warning) << "In selection_parameters range_upper <= 0.0. Default value of 2.0*1E9/160.316E6 will be set. \n";
    range_upper = 2.0*1E9/160.316E6;
  }

  std::shared_ptr<TClonesArray> filteredHits = std::make_shared<TClonesArray>("sndScifiHit", digiHits->GetEntries());

  float peakTiming = -1.0;

  sndScifiHit * hit;

  if (make_selection) {

    std::shared_ptr<TClonesArray> selectedHits = std::make_shared<TClonesArray>("sndScifiHit", digiHits->GetEntries());
    selectedHits = getScifiHits(digiHits, station, orientation);

    peakTiming = peakScifiTiming(&(*selectedHits), bins_x, min_x, max_x);

    TIter hitIterator(&(*selectedHits)); 
  
    int i = 0;
    while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
      if (!validateHit(hit, station, orientation)){continue;}
      if ((peakTiming - range_lower > hit->GetTime()*1E9/160.316E6) || (hit->GetTime()*1E9/160.316E6 > peakTiming + range_upper)){continue;}
      (*filteredHits)[i] = hit->Clone();
      i++;
    }
  }

  // Does not create selectedHits and just uses digiHits (not shared_ptr)
  else{

    peakTiming = peakScifiTiming(digiHits, bins_x, min_x, max_x);

    TIter hitIterator(digiHits);
  
    int i = 0;
    while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
      if (!validateHit(hit, station, orientation)){continue;}
      if ((peakTiming - range_lower > hit->GetTime()*1E9/160.316E6) || (hit->GetTime()*1E9/160.316E6 > peakTiming + range_upper)){continue;}
      (*filteredHits)[i] = hit->Clone();
      i++;
    }
  }


  return filteredHits;
}


// Takes 1 input for range and selects events within a symmetric interval of peakTiming \pm range
std::shared_ptr<TClonesArray> snd::analysis_tools::getScifiHits(TClonesArray * digiHits, int station, bool orientation, int bins_x, float min_x, float max_x, float window_range, bool make_selection){

  return getScifiHits(digiHits, station, orientation, bins_x, min_x, max_x, window_range, window_range, make_selection);
}



// Takes a vector with 4 or 5 arguments as an input
// When vector as fewer than 4 arguments corrects them to a default of [52, 0.0, 26.0,
// 1E9/(2*160.316E6)] and runs for the symmetric interval
std::shared_ptr<TClonesArray> snd::analysis_tools::getScifiHits(TClonesArray * digiHits, int station, bool orientation, std::vector<float> &selection_parameters, bool make_selection){

  if (selection_parameters.size() < 4){
    LOG (warning) << "Dimensions of selection_parameters are incorrect. Will use default values of [bins_x=52; min_x=0.0; max_x=26.0; range=\\pm 1E9/(2*160.316E6)] \n";
    for (int j = 0; j<4-selection_parameters.size();j++){selection_parameters.push_back(-1.);}
    selection_parameters[0]=52.0;
    selection_parameters[1]=0.0;
    selection_parameters[2]=26.0;
    selection_parameters[3]=1E9/(2*160.316E6);
  }

  float range_lower = -1.;
  float range_upper = -1.;

  if (selection_parameters.size() == 4){
    range_lower = selection_parameters[3];
    range_upper = selection_parameters[3];
  }
  else{
    range_lower = selection_parameters[3];
    range_upper = selection_parameters[4];
  }

  return getScifiHits(digiHits, station, orientation, int(selection_parameters[0]), selection_parameters[1], selection_parameters[2], range_lower, range_upper, make_selection);

}


// Filter currently applies to TI18 (5 Scifi Planes)
std::shared_ptr<TClonesArray> snd::analysis_tools::filterScifiHits(TClonesArray * digiHits, std::vector<float> &selection_parameters, int method, std::string setup){
  TClonesArray emptyArray("sndScifiHit",0);
  std::shared_ptr<TClonesArray> filteredHits = std::make_shared<TClonesArray>("sndScifiHit",digiHits->GetEntries());
  int filteredHitsIndex = 0;
  int ScifiStations = 5;
  if(setup == "H8"){ScifiStations = 4;}
  else{LOG (info) << "\"TI18\" setup will be used by default, please provide \"H8\" for the Testbeam setup.\n";}
 
  sndScifiHit * hit;
  TIter hitIterator(&emptyArray);

  if (method == 0){

    //This is overwriting the previous arrays with the newest one
    for(int station = 1; station < ScifiStations+1; station++){
      
      emptyArray = (*getScifiHits(digiHits, station, false, selection_parameters, true));
      hitIterator.Reset();
      while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
        if (hit->isValid()){
	  (*filteredHits)[filteredHitsIndex] = hit->Clone();
	  filteredHitsIndex++;
	}
      }
      emptyArray = (*getScifiHits(digiHits, station, true, selection_parameters, true));
      hitIterator.Reset();
      while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
        if (hit->isValid()){
	  (*filteredHits)[filteredHitsIndex] = hit->Clone();
	  filteredHitsIndex++;
	}
      }

    }
  }
  else{
  LOG (error) << "Please provide a valid time filter method from: \n";
  LOG (error) << "(0): Events within \\mp range_lower range_upper of the peak of the time distribution for Scifi Hits within each station and orientation\n";
  LOG (error) << "selection_parameters = [bins_x, min_x, max_x, range_lower, range_upper].\n";
  return filteredHits;
  }

  return filteredHits;

}


int snd::analysis_tools::densityScifi(int reference_SiPM, TClonesArray * digiHits, int radius, int min_hit_density, bool min_check){

  int hit_density = 0;
 
  bool orientation = false;
  if (int(reference_SiPM/100000)%10 == 1){orientation = true;}
  int ref_station = reference_SiPM/1000000;


  // Add valid hits to hits per plane vectors
  sndScifiHit * hit;
  TIter hitIterator(digiHits);

  int referenceChannel = (int(reference_SiPM%10000)/1000)*128+ reference_SiPM%1000;

  while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
    if (!validateHit(hit, ref_station, orientation)){continue;}
    int hitChannel = hit->GetSiPM()*128 + hit->GetSiPMChan(); 
    if (radius == -1){
      hit_density++;
    }
    else{
      if (hitChannel > referenceChannel + radius){break;}
      if (abs(referenceChannel - hitChannel) <= radius){hit_density++;}
    }
    
    if (min_check && (hit_density>=min_hit_density)){break;}
  }

  return hit_density;

}


//Auxiliary function to access the nth element of the sets used to store the useful hits
//that perform the densityCheck. else should never happen, but in case it does it returns -1
//which should never happen since the set to be checked contains the SiPM channel number
int getElementN(std::set<int>& set, int n){

  int val=-1;

  if(set.size() > n){
    auto it = std::next(set.begin(), n);
    val = *it;
  }
  return val;
}


//Perform a density check for a specific station and orientation (dafault is horizontal)
bool snd::analysis_tools::densityCheck(TClonesArray * digiHits, int radius, int min_hit_density, bool orientation=false){

  if(min_hit_density > 2*radius){
    LOG (warning) << "Warning! Radius of density check does not allow for the required minimum density! \n";
    return false;}

  //Creating a set that stores the fired SiPM channels (already ordered)
  std::set<int> fired_channels{};

  // Making sure we only look at hits on the same station (defined by the first hit in the array)
  int ref_station = static_cast<sndScifiHit*>(digiHits->At(0))->GetStation();

  sndScifiHit * hit;
  TIter hitIterator(digiHits);

  //Only fill the set with hits from the same station and with the same orientation as given in
  //argument (false==horizontal and true==vertical)
  while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
    if (!validateHit(hit, ref_station, orientation)){continue;}
    fired_channels.insert(hit->GetSiPM()*128 + hit->GetSiPMChan());   
  }

  if(fired_channels.size() < min_hit_density){return false;}

  //Looping over the ordered hits, checking whether within an interval of "min_hit_density" the
  //difference between the channel IDs is smaller than "radius". If so, then we have the required
  //density. Else, we check the next combination until we meet the criterion, or end the loop,
  //returning false
  int n_fired_channels = fired_channels.size();
  for(int i=0; i<n_fired_channels-min_hit_density; i++){
    
    if(getElementN(fired_channels,i+min_hit_density) - getElementN(fired_channels,i) <= radius*2){return true;}

  }


  return false;
}
