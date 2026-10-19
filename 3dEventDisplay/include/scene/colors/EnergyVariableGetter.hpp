/**
 * @file EnergyVariableGetter.hpp
 * @brief Extracts energy values for hit color mapping.
 *
 * EnergyVariableGetter is a concrete implementation of the VariableGetter strategy
 * that extracts energy information from detector hits. It provides the energy range
 * for the current event and retrieves individual hit energy values, enabling
 * energy-based color gradients in the 3D visualization.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-14
 */

#pragma once

#include "scene/colors/VariableGetter.hpp"

#include "io/EventData.hpp"

namespace snd3D {
    class EnergyVariableGetter : public VariableGetter {
        public:
            EnergyVariableGetter(const EventData* event);
            virtual const std::pair<float, float>& getMappedRange() override;
            virtual float getValueToMap(HitData* hit) override;
            virtual const std::string& getUnitOfMeasure() override;

        private:
            std::string unit;
            std::pair<float, float> energyRange;
    };
}
