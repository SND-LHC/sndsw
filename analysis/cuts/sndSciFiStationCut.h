#pragma once

#include "sndSciFiBaseCut.h"

#include "TChain.h"
#include "sndScifiHit.h"

namespace snd {
  namespace analysis_cuts {

    class sciFiStationCut : public snd::analysis_cuts::sciFiBaseCut {
    private :
      float fractionThreshold;
      std::vector<int> stations_to_exclude;
    public :
      sciFiStationCut(float threshold, std::vector<int> excluded_stations, TChain * ch);
      ~sciFiStationCut(){;}

      bool passCut();
    };

  }
}
