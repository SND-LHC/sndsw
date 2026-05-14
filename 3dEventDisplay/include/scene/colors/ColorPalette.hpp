#pragma once

#include <utility>
#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "io/HitData.hpp"

/*
 * This class is used as Strategy Pattern interface to provide a common
 * interface to compute the color of an Hit based on different criteria.
 */

namespace snd3D {
    class ColorPalette {
        public:
            // The strategy that will return the computed color
            virtual glm::vec3 getColor(HitData *hit) = 0;

            // Get the range of the values mapped
            virtual const std::pair<double, double>& getRange() = 0;
            
            // Returns a vector of colors mapped every certain interval
            // It can be used to paint the color scale
            virtual const std::vector<glm::vec3>& getScale() = 0;

            // Get the unit of measurement of the values
            virtual const std::string& getUdm() = 0;
    };
}
