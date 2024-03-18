#include "sndSciFiTools.h"

#include <numeric>
#include <algorithm>
#include <set>

#include "TChain.h"
#include "TH1D.h"
#include "TMath.h"
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
      int sta = hit->GetStation();
      if (hit->isVertical()){
	vertical_hits[sta-1]++;
      } else {
	horizontal_hits[sta-1]++;
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

// Getting the max for the ScifiHits timing distribution in order to select hits within \pm 3ns
// minX and maxX should be given in ns
// The code automatically filters for hits within the same station and orientation as the first hit
float snd::analysis_tools::peakScifiTiming(TClonesArray * digiHits, int bins, double minX, double maxX){

  if (digiHits == NULL || digiHits->GetEntries() <= 0){return -1.0;}

  TH1D * ScifiTiming = new TH1D("Timing", "Scifi Timing", bins, minX, maxX);

  sndScifiHit * hit;
  TIter hitIterator(digiHits);

  int refStation = ((sndScifiHit*)digiHits->At(0))->GetStation();
  bool refOrientation = ((sndScifiHit*)digiHits->At(0))->isVertical();
  double hitTime = -1.0;

  while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
    if (hit->isValid()){
      if (hit->GetStation() != refStation || hit->isVertical() != refOrientation){continue;}
      hitTime = hit->GetTime()*1E9/160.316E6;
      if (hitTime < minX || hitTime > maxX){continue;}
      ScifiTiming->Fill(hitTime);
      hitTime = -1.0;
    }
  }

  float peakTiming = (ScifiTiming->GetMaximumBin()-0.5)*(maxX-minX)/bins + minX;

  delete ScifiTiming;
	 
  return peakTiming;
}

// Getting all the Scifi hits for a specific station and orientation
TClonesArray snd::analysis_tools::getScifiHits(TClonesArray * digiHits, int station, bool orientation){

  sndScifiHit * hit;
  TIter hitIterator(digiHits);

  int length = 0;
  while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
    if (hit->isValid()){
      if (station != hit->GetStation()){continue;}
      if (orientation != hit->isVertical()){continue;}
      length++;
    }
  }

  TClonesArray selectedHits("sndScifiHit", length);

  hitIterator.Reset();  
  int i = 0;
  while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
    if (hit->isValid()){
      if (station != hit->GetStation()){continue;}
      if (orientation != hit->isVertical()){continue;}
      selectedHits[i] = hit->Clone();

      i++;
    }
  }

  return selectedHits;

}



// Getting all the Scifi hits for a specific station and orientation, taking into account a filter
// of \pm range around the peak of the timing distribution for said Scifi plane
// option == true allows you to first perform a selection of the relevant hits, and only afterwards
// apply the filter. This is recomended for when the selection has not been made previously
TClonesArray snd::analysis_tools::getScifiHits(TClonesArray * digiHits, int station, bool orientation, float range, bool option){

  TClonesArray selectedHits("sndScifiHit", 0);
  int length = 0;

  sndScifiHit * hit;
  TIter hitIterator(digiHits);


  if (option) {
    selectedHits = getScifiHits(digiHits, station, orientation);
  }
  //Not an elegant solution but should work. Needs testing
  else{
    while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
      selectedHits[length] = hit->Clone();
      length++;
    } 

  }

  float peakTiming = peakScifiTiming(&selectedHits, 52, 0.0, 26.0);

  TIter secondHitIterator(&selectedHits);

  if (length == 0){
    while ( (hit = (sndScifiHit*) secondHitIterator.Next()) ){
      if (hit->isValid()){
        if (station != hit->GetStation()){continue;}
        if (orientation != hit->isVertical()){continue;}
        length++;
      }
    }
  }

  TClonesArray filteredHits("sndScifiHit", length);

  hitIterator.Reset();  
  int i = 0;
  while ( (hit = (sndScifiHit*) secondHitIterator.Next()) ){
    if (hit->isValid()){
      if (station != hit->GetStation()){continue;}
      if (orientation != hit->isVertical()){continue;}
      if (abs(peakTiming - hit->GetTime()*1E9/160.316E6) > range){continue;}
      filteredHits[i] = hit->Clone();
      i++;
    }
  }

  return filteredHits;
}



