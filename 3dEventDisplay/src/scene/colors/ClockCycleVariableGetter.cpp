/**
 * @file ClockCycleVariableGetter.cpp
 * @brief Implementation of clock cycle value extraction for color mapping.
 *
 * @author Enrico Bartocetti
 * @date 2026-06-24
 */

#include "scene/colors/ClockCycleVariableGetter.hpp"

namespace snd3D {

    ClockCycleVariableGetter::ClockCycleVariableGetter(const EventData* event) {
        this->unit = "clk cycle";
        this->clockRange.first = (float)event->clockCycleRange.first;
        this->clockRange.second = (float)event->clockCycleRange.second;
    }

    const std::pair<float, float>& ClockCycleVariableGetter::getMappedRange() {
        return this->clockRange;
    }

    float ClockCycleVariableGetter::getValueToMap(HitData* hit) {
        return (float)hit->clockCycle;
    }

    const std::string& ClockCycleVariableGetter::getUnitOfMeasure() {
        return this->unit;
    }
}


