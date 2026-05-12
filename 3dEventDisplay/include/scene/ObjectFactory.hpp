#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "scene/Object.hpp"
#include "io/EventData.hpp"
#include "rendering/engine/GpuMesh.hpp"

namespace snd3D {
    class ObjectFactory {
        public:
            ObjectFactory();
            Object* getFromFile(std::string filePath);
            Object* getSphere();
            Object* getCube();
            Object* getHits(const EventData* event);

        private:
            static GpuMesh* createSphere(glm::vec4 baseColor = glm::vec4(0, 1, 1, 1.0f));
            static GpuMesh* createCube(glm::vec4 baseColor = glm::vec4(1, 1, 0, 1.0f));
            static GpuMesh* loadAssimpMesh(aiMesh* mesh, glm::vec4 baseColor);
            std::shared_ptr<GpuMesh> sphere;
            std::shared_ptr<GpuMesh> cube;
    };
}
