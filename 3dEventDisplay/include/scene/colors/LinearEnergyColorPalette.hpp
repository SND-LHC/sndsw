#pragma once

#include "AbstractColorPalette.hpp"

/*
 * This class is an implementation of the Themplate Method,
 * which uses the Energy as value to be mapped and performs
 * a simple linear interpolation. 
 */

namespace snd3D {
    class LinearEnergyColorPalette : public AbstractColorPalette {
        public:
            LinearEnergyColorPalette(std::pair<double, double> _energyRange, glm::vec3 _colorMin, glm::vec3 _colorMax);

        private:
            virtual glm::vec3 mapValueInColor(float value) override;
            virtual float getValueToMap(HitData* hit) override;
    };
}
