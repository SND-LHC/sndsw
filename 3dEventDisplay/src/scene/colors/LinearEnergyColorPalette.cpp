#include "scene/colors/LinearEnergyColorPalette.hpp"

namespace snd3D {

    LinearEnergyColorPalette::LinearEnergyColorPalette(std::pair<double, double> _energyRange, glm::vec3 _colorMin, glm::vec3 _colorMax)
        : AbstractColorPalette(_energyRange, _colorMin, _colorMax, "GeV") {
    }

    float LinearEnergyColorPalette::getValueToMap(HitData* hit) {
        return (float)hit->energy;
    }

    glm::vec3 LinearEnergyColorPalette::mapValueInColor(float value) {
        return glm::mix(this->colorMin, this->colorMax, (value - (float)this->range.first) / this->length);
    }
}
