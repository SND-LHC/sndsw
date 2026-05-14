#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <memory>

#include "io/DetectorData.hpp"

namespace snd3D {
    class EventData {
        public:
            std::pair<double, double> energyRange;
            std::pair<double, double> timeRange;

            EventData(int _id, std::string _dateTime, int64_t _timestamp);
            int getId() const;
            std::string getDateTime() const;
            int64_t getTimestamp() const;
            void addDetector(DetectorData* detector);
            const std::vector<std::unique_ptr<DetectorData>>& getDetectors() const;

        private:
            std::vector<std::unique_ptr<DetectorData>> detectors;
            const int id;
            const std::string dateTime;
            const int64_t timestamp;
    };
}
