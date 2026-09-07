/**
 * @file GpuMeshFactory.hpp
 * @brief Factory for creating GPU meshes (spheres, cubes, Assimp models).
 *
 * GpuMeshFactory provides static factory methods for generating common geometric
 * primitives and loading complex meshes from Assimp model files. Handles vertex
 * generation, normal computation, and GPU resource creation. Simplifies mesh
 * instantiation throughout the application.
 *
 * @author Enrico Bartocetti
 * @date 2026-05-14
 */

#pragma once

#include <vector>

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
            static glm::vec3 computeCenter(const std::vector<glm::vec3>& vertices);
    };
}
