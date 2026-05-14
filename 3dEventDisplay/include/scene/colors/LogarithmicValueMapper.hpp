#pragma once

#include "ValueMapper.hpp"

/*
 * Concrete Strategy for ValueMapper, it returns the color
 * of the value mapped to a logarithmic scale.
 */
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

