#include "sndPlaneTools.h"

#include <vector>

#include "TClonesArray.h"
#include "Scifi.h"
#include "MuFilter.h"
#include "sndConfiguration.h"
#include "sndScifiPlane.h"
#include "sndUSPlane.h"
#include "sndScifiHit.h"
#include "MuFilterHit.h"

std::vector<snd::analysis_tools::ScifiPlane> snd::analysis_tools::FillScifi(const snd::Configuration &configuration, TClonesArray *sf_hits, Scifi *scifi_geometry)
{

  std::vector<snd::analysis_tools::ScifiPlane> scifi_planes;

  int begin{0};
  int count{0};

  int n_sf_hits{sf_hits->GetEntries()};

  for (int st{1}; st <= configuration.scifi_n_stations; ++st)
  {
    begin = count;
    while (count < n_sf_hits &&
           st == static_cast<sndScifiHit *>(sf_hits->At(count))->GetStation())
    {
      ++count;
    }
    scifi_planes.emplace_back(snd::analysis_tools::ScifiPlane(sf_hits, configuration, scifi_geometry, begin, count, st));
  }
  return scifi_planes;
}

std::vector<snd::analysis_tools::USPlane> snd::analysis_tools::FillUS(const snd::Configuration &configuration, TClonesArray *mufi_hits, MuFilter *mufilter_geometry)
{

  std::vector<snd::analysis_tools::USPlane> us_planes;

  int begin{0};
  int count{0};

  int n_mufi_hits{mufi_hits->GetEntries()};
  // skip veto/beam monitor
  while (count < n_mufi_hits &&
         static_cast<MuFilterHit *>(mufi_hits->At(count))->GetSystem() != 2)
  {
    ++count;
  }
  // plane count starts from 0
  for (int pl{0}; pl < configuration.us_n_stations; ++pl)
  {
    begin = count;
    while (count < n_mufi_hits &&
           pl == static_cast<MuFilterHit *>(mufi_hits->At(count))->GetPlane() &&
           static_cast<MuFilterHit *>(mufi_hits->At(count))->GetSystem() == 2) // stop before DS
    {
      ++count;
    }
    us_planes.emplace_back(snd::analysis_tools::USPlane(mufi_hits, configuration, mufilter_geometry, begin, count, pl + 1));
  }
  return us_planes;
}