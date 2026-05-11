#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <memory>

#include "io/DetectorData.hpp"

namespace snd3D {
    class EventData {
        public:
            const int id;
            const std::string dateTime;
            const int64_t timestamp;

            EventData(int _id, std::string _dateTime, int64_t _timestamp);
            void addDetector(DetectorData* detector);
            const std::vector<std::unique_ptr<DetectorData>>& getDetectors() const;

        private:
            std::vector<std::unique_ptr<DetectorData>> detectors;
    };
}
