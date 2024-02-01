#pragma once

#include "sndSciFiBaseCut.h"

#include "TChain.h"
#include "sndScifiHit.h"

namespace snd {
  namespace analysis_cuts {
    class minSciFiConsecutivePlanes : public snd::analysis_cuts::sciFiBaseCut {
    public :
      minSciFiConsecutivePlanes(TChain * ch);
      ~minSciFiConsecutivePlanes(){;}

      bool passCut();

    };

  }
}
