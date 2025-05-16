#pragma once

#include <string>
#include <utility>

#include "Scifi.h"
#include "MuFilter.h"

namespace snd {
    namespace analysis_tools {
        std::string GetGeoPath(int run_number);
        std::pair<Scifi *, MuFilter *> GetGeometry(std::string geometry_path);
        std::pair<Scifi *, MuFilter *> GetGeometry(int run_number);
    }
}


