#include "scene/PointLight.hpp"

namespace snd3D {

    PointLight::PointLight(glm::vec3 _position, glm::vec3 _color, float _power) {
        this->setPosition(_position);
        this->setColor(_color);
        this->setPower(_power);
    }

    const glm::vec3& PointLight::getPosition() const {
        return this->position;
    }

    const glm::vec3& PointLight::getColor() const {
        return this->color;
    }

    float PointLight::getPower() const {
        return this->power;
    }

    void PointLight::setPosition(glm::vec3 newPos) {
        this->position = newPos;
    }

    void PointLight::setColor(glm::vec3 newColor) {
        this->color = glm::clamp(newColor, 0.0f, 1.0f);
    }

    void PointLight::setPower(float newPower) {
        this->power = newPower >= 0 ? newPower : 0;
    }
}
