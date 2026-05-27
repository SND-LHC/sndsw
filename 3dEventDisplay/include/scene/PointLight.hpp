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
