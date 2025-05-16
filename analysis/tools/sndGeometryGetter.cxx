#include "sndGeometryGetter.h"

#include <string>
#include <utility>
#include <stdexcept>

#include "Scifi.h"
#include "MuFilter.h"
#include "TPython.h"
#include "TROOT.h"

// Get geometry full path, works with test beam too
std::string snd::analysis_tools::GetGeoPath(int run_number)
{
    std::string geo_path = "";

     if (run_number < 7357)
    {
        geo_path = "root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2023/geofile_sndlhc_TI18_V4_2023.root";
    }
    else if (run_number < 10423)
    {
        geo_path = "root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/physics/2024/geofile_sndlhc_TI18_V12_2024.root";
    }
    else if (run_number > 100237 && run_number < 100680)
    {
        geo_path = "root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/commissioning/testbeam_June2023_H8/geofile_sndlhc_H8_2023_3walls.root";
    }
    else if (run_number > 100840 && run_number < 100954)
    {
        geo_path = "root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/commissioning/testbeam_24/geofile_sndlhc_H4_2024_W_2walls_v2.root";
    }
    else if (run_number > 100953 && run_number < 100986)
    {
        geo_path = "root://eospublic.cern.ch//eos/experiment/sndlhc/convertedData/commissioning/testbeam_24/geofile_sndlhc_H4_2024_Fe_1wall.root";
    }
    else
    {
        throw std::runtime_error{"Run not found."};
    }
    return geo_path;
}

// Get SciFi and MuFilter geometries
std::pair<Scifi *, MuFilter *> snd::analysis_tools::GetGeometry(std::string geometry_path)
{
    TPython::Exec("import SndlhcGeo");
    TPython::Exec(("SndlhcGeo.GeoInterface('" + geometry_path + "')").c_str());

    // Init detectors
    Scifi *scifi = new Scifi("Scifi", kTRUE);
    MuFilter *mufilter = new MuFilter("MuFilter", kTRUE);

    // Retrieve the detectors from ROOT's global list
    scifi = (Scifi *)gROOT->GetListOfGlobals()->FindObject("Scifi");
    mufilter = (MuFilter *)gROOT->GetListOfGlobals()->FindObject("MuFilter");

    return std::make_pair(scifi, mufilter);
}

// Get SciFi and MuFilter geometries directly from run number
std::pair<Scifi *, MuFilter *> snd::analysis_tools::GetGeometry(int run_number)
{
    std::string geometry_path = GetGeoPath(run_number);

    TPython::Exec("import SndlhcGeo");
    TPython::Exec(("SndlhcGeo.GeoInterface('" + geometry_path + "')").c_str());

    // Init detectors
    Scifi *scifi = new Scifi("Scifi", kTRUE);
    MuFilter *mufilter = new MuFilter("MuFilter", kTRUE);

    // Retrieve the detectors from ROOT's global list
    scifi = (Scifi *)gROOT->GetListOfGlobals()->FindObject("Scifi");
    mufilter = (MuFilter *)gROOT->GetListOfGlobals()->FindObject("MuFilter");

    return std::make_pair(scifi, mufilter);
}
