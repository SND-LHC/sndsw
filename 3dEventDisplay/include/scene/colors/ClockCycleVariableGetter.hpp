/**
 * @file ClockCycleVariableGetter.hpp
 * @brief Extracts clock cycle values for hit color mapping.
 *
 * ClockCycleVariableGetter is a concrete implementation of the VariableGetter strategy
 * that extracts temporal information (clock cycle) from detector hits. It provides
 * the clock cycle range for the current event and retrieves individual hit timing
 * values, enabling time-based color gradients in the 3D visualization.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-14
 */

#pragma once

#include "scene/colors/VariableGetter.hpp"

#include "io/EventData.hpp"

namespace snd3D {
    class ClockCycleVariableGetter : public VariableGetter {
        public:
            ClockCycleVariableGetter(const EventData* event);
            virtual const std::pair<float, float>& getMappedRange() override;
            virtual float getValueToMap(HitData* hit) override;
            virtual const std::string& getUnitOfMeasure() override;

        private:
            std::string unit;
            std::pair<float, float> clockRange;
    };
}


