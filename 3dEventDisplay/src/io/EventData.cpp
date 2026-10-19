/**
 * @file EventData.cpp
 * @brief Implementation of event data structure management.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-24
 */

#include "io/EventData.hpp"

namespace snd3D {

    EventData::EventData(int _id, std::string _dateTime, int64_t _timestamp)
            : id(_id), dateTime(std::move(_dateTime)), timestamp(_timestamp) {
    }

    int EventData::getId() const {
        return this->id;
    }

    std::string EventData::getDateTime() const {
        return this->dateTime;
    }

    int64_t EventData::getTimestamp() const {
        return this->timestamp;
    }

    void EventData::addDetector(DetectorData* detector) {
        this->detectors.push_back(std::unique_ptr<DetectorData>(detector));
    }

    const std::vector<std::unique_ptr<DetectorData>>& EventData::getDetectors() const {
        return this->detectors;
    }

}
