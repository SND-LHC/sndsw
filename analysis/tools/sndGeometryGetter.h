#ifndef SND_GEOMETRY_GETTER
#define SND_GEOMETRY_GETTER

#include <string>
#include <utility>

#include "Scifi.h"
#include "MuFilter.h"

namespace snd {
    namespace analysis_tools {
        std::string GetGeoPath(const std::string& csv_file_path, int run_number);
        std::pair<Scifi *, MuFilter *> GetGeometry(const std::string& geometry_path);
        std::pair<Scifi *, MuFilter *> GetGeometry(const std::string& csv_file_path, int run_number);
    }
}

#endif
