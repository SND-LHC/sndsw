/**
 * @file ValueMapper.hpp
 * @brief Abstract strategy interface for mapping numerical values to colors.
 *
 * ValueMapper is a strategy interface that defines how to map a numerical value
 * from a given range into an RGB color. Different implementations can apply
 * linear interpolation, logarithmic scaling, or fixed coloring. Enables flexible
 * color assignment based on hit properties.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-14
 */

#pragma once

#include <utility>

#include <glm/glm.hpp>

namespace snd3D {
    class ValueMapper {
        public:
            virtual glm::vec3 toColor(float value, const std::pair<float, float>& range) = 0;
    };
}
