#pragma once

#include "ValueMapper.hpp"

/*
 * Concrete Strategy for ValueMapper, it returns always the same color.
 */
namespace snd3D {
    class FixedValueMapper : public ValueMapper {
        public:
            FixedValueMapper(glm::vec3 _color);
            virtual glm::vec3 toColor(float value, const std::pair<float, float>& range) override;

        private:
            glm::vec3 color;
    };
}

