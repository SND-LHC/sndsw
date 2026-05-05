#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "scene/Object.hpp"

namespace snd3D {
    class ObjectFactory {
        public:
            ObjectFactory();
            Object* getFromFile(std::string filePath);
            Object* getSphere();
            Object* getCube();

        private:
            static Mesh* createSphere(glm::vec4 baseColor = glm::vec4(0, 1, 1, 1.0f));
            static Mesh* createCube(glm::vec4 baseColor = glm::vec4(1, 1, 0, 1.0f));
            std::shared_ptr<Mesh> sphere;
            std::shared_ptr<Mesh> cube;
    };
}
