/**
 * @file GeometryMismatchException.hpp
 * @brief Exception thrown when geometry configuration mismatches occur.
 *
 * GeometryMismatchException is raised when an attempt is made to load event data
 * that requires a different detector geometry configuration than the one currently
 * active. Provides detailed error information about both the active and requested
 * geometry configurations to aid in troubleshooting.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-06
 */

#pragma once

#include <stdexcept>
#include <string>
#include <cstdint>

namespace snd3D {

    class GeometryMismatchException : public std::runtime_error {

        public:
            GeometryMismatchException(uint64_t activeRun, const std::string& activeGeo, uint64_t reqRun, const std::string& reqGeo);

        private:
            static std::string buildMessage(uint64_t activeRun, const std::string& activeGeo, uint64_t reqRun, const std::string& reqGeo);
    };
}
