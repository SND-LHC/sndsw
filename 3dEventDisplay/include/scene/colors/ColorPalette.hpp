/**
 * @file ColorPalette.hpp
 * @brief Manages color mapping for hit visualization using strategy pattern.
 *
 * ColorPalette combines two strategy patterns: VariableGetter (selects which hit
 * property to map: energy, time, etc.) and ValueMapper (applies color transformation:
 * linear, logarithmic, etc.). Generates a pre-computed color scale for GUI display
 * and provides per-hit color computation. Enables flexible, data-driven coloring of
 * detector hits in the 3D scene.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-14
 */

#pragma once

#include <utility>
#include <vector>
#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "io/HitData.hpp"
#include "scene/colors/VariableGetter.hpp"
#include "scene/colors/ValueMapper.hpp"

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
