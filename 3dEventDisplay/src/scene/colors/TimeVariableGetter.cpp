#include "scene/colors/TimeVariableGetter.hpp"

namespace snd3D {

    TimeVariableGetter::TimeVariableGetter(const EventData* event) {
        this->unit = "ns";
        this->timeRange.first = (float)event->timeRange.first;
        this->timeRange.second = (float)event->timeRange.second;
    }
    
    const std::pair<float, float>& TimeVariableGetter::getMappedRange() {
        return this->timeRange;    
    }
    
    float TimeVariableGetter::getValueToMap(HitData* hit) {
        return (float)hit->time;    
    }
    
    const std::string& TimeVariableGetter::getUnitOfMeasure() {
        return this->unit;
    }
}

