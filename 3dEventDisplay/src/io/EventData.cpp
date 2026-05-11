#include "io/EventData.hpp"

namespace snd3D {

    EventData::EventData(int _id, std::string _dateTime, int64_t _timestamp)
            : id(_id), dateTime(std::move(_dateTime)), timestamp(_timestamp) {
    }

    void EventData::addDetector(DetectorData* detector) {
        this->detectors.push_back(std::unique_ptr<DetectorData>(detector));
    }

    const std::vector<std::unique_ptr<DetectorData>>& EventData::getDetectors() const {
        return this->detectors;
    }

}
