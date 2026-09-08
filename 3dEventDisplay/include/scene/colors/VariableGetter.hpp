/**
 * @file VariableGetter.hpp
 * @brief Abstract strategy interface for extracting hit properties for color mapping.
 *
 * VariableGetter is a strategy interface that defines how to extract numerical values
 * from detector hits for use in color computation. Different implementations can extract
 * energy, time, or clock cycle data. Each implementation provides the data range and
 * unit of measure for its property.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-14
 */

#pragma once

#include <utility>
#include <string>

#include "io/HitData.hpp"

namespace snd3D {
    class VariableGetter {
        public:
            virtual const std::pair<float, float>& getMappedRange() = 0;
            virtual float getValueToMap(HitData* hit) = 0;
            virtual const std::string& getUnitOfMeasure() = 0;
    };
}
