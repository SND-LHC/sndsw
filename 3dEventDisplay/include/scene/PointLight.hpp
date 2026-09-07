/**
 * @file PointLight.hpp
 * @brief Represents a point light source in the 3D scene.
 *
 * PointLight encapsulates a point light with position, color, and power parameters
 * for scene illumination. Supports dynamic property updates and state reset functionality.
 * Used in conjunction with shaders to compute Phong-based lighting for the scene.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-20
 */

#pragma once

#include <string>

#include <glm/glm.hpp>

namespace snd3D {
    class PointLight {
        public:
            PointLight(std::string _name, glm::vec3 _position, glm::vec3 _color, float _power);
            const glm::vec3& getPosition() const;
            const glm::vec3& getColor() const;
            float getPower() const;
            const std::string& getName() const;
            void setPosition(glm::vec3 newPos);
            void setColor(glm::vec3 newColor);
            void setPower(float newPower);
            void reset();

        private:
            std::string name;
            glm::vec3 position;
            glm::vec3 color;
            float power;

            // Used to reset
            glm::vec3 startPosition;
            glm::vec3 startColor;
            float startPower;
    };
}
