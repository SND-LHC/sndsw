#ifndef SND_GEOMETRY_GETTER
#define SND_GEOMETRY_GETTER

#include <string>
#include <utility>

#include "Scifi.h"
#include "MuFilter.h"

namespace snd {
    namespace analysis_tools {
        // With empty csv_file_path, it gets the official one from the sndsw installation
        std::string GetGeoPath(int run_number, std::string csv_file_path = "");
        std::pair<Scifi *, MuFilter *> GetGeometry(const std::string& geometry_path);
        std::pair<Scifi *, MuFilter *> GetGeometry(int run_number, const std::string& csv_file_path = "");
    }
}

#endif
