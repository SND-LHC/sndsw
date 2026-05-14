#pragma once

#include <utility>
#include <vector>
#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "io/HitData.hpp"
#include "scene/colors/VariableGetter.hpp"
#include "scene/colors/ValueMapper.hpp"

/*  
 * This class uses two Strategy patterns to manage the color of the hits.
 * The first Strategy interface is VariableGetter, used to get the value that
 * needs to be mapped into a color (like energy, time, ecc...).
 * The second Strategy is used to perform the computations, that can be carried out
 * in different ways (like logarithmic, linear, ecc).
 */

namespace snd3D {
    class ColorPalette {
        public:
            ColorPalette(VariableGetter* _variableGetter, ValueMapper* _valueMapper);

            // The method that returns the specific color for the Hit
            glm::vec3 getColor(HitData* hit);

            // Get the range of the values mapped
            const std::pair<float, float>& getRange();

            // Returns a vector of colors mapped every certain interval
            // It can be used to paint the color scale
            const std::vector<glm::vec3>& getScale();

            // Get the unit of measurement of the values managed by the class
            const std::string& getUnitOfMeasure();

        private:
            std::unique_ptr<ValueMapper> valueMapper;
            std::unique_ptr<VariableGetter> variableGetter;
            const static int NUM_STEPS = 11; // Number of steps to sample the color scale
            std::vector<glm::vec3> colorScale; // Colors of the color scale sampled
    };
}
