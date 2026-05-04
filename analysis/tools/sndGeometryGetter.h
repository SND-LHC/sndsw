#ifndef SND_GEOMETRY_GETTER
#define SND_GEOMETRY_GETTER

#include <string>
#include <utility>

#include "Scifi.h"
#include "MuFilter.h"
#include "sndConfiguration.h"


namespace snd {
    namespace analysis_tools {
        // With empty csv_file_path, it gets the official one from the sndsw installation
        std::string GetGeoPath(int run_number, std::string csv_file_path = "");
        std::pair<Scifi *, MuFilter *> GetGeometry(const std::string& geometry_path);
        std::pair<Scifi *, MuFilter *> GetGeometry(int run_number, const std::string& csv_file_path = "");

        std::pair<int, int> GetDetectorStation(const int &detector_id);  

        struct DetectorBoundaries {
          std::vector<std::map<std::string, double>> veto_boundaries;
          std::vector<std::map<std::string, double>> scifi_boundaries;
          std::vector<std::map<std::string, double>> us_boundaries;
          std::vector<std::map<std::string, double>> ds_boundaries;
          DetectorBoundaries(const snd::Configuration &configuration, int run_number);
          void Print();
        };   

        const std::map<std::string, double>* FindBoundary(const snd::analysis_tools::DetectorBoundaries &boundaries, double z_pos);
    }
}

#endif
