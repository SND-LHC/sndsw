/**
 * @file LogarithmicValueMapper.hpp
 * @brief Maps values to colors using logarithmic scaling.
 *
 * LogarithmicValueMapper is a concrete implementation of the ValueMapper strategy
 * that applies logarithmic scaling to value-to-color mapping. Useful for data with
 * large dynamic ranges, compressing high values and emphasizing variations in low values.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-14
 */

#pragma once

#include "ValueMapper.hpp"

namespace snd3D {
    class LogarithmicValueMapper : public ValueMapper {
        public:
            LogarithmicValueMapper(glm::vec3 _colorMin, glm::vec3 _colorMax);
            virtual glm::vec3 toColor(float value, const std::pair<float, float>& range) override;

        private:
            glm::vec3 colorMin;
            glm::vec3 colorMax;
    };
}

