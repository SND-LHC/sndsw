#include "sndSciFiStationCut.h"

#include "sndSciFiTools.h"

#include "TChain.h"

namespace snd::analysis_cuts{
  sciFiStationCut::sciFiStationCut(float threshold, std::vector<int> excluded_stations, TChain * ch) : sciFiBaseCut(ch){
    fractionThreshold = threshold;
    stations_to_exclude = std::vector(excluded_stations);
    cutName = "Exclude stations";
    for (int sta : excluded_stations){
      cutName += " "+std::to_string(sta);
    }
    cutName += ". Threshold "+std::to_string(fractionThreshold);

    shortName = "SciFiStation";
    for (int sta : excluded_stations) shortName += "_"+std::to_string(sta);

    nbins = std::vector<int>{5};
    range_start = std::vector<double>{1};
    range_end = std::vector<double>{6};
    plot_var = std::vector<double>{-1};
  }

  bool sciFiStationCut::passCut(){
    initializeEvent();
    
    int station = snd::analysis_tools::findScifiStation(hits_per_plane_horizontal, hits_per_plane_vertical, fractionThreshold);
    
    plot_var[0] = station;

    if (std::find(stations_to_exclude.begin(), stations_to_exclude.end(), station) == stations_to_exclude.end()){
      return true;
    } else {
      return false;
    }
  }
}	     
