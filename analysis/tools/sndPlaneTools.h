#ifndef SND_PLANETOOLS_H
#define SND_PLANETOOLS_H        

#include <vector>

#include "TClonesArray.h"
#include "Scifi.h"
#include "MuFilter.h"
#include "sndConfiguration.h"
#include "sndScifiPlane.h"
#include "sndUSPlane.h"

namespace snd {
    namespace analysis_tools {
        // Produce scifi and us planes from data 
        std::vector<ScifiPlane> FillScifi(const Configuration &configuration, TClonesArray *sf_hits, Scifi *scifi_geometry);
        std::vector<USPlane> FillUS(const Configuration &configuration, TClonesArray *mufi_hits, MuFilter *mufilter_geometry);
    }
}

#endif