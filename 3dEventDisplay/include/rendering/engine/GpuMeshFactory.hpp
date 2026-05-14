#pragma once

#include <glm/glm.hpp>
#include <assimp/mesh.h>

#include "core/Constants.hpp"
#include "rendering/engine/GpuMesh.hpp"

namespace snd3D {
    class GpuMeshFactory {
        public:
            static GpuMesh* createSphere(glm::vec4 baseColor = glm::vec4(constants::defaults::colors::PIVOT_R, constants::defaults::colors::PIVOT_G, constants::defaults::colors::PIVOT_B, 1));
            static GpuMesh* createCube(glm::vec4 baseColor = glm::vec4(1, 1, 0, 1.0f));
            static GpuMesh* loadAssimpMesh(aiMesh* mesh, glm::vec4 baseColor);
        
        private:
            GpuMeshFactory() {}
    };
}
