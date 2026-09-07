/**
 * @file PointLight.cpp
 * @brief Implementation of point light management.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-20
 */

#include "scene/PointLight.hpp"

namespace snd3D {

    PointLight::PointLight(std::string _name, glm::vec3 _position, glm::vec3 _color, float _power) {
        this->name = _name;
        this->setPosition(_position);
        this->setColor(_color);
        this->setPower(_power);
        this->startPosition = this->position;
        this->startColor = this->color;
        this->startPower = this->power;
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

    const std::string& PointLight::getName() const {
        return this->name;
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

    void PointLight::reset() {
        this->setPosition(this->startPosition);
        this->setColor(this->startColor);
        this->setPower(this->startPower);
    }
}
