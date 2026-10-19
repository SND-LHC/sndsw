/**
 * @file Mesh.hpp
 * @brief Represents a single mesh within an object in the 3D scene.
 *
 * Mesh is a leaf node in the scene graph containing geometry and material properties.
 * It references a GPU-resident mesh (VBO/VAO) and manages a local model matrix for
 * transformations. Supports shader assignment and provides interfaces for visualization.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#pragma once

#include <string>
#include <memory>

#include <assimp/mesh.h>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>

#include "rendering/engine/Material.hpp"
#include "rendering/engine/GpuMesh.hpp"
#include "rendering/engine/Shader.hpp"

namespace snd3D {
    class Mesh {

        friend class Gui; // Needs access to show and edit mesh propreties

        public:
            Mesh(std::string _name, const std::shared_ptr<GpuMesh>& _gpuMesh);
            void updateGlobalModelMatrix(const glm::mat4& parentModelMatrix);
            void setMaterial(const std::shared_ptr<Material>& _material);
            void setActive(bool value);
            void render(bool showAnchor, Shader* shader);
            const glm::vec3& getAnchor();

        private:
            std::string name;
            std::shared_ptr<Material> material;
            std::shared_ptr<GpuMesh> gpuMesh;
            bool active = true;
            glm::mat4 modelMatrix;
            glm::vec3 anchor;
    };
}
