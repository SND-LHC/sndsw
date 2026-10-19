#include "sndGeometryGetter.h"
#include "sndConfiguration.h"


#include <string>
#include <utility>
#include <stdexcept>
#include <cstdlib>

#include "Scifi.h"
#include "MuFilter.h"
#include "TPython.h"
#include "TROOT.h"
#include "TGeoManager.h"
#include "TGeoBBox.h"

// Get geometry full path, works with test beam too
// 2022 constants are included in the 2023 geofile
std::string snd::analysis_tools::GetGeoPath(int run_number, std::string csv_file_path)
{
    if (csv_file_path.empty()) {
        csv_file_path = std::string(getenv("SNDSW_ROOT")) + "/analysis/tools/geo_paths.csv";
    }
    std::ifstream file(csv_file_path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open CSV file: " + csv_file_path);
    }

    std::string line;
    std::getline(file, line); // skip header

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string token;

        std::getline(ss, token, ',');
        int min_run = std::stoi(token);

        std::getline(ss, token, ',');
        int max_run = std::stoi(token);

        std::getline(ss, token);
        std::string path = token;

        if (run_number >= min_run && run_number <= max_run) {
            return path;
        }
    }
    throw std::runtime_error("Run number not found in CSV mapping.");
}

// Get SciFi and MuFilter geometries
std::pair<Scifi *, MuFilter *> snd::analysis_tools::GetGeometry(const std::string& geometry_path)
{
    TPython::Exec("import SndlhcGeo");
    TPython::Exec(("SndlhcGeo.GeoInterface('" + geometry_path + "')").c_str());

    // Init detectors
    Scifi *scifi = new Scifi("Scifi", kTRUE);
    MuFilter *mufilter = new MuFilter("MuFilter", kTRUE);

    // Retrieve the detectors from ROOT's global list
    scifi = dynamic_cast<Scifi *>(gROOT->GetListOfGlobals()->FindObject("Scifi"));
    mufilter = dynamic_cast<MuFilter *>(gROOT->GetListOfGlobals()->FindObject("MuFilter"));

    return std::make_pair(scifi, mufilter);
}

// Get SciFi and MuFilter geometries directly from run number
std::pair<Scifi *, MuFilter *> snd::analysis_tools::GetGeometry(int run_number, const std::string& csv_file_path)
{
    std::string geometry_path = GetGeoPath(run_number, csv_file_path);

    return GetGeometry(geometry_path);
}



