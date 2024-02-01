#pragma once

#include "sndSciFiBaseCut.h"

#include "TChain.h"
#include "sndScifiHit.h"

namespace snd {
  namespace analysis_cuts {
    class minSciFiHits : public snd::analysis_cuts::sciFiBaseCut {
    private :
      int hitThreshold;
    public :
      minSciFiHits(int threshold, TChain * ch);
      ~minSciFiHits(){;}

      bool passCut();

    };

  }
}
