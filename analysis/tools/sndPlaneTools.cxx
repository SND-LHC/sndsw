#include "sndPlaneTools.h"

#include <vector>
#include <stdexcept>
#include <cmath>

#include "TClonesArray.h"
#include "Scifi.h"
#include "MuFilter.h"
#include "sndConfiguration.h"
#include "sndVetoPlane.h"
#include "sndScifiPlane.h"
#include "sndUSPlane.h"
#include "sndDSPlane.h"
#include "sndScifiHit.h"
#include "MuFilterHit.h"

std::vector<snd::analysis_tools::VetoPlane> snd::analysis_tools::FillVeto(const snd::Configuration &configuration, TClonesArray *mufi_hits, MuFilter *mufilter_geometry)
{

  std::vector<snd::analysis_tools::VetoPlane> veto_planes;
  const int n_mufi_hits{mufi_hits->GetEntries()};

  const int n_station = configuration.veto_n_stations;
  std::vector<std::vector<MuFilterHit*>> plane_hits(n_station);

  for (int i{0}; i < n_mufi_hits; ++i) {
    auto hit = static_cast<MuFilterHit*>(mufi_hits->At(i));
    if (hit->GetSystem()!=1) continue;
    int station_id = hit->GetPlane();
    if (station_id > -1 && station_id < n_station) {
          plane_hits[station_id].push_back(hit);
      }
    else throw std::runtime_error{"Invalid Veto plane"};
  }
  for (int st{0}; st < n_station; ++st) {
          veto_planes.emplace_back(snd::analysis_tools::VetoPlane(plane_hits[st], configuration, mufilter_geometry, st+1));
  }
  return veto_planes;
}

std::vector<snd::analysis_tools::ScifiPlane> snd::analysis_tools::FillScifi(const snd::Configuration &configuration, TClonesArray *sf_hits, Scifi *scifi_geometry)
{

  std::vector<snd::analysis_tools::ScifiPlane> scifi_planes;
  const int n_sf_hits{sf_hits->GetEntries()};

  const int max_station = configuration.scifi_n_stations;
  std::vector<std::vector<sndScifiHit*>> stations_hits(max_station);

  for (int i{0}; i < n_sf_hits; ++i) {
      auto hit = static_cast<sndScifiHit*>(sf_hits->At(i));
      int station_id = hit->GetStation()-1;

      if (station_id > -1 && station_id < max_station) {
          stations_hits[station_id].push_back(hit);
      }
      else throw std::runtime_error{"Invalid SciFi station"};
  }
  for (int st{0}; st < max_station; ++st) {
          scifi_planes.emplace_back(snd::analysis_tools::ScifiPlane(stations_hits[st], configuration, scifi_geometry, st+1));
  }
  return scifi_planes;
}


std::vector<snd::analysis_tools::USPlane> snd::analysis_tools::FillUS(const snd::Configuration &configuration, TClonesArray *mufi_hits, MuFilter *mufilter_geometry, bool use_small_sipms)
{

  std::vector<snd::analysis_tools::USPlane> us_planes;
  const int n_mufi_hits{mufi_hits->GetEntries()};

  const int n_station = configuration.us_n_stations;
  std::vector<std::vector<MuFilterHit*>> plane_hits(n_station);

  for (int i{0}; i < n_mufi_hits; ++i) {
    auto hit = static_cast<MuFilterHit*>(mufi_hits->At(i));
    if (hit->GetSystem()!=2) continue;
    int station_id = hit->GetPlane();
    if (station_id > -1 && station_id < n_station) {
          plane_hits[station_id].push_back(hit);
      }
    else throw std::runtime_error{"Invalid US plane"};
  }
  for (int st{0}; st < n_station; ++st) {
          us_planes.emplace_back(snd::analysis_tools::USPlane(plane_hits[st], configuration, mufilter_geometry, st+1, use_small_sipms));
  }
  return us_planes;
}

std::vector<snd::analysis_tools::DSPlane> snd::analysis_tools::FillDS(const snd::Configuration &configuration, TClonesArray *mufi_hits, MuFilter *mufilter_geometry)
{

  std::vector<snd::analysis_tools::DSPlane> ds_planes;
  const int n_mufi_hits{mufi_hits->GetEntries()};

  const int n_station = configuration.ds_n_stations;
  std::vector<std::vector<MuFilterHit*>> plane_hits(n_station);

  for (int i{0}; i < n_mufi_hits; ++i) {
    auto hit = static_cast<MuFilterHit*>(mufi_hits->At(i));
    if (hit->GetSystem()!=3) continue;
    int station_id = hit->GetPlane();
    if (station_id > -1 && station_id < n_station) {
          plane_hits[station_id].push_back(hit);
      }
    else throw std::runtime_error{"Invalid DS plane"};
  }
  for (int st{0}; st < n_station; ++st) {
          ds_planes.emplace_back(snd::analysis_tools::DSPlane(plane_hits[st], configuration, mufilter_geometry, st+1));
  }
  return ds_planes;
}


std::pair<double, double> snd::analysis_tools::FindRange(const std::vector<std::vector<snd::analysis_tools::Cluster>> &planes, const bool time){

  // flatten vector
  size_t n_clusters{0};
  for (const auto& p : planes) {
      n_clusters += p.size();
  }
    
  if (n_clusters==0) return std::make_pair(std::nan(""), std::nan(""));
  std::vector<snd::analysis_tools::Cluster> flattened_planes; 
  double min, max;

  for (auto &c : planes) {
    flattened_planes.insert(flattened_planes.end(), c.begin(), c.end());
  }

  if (time) {
    std::sort(flattened_planes.begin(), flattened_planes.end(), [](const auto& a, const auto& b) {
      return a.time < b.time;
    });
  }
  else {
    std::sort(flattened_planes.begin(), flattened_planes.end(), [](const auto& a, const auto& b) {
      return a.energy < b.energy;
    });
  }

  return std::make_pair(flattened_planes[0].time, flattened_planes[flattened_planes.size()-1].time);
}