snd::analysis_tools::DetectorBoundaries::DetectorBoundaries(const snd::Configuration &configuration, int run_number) {
  // Load the geometry file
  std::string geometry_path = snd::analysis_tools::GetGeoPath(run_number);

  if (!TGeoManager::Import(geometry_path.c_str())) {
    std::cerr << "Failed to load geometry file: " << geometry_path << std::endl;
    return;
  }

  // Check if geometry was loaded
  if (!gGeoManager) {
    std::cerr << "Geometry manager is not initialized!" << std::endl;
    return;
  }

  // the logic behind the cryptic volMuUpstreamDet_P_GP is P for plane inside the system (0-4 for US) and then global
  // plane number(GP) for the whole MuFilter, so for US it starts from 2 in 2023 and from 3 since 2024

  std::vector<std::string> veto_nodes;
  std::vector<std::string> us_nodes;
  std::vector<std::string> scifi_nodes;
  std::vector<std::string> ds_nodes;

  for (int st{0}; st < configuration.veto_n_stations; ++st) {
    std::string node_name = Form("/Detector_0/volVeto_1/volVetoPlane_%d_%d", st, st);
    veto_nodes.push_back(node_name);
  }

  // scifi starts from 1 for some unclear reason
  for (int st{1}; st < configuration.scifi_n_stations + 1; ++st) {
    std::string node_name = Form("/Detector_0/volTarget_1/ScifiVolume%d_%d000000", st, st);
    scifi_nodes.push_back(node_name);
  }

  for (int st{0}; st < configuration.us_n_stations; ++st) {
    std::string node_name =
        Form("/Detector_0/volMuFilter_1/volMuUpstreamDet_%d_%d", st, configuration.veto_n_stations + st);
    us_nodes.push_back(node_name);
  }

  for (int st{0}; st < configuration.ds_n_stations; ++st) {
    std::string node_name = Form("/Detector_0/volMuFilter_1/volMuDownstreamDet_%d_%d", st,
                                 configuration.veto_n_stations + configuration.us_n_stations + st);
    ds_nodes.push_back(node_name);
  }

  auto FillBoundaries = [&](const std::vector<std::string> &node_paths,
                            std::vector<std::map<std::string, double>> &det_boundaries) {
    auto navigator = gGeoManager->GetCurrentNavigator();
    for (const auto &node_path : node_paths) {
      if (gGeoManager->cd(node_path.c_str())) {
        TGeoShape *shape = gGeoManager->GetCurrentNode()->GetVolume()->GetShape();
        // Check if the shape is a TGeoBBox (Box) and get dimensions
        if (shape->InheritsFrom("TGeoBBox")) {
          TGeoBBox *box = dynamic_cast<TGeoBBox *>(shape);
          double dx = box->GetDX();
          double dy = box->GetDY();
          double dz = box->GetDZ();
          const Double_t *origin = box->GetOrigin();  // gGeoManager->GetCurrentNode()->GetMatrix()->GetTranslation();

          std::map<std::string, double> boundaries;
          Double_t origin_master[3];
          navigator->cd(node_path.c_str());
          navigator->LocalToMaster(origin, origin_master);
          boundaries["x_min"] = origin_master[0] - dx;
          boundaries["x_max"] = origin_master[0] + dx;
          boundaries["x_avg"] = origin_master[0];
          boundaries["y_min"] = origin_master[1] - dy;
          boundaries["y_max"] = origin_master[1] + dy;
          boundaries["y_avg"] = origin_master[1];
          boundaries["z_min"] = origin_master[2] - dz;
          boundaries["z_max"] = origin_master[2] + dz;
          boundaries["z_avg"] = origin_master[2];

          det_boundaries.push_back(boundaries);
        }
      }
    }
  };

  FillBoundaries(veto_nodes, veto_boundaries);
  FillBoundaries(scifi_nodes, scifi_boundaries);
  FillBoundaries(us_nodes, us_boundaries);
  FillBoundaries(ds_nodes, ds_boundaries);
}

void snd::analysis_tools::DetectorBoundaries::Print() {
  auto PrintBoundaries = [](std::vector<std::map<std::string, double>> boundaries) {
    for (int i{0}; i < boundaries.size(); ++i) {
      std::cout << "Plane " << i + 1 << " :\n";
      std::cout << "\t" << boundaries[i]["x_min"] << " < x < " << boundaries[i]["x_max"]
                << "\t Average:" << boundaries[i]["x_avg"] << "\n";
      std::cout << "\t" << boundaries[i]["y_min"] << " < y < " << boundaries[i]["y_max"]
                << "\t Average:" << boundaries[i]["y_avg"] << "\n";
      std::cout << "\t" << boundaries[i]["z_min"] << " < z < " << boundaries[i]["z_max"]
                << "\t Average:" << boundaries[i]["z_avg"] << "\n";
    }
  };

  std::cout << "VETO BOUNDARIES:\n";
  PrintBoundaries(veto_boundaries);
  std::cout << "SCIFI BOUNDARIES:\n";
  PrintBoundaries(scifi_boundaries);
  std::cout << "US BOUNDARIES:\n";
  PrintBoundaries(us_boundaries);
  std::cout << "DS BOUNDARIES:\n";
  PrintBoundaries(ds_boundaries);
}

std::pair<int, int> GetDetectorStation(const int &detector_id) {
  int system = floor(detector_id / 10000);
  int station{-1};
  if (system > 10)
    station = floor(detector_id / 1000000);
  else
    station = static_cast<int>(detector_id / 1000) % 10;

  return std::make_pair(system, station);
}

const std::map<std::string, double>* snd::analysis_tools::FindBoundary(const snd::analysis_tools::DetectorBoundaries& boundaries, double z_pos)
{
    for (const auto* collection : {
            &boundaries.veto_boundaries,
            &boundaries.scifi_boundaries,
            &boundaries.us_boundaries,
            &boundaries.ds_boundaries })
    {
        for (const auto& b : *collection) {
            if (z_pos >= b.at("z_min") && z_pos <= b.at("z_max"))
                return &b;
        }
    }
    return nullptr;
}