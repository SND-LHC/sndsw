/**
 * @file EventData.hpp
 * @brief Manages event-level data containing detector information.
 *
 * EventData represents a single recorded physics event, identified by event ID,
 * timestamp, and date/time metadata. Contains a collection of DetectorData objects,
 * one per detector that recorded hits in the event. Maintains global energy and
 * clock cycle ranges aggregated across all detectors to enable consistent color
 * mapping and visualization.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-24
 */

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
            std::pair<double, double> clockCycleRange;

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
