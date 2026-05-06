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
