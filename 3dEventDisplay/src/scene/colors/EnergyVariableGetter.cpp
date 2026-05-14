#include "scene/colors/EnergyVariableGetter.hpp"

namespace snd3D {

    EnergyVariableGetter::EnergyVariableGetter(const EventData* event) {
        this->unit = "GeV";
        this->energyRange.first = (float)event->energyRange.first;
        this->energyRange.second = (float)event->energyRange.second;
    }
    
    const std::pair<float, float>& EnergyVariableGetter::getMappedRange() {
        return this->energyRange;    
    }
    
    float EnergyVariableGetter::getValueToMap(HitData* hit) {
        return (float)hit->energy;    
    }
    
    const std::string& EnergyVariableGetter::getUnitOfMeasure() {
        return this->unit;
    }
}
