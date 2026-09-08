/**
 * @file LinearValueMapper.hpp
 * @brief Maps values to colors using linear interpolation.
 *
 * LinearValueMapper is a concrete implementation of the ValueMapper strategy that
 * performs linear color interpolation between a minimum and maximum color based on
 * the normalized value position within the specified range.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-14
 */

#pragma once

#include "ValueMapper.hpp"

namespace snd3D {
    class LinearValueMapper : public ValueMapper {
        public:
            LinearValueMapper(glm::vec3 _colorMin, glm::vec3 _colorMax);
            virtual glm::vec3 toColor(float value, const std::pair<float, float>& range) override;

        private:
            glm::vec3 colorMin;
            glm::vec3 colorMax;
    };
}