// Filter currently applies to TI18 (5 Scifi Planes)
TClonesArray snd::analysis_tools::filterScifiHits(TClonesArray * digiHits, int method, float range, std::string setup){
  TClonesArray emptyArray("sndScifiHit",0);
  TClonesArray filteredHits("sndScifiHit",0);
  int filteredHitsIndex = 0;
  int ScifiStations = 5;
  if(setup == "TI18"){ScifiStations = 5;}
  else if(setup == "H8"){ScifiStations = 4;}
  else{std::cout << "No valid setup was provided. \"TI18\" will be used by default, please provide \"TI18\" or \"H8\".\n";}
 
  sndScifiHit * hit;
  TIter hitIterator(&emptyArray);

  if (method == 0){

    //This is overwriting the previous arrays with the newest one
    for(int station = 1; station < ScifiStations+1; station++){
      
      emptyArray = getScifiHits(digiHits, station, false, range, true);
      hitIterator.Reset();
      while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
        if (hit->isValid()){
	  filteredHits[filteredHitsIndex]=hit->Clone();
	  filteredHitsIndex++;
	}
      }
      emptyArray = getScifiHits(digiHits, station, true, range, true);
      hitIterator.Reset();
      while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
        if (hit->isValid()){
	  filteredHits[filteredHitsIndex]=hit->Clone();
	  filteredHitsIndex++;
	}
      }

    }
	//print(filteredHits)
  }
  else{
  std::cout << "Please provide a valid time filter method from: \n";
  std::cout << "(0): Events within \\pm range of the peak of the time distribution for Scifi Hits within each station and orientation\n";
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
    if (hit->isValid()){
      int sta = hit->GetStation();
      if (ref_station != sta){continue;}
      if (orientation != hit->isVertical()){continue;}
      int hitChannel = ((int(hit->GetChannelID()%10000)/1000)*128 + hit->GetChannelID()%1000); 
      if (radius == -1){
        hit_density++;
      }
      else{
        if (hitChannel > referenceChannel + radius){break;}
	if (abs(referenceChannel - hitChannel) <= radius){hit_density++;}
      }
    }
    if (min_check && (hit_density>=min_hit_density)){break;}
  }

  return hit_density;

}


//Auxiliary function to access the nth element of the sets used to store the usefull hits
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
    std::cout << "Warning! Radius of density check does not allow for the required minimum density! \n";
    return false;}

  //Creating a set that stores the fired SiPM channels (already ordered)
  std::set<int> fired_channels = std::set<int>();

  // Making sure we only look at hits on the same station (defined by the first hit in the array)
  int ref_station = static_cast<sndScifiHit*>(digiHits->At(0))->GetStation();

  sndScifiHit * hit;
  TIter hitIterator(digiHits);

  //Only fill the set with hits from the same station and with the same orientation as given in
  //argument (false==horizontal and true==vertical)
  while ( (hit = (sndScifiHit*) hitIterator.Next()) ){
    if (hit->isValid()){
      int sta = hit->GetStation();
      if (ref_station != sta){continue;}
      if (orientation != hit->isVertical()){continue;}
      fired_channels.insert((int(hit->GetChannelID()%10000)/1000)*128 + hit->GetChannelID()%1000); 
    }
  }

  if(fired_channels.size() < min_hit_density){return false;}

  //Looping over the ordered hits, checking whether within an interval of "min_hit_density" the
  //difference between the channel IDs is smaller than "radius". If so, then we have the required
  //density. Else, we check the next combination until we meet the criterion, or end the loop,
  //returning false
  int n_fired_channels = fired_channels.size();
  for(int i=0; i<n_fired_channels-min_hit_density; i++){
    
    if(getElementN(fired_channels,i+min_hit_density) - getElementN(fired_channels,i) < radius){return true;}

  }

  return false;
}
