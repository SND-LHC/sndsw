#pragma once

#include <utility>

#include <glm/glm.hpp>

/*
 * This Strategy class provides the interface that can be used to map a certain value
 * between a specific range into a color. It can be implemented concretely using
 * a linear scale, logarithmic scale, ...
 */
namespace snd3D {
    class ValueMapper {
        public:
            virtual glm::vec3 toColor(float value, const std::pair<float, float>& range) = 0;
    };
}
