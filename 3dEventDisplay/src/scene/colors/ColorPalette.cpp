#include "scene/colors/ColorPalette.hpp"

namespace snd3D {

    ColorPalette::ColorPalette(VariableGetter* _variableGetter, ValueMapper* _valueMapper) {
        this->variableGetter = std::unique_ptr<VariableGetter>(_variableGetter);
        this->valueMapper = std::unique_ptr<ValueMapper>(_valueMapper);

        for (int i = 0; i < NUM_STEPS; i++) {
            this->colorScale.push_back(
                this->valueMapper->toColor(
                    this->getRange().first + i * (this->getRange().second - this->getRange().first) / ColorPalette::NUM_STEPS,
                    this->getRange()
                )
            );
        }
    }

    glm::vec3 ColorPalette::getColor(HitData *hit) {
        return this->valueMapper->toColor(this->variableGetter->getValueToMap(hit), this->getRange());
    }

    const std::pair<float, float>& ColorPalette::getRange() {
        return this->variableGetter->getMappedRange();
    }

    const std::vector<glm::vec3>& ColorPalette::getScale() {
        return this->colorScale;
    }

    const std::string& ColorPalette::getUnitOfMeasure() {
        return this->variableGetter->getUnitOfMeasure();
    }
}
