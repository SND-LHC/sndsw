#include "scene/colors/AbstractColorPalette.hpp"

namespace snd3D {

    AbstractColorPalette::AbstractColorPalette(std::pair<double, double> _range, glm::vec3 _colorMin, glm::vec3 _colorMax, std::string _udm) {
        this->udm = _udm;
        this->range = _range;    
        this->length = (float)(range.second - range.first);
        this->colorMin = _colorMin;
        this->colorMax = _colorMax;
    }

    glm::vec3 AbstractColorPalette::getColor(HitData *hit) {
        return (this->length == 0 || this->getValueToMap(hit) == 0) ?
            this->colorMin :
            this->mapValueInColor(this->getValueToMap(hit));
    }

    const std::pair<double, double>& AbstractColorPalette::getRange() {
        return this->range;
    }

    const std::vector<glm::vec3>& AbstractColorPalette::getScale() {
        if (this->colorScale.empty()) {
            for (int i = 0; i < 11; i++) {
                this->colorScale.push_back(this->mapValueInColor((float)this->range.first + (this->length / 11) * i));
            }
        }
        return this->colorScale;
    }

    const std::string& AbstractColorPalette::getUdm() {
        return this->udm;
    }
}
