/**
 * @file TimeVariableGetter.hpp
 * @brief Extracts time values (nanoseconds) for hit color mapping.
 *
 * TimeVariableGetter is a concrete implementation of the VariableGetter strategy
 * that extracts timing information from detector hits in nanoseconds. Converts
 * internal clock cycle values to physical time units and provides the time range
 * for the current event, enabling time-based color gradients.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-14
 */

#pragma once

#include "scene/colors/VariableGetter.hpp"

#include "io/EventData.hpp"

namespace snd3D {
    class TimeVariableGetter : public VariableGetter {
        public:
            TimeVariableGetter(const EventData* event);
            virtual const std::pair<float, float>& getMappedRange() override;
            virtual float getValueToMap(HitData* hit) override;
            virtual const std::string& getUnitOfMeasure() override;

        private:
            std::string unit;
            std::pair<float, float> timeRange;
    };
}

