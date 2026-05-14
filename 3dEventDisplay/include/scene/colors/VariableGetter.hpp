#pragma once

#include <utility>
#include <string>

#include "io/HitData.hpp"

/*
 * This Strategy interface returns the values of the Hit that need to be mapped.
 * It returns also the unit of measure of the scale, so it can be visualized in the GUI.
 */
namespace snd3D {
    class VariableGetter {
        public:
            virtual const std::pair<float, float>& getMappedRange() = 0;
            virtual float getValueToMap(HitData* hit) = 0;
            virtual const std::string& getUnitOfMeasure() = 0;
    };
}
