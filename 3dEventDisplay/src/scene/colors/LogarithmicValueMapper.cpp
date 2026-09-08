/**
 * @file LogarithmicValueMapper.cpp
 * @brief Implementation of logarithmic value-to-color mapping.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-14
 */

#include "scene/colors/LogarithmicValueMapper.hpp"

#include <cmath>
#include <algorithm>

namespace snd3D {

    LogarithmicValueMapper::LogarithmicValueMapper(glm::vec3 _colorMin, glm::vec3 _colorMax) {
        this->colorMin = _colorMin;
        this->colorMax = _colorMax;
    }

    glm::vec3 LogarithmicValueMapper::toColor(float value, const std::pair<float, float>& range) {
        if ((range.second - range.first) == 0 || value == 0) return this->colorMin;

        float epsilon = 1.0f; // Used to shift the range to avoid log(0)

        float logMin = std::log(epsilon);
        float logMax = std::log(range.second - range.first + epsilon);

        float shiftedVal = value - range.first + epsilon;
        float t = (std::log(shiftedVal) - logMin) / (logMax - logMin);

        return glm::mix(this->colorMin, this->colorMax, std::clamp(t, 0.0f, 1.0f));
    }

}

