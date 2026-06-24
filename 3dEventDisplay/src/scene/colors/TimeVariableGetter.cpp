#include "scene/colors/TimeVariableGetter.hpp"

#include <RtypesCore.h>
#include <ShipUnit.h>

namespace snd3D {

    TimeVariableGetter::TimeVariableGetter(const EventData* event) {
        this->unit = "ns";
        this->timeRange.first = (float)event->clockCycleRange.first * ShipUnit::snd_TDC2ns;
        this->timeRange.second = (float)event->clockCycleRange.second * ShipUnit::snd_TDC2ns;
    }
    
    const std::pair<float, float>& TimeVariableGetter::getMappedRange() {
        return this->timeRange;
    }
    
    float TimeVariableGetter::getValueToMap(HitData* hit) {
        return (float)hit->clockCycle * ShipUnit::snd_TDC2ns;    
    }
    
    const std::string& TimeVariableGetter::getUnitOfMeasure() {
        return this->unit;
    }
}

