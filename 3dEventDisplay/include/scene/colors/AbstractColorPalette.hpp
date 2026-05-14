#pragma once

#include <utility>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "ColorPalette.hpp"
#include "io/HitData.hpp"

/*  
 * This class works as the concrete strategy that provides the mapped color.
 * Because various algorithms are possible, the mapping is performed using
 * the template method: the getColor method (template) will use the virtual methods
 * mapValueInColor and getValueToMap to compute the color based on the choosen value.
 */

namespace snd3D {
    class AbstractColorPalette : public ColorPalette {
        public:
            AbstractColorPalette(std::pair<double, double> _range, glm::vec3 _colorMin, glm::vec3 _colorMax, std::string _udm);
            virtual glm::vec3 getColor(HitData* hit) override;
            virtual const std::pair<double, double>& getRange() override;
            virtual const std::vector<glm::vec3>& getScale() override;
            virtual const std::string& getUdm() override;

        protected:
            std::pair<double, double> range;
            float length;
            glm::vec3 colorMin, colorMax;
            std::vector<glm::vec3> colorScale;
            std::string udm;

            virtual glm::vec3 mapValueInColor(float value) = 0;
            virtual float getValueToMap(HitData* hit) = 0;
    };
}
