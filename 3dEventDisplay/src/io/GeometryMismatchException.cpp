#include "io/GeometryMismatchException.hpp"

#include <sstream>

namespace snd3D {

    GeometryMismatchException::GeometryMismatchException(uint64_t activeRun, const std::string& activeGeo, uint64_t reqRun, const std::string& reqGeo)
        : std::runtime_error(GeometryMismatchException::buildMessage(activeRun, activeGeo, reqRun, reqGeo)) {
        
    }

    std::string GeometryMismatchException::buildMessage(uint64_t activeRun, const std::string& activeGeo, uint64_t reqRun, const std::string& reqGeo) {
        std::ostringstream oss;
        oss << "GEOMETRY MISMATCH ERROR:\n"
            << "The requested Run requires a different geometry configuration than the one currently loaded:\n"
            << " - Active: Run N°" << activeRun << " [" << activeGeo << "]\n"
            << " - Requested: Run N°" << reqRun << " [" << reqGeo << "]\n\n"
            << "ACTION REQUIRED: To resolve this conflict and load a different geometry, you must RESTART the application";
        return oss.str();
    }
}
