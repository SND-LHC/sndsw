#pragma once

#include "scene/colors/VariableGetter.hpp"

#include "io/EventData.hpp"

/*
 * Concrete Strategy for VariableGetter, it returns the energy
 * of the HIT and the corresponding unit of measure.
 */
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
