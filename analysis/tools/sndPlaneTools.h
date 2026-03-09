#ifndef SND_PLANETOOLS_H
#define SND_PLANETOOLS_H        

#include <vector>

#include "TClonesArray.h"
#include "Scifi.h"
#include "MuFilter.h"
#include "sndConfiguration.h"
#include "sndVetoPlane.h"
#include "sndScifiPlane.h"
#include "sndUSPlane.h"
#include "sndDSPlane.h"

namespace snd {
    namespace analysis_tools {
        // Produce veto, scifi, us and ds planes from data 
        std::vector<VetoPlane> FillVeto(const Configuration &configuration, TClonesArray *mufi_hits, MuFilter *mufilter_geometry);
        std::vector<ScifiPlane> FillScifi(const Configuration &configuration, TClonesArray *sf_hits, Scifi *scifi_geometry);
        std::vector<USPlane> FillUS(const Configuration &configuration, TClonesArray *mufi_hits, MuFilter *mufilter_geometry, bool use_small_sipms=false);
        std::vector<DSPlane> FillDS(const Configuration &configuration, TClonesArray *mufi_hits, MuFilter *mufilter_geometry);
    }
}

#endif
