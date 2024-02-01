#include "sndMinSciFiHitsCut.h"

#include "sndSciFiTools.h"

#include "TChain.h"

namespace snd::analysis_cuts{
  minSciFiHits::minSciFiHits(int threshold, TChain * ch) : sciFiBaseCut(ch){
    hitThreshold = threshold;
    cutName = "More than "+std::to_string(hitThreshold)+" SciFi hits";
    shortName = "SciFiMinHits";
    nbins = std::vector<int>{1536};
    range_start = std::vector<double>{0};
    range_end = std::vector<double>{1536};
    plot_var = std::vector<double>{-1};
  }

  bool minSciFiHits::passCut(){
    initializeEvent();
    plot_var[0] = snd::analysis_tools::getTotalSciFiHits(hits_per_plane_horizontal, hits_per_plane_vertical);
    if ( plot_var[0] < hitThreshold) return false;
    return true;
  }
}	     
