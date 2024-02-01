#pragma once

#include "sndSciFiBaseCut.h"

#include "TChain.h"
#include "sndScifiHit.h"

namespace snd {
  namespace analysis_cuts {
    class avgSciFiFiducialCut : public snd::analysis_cuts::sciFiBaseCut {
    private :
      double vertical_min, vertical_max, horizontal_min, horizontal_max;
      bool reversed;
    public :
      avgSciFiFiducialCut(double vertical_min_cut, double vertical_max_cut, double horizontal_min_cut, double horizontal_max_cut, TChain * ch, bool reverseCuts = false);
      ~avgSciFiFiducialCut(){;}

      bool passCut();
    };

  }
}
