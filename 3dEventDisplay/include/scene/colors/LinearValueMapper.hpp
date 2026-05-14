#pragma once

#include "ValueMapper.hpp"

/*
 * Concrete Strategy for ValueMapper, it returns the color
 * of the value linearly interpolated in the given range.
 */
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
