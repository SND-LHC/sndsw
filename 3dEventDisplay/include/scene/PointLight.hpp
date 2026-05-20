#pragma once

#include <glm/glm.hpp>

namespace snd3D {
    class PointLight {
        public:
            PointLight(glm::vec3 _position, glm::vec3 _color, float _power);
            const glm::vec3& getPosition() const;
            const glm::vec3& getColor() const;
            float getPower() const;
            void setPosition(glm::vec3 newPos);
            void setColor(glm::vec3 newColor);
            void setPower(float newPower);

        private:
            glm::vec3 position;
            glm::vec3 color;
            float power;
    };
}
