/**
 * @file FixedValueMapper.cpp
 * @brief Implementation of fixed color mapper.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-15
 */

#include "scene/colors/FixedValueMapper.hpp"

namespace snd3D {

    FixedValueMapper::FixedValueMapper(glm::vec3 _color) {
        this->color = _color;
    }

    glm::vec3 FixedValueMapper::toColor(float value, const std::pair<float, float>& range) {
        return this->color;
    }
}

