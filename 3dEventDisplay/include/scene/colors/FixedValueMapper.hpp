/**
 * @file FixedValueMapper.hpp
 * @brief Maps all values to a fixed color.
 *
 * FixedValueMapper is a concrete implementation of the ValueMapper strategy that
 * ignores the input value and its range, returning a constant color for all hits.
 * Useful for monochromatic rendering modes where all hits in a detector should
 * appear with the same visual appearance.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-15
 */

#pragma once

#include "ValueMapper.hpp"

namespace snd3D {
    class FixedValueMapper : public ValueMapper {
        public:
            FixedValueMapper(glm::vec3 _color);
            virtual glm::vec3 toColor(float value, const std::pair<float, float>& range) override;

        private:
            glm::vec3 color;
    };
}

