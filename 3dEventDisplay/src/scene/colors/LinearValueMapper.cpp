#include "scene/colors/LinearValueMapper.hpp"

namespace snd3D {

    LinearValueMapper::LinearValueMapper(glm::vec3 _colorMin, glm::vec3 _colorMax) {
        this->colorMin = _colorMin;
        this->colorMax = _colorMax;
    }

    glm::vec3 LinearValueMapper::toColor(float value, const std::pair<float, float>& range) {
        return ((range.second - range.first) == 0 || value == 0) ?
            this->colorMin :
            glm::mix(this->colorMin, this->colorMax, (value - range.first) / (range.second - range.first));
    }
}
