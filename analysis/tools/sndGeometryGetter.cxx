#include "sndGeometryGetter.h"

#include <string>
#include <utility>
#include <stdexcept>

#include "Scifi.h"
#include "MuFilter.h"
#include "TPython.h"
#include "TROOT.h"

// Get geometry full path, works with test beam too
// 2022 constants are included in the 2023 geofile
std::string snd::analysis_tools::GetGeoPath(const std::string& csv_file_path, int run_number)
{
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
std::pair<Scifi *, MuFilter *> snd::analysis_tools::GetGeometry(const std::string& csv_file_path, int run_number)
{
    std::string geometry_path = GetGeoPath(csv_file_path, run_number);

    return GetGeometry(geometry_path);
}
