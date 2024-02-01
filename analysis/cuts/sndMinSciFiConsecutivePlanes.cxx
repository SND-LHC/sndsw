#include "sndMinSciFiConsecutivePlanes.h"

#include "sndSciFiTools.h"

#include "TChain.h"

namespace snd::analysis_cuts{
  minSciFiConsecutivePlanes::minSciFiConsecutivePlanes(TChain * ch) : sciFiBaseCut(ch){
    cutName = "Two or more consecutive SciFi planes";

    shortName = "At least two consecutive SciFi planes";
    nbins = std::vector<int>{1};
    range_start = std::vector<double>{0};
    range_end = std::vector<double>{1};
    plot_var = std::vector<double>{-1};

  }

  bool minSciFiConsecutivePlanes::passCut(){
    initializeEvent();
    
    // For a plane to count, need both planes to have hits
    for (int i = 0; i < hits_per_plane_horizontal.size() - 1; i++){
      if (hits_per_plane_horizontal[i] * hits_per_plane_vertical[i]
	  *hits_per_plane_horizontal[i+1] * hits_per_plane_vertical[i+1] > 0) return true;
    }
    return false;
  }
}	     
