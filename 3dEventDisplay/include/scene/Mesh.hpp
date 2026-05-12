#pragma once

#include <string>
#include <memory>

#include <assimp/mesh.h>
#include <glm/glm.hpp>

#include "rendering/engine/Material.hpp"
#include "rendering/engine/GpuMesh.hpp"
#include "rendering/engine/Shader.hpp"

namespace snd3D {
    class Mesh {

        friend class Gui; // Needs access to show and edit mesh propreties

        public:
            Mesh(std::string _name, const std::shared_ptr<GpuMesh>& _gpuMesh);
            void setMaterial(const std::shared_ptr<Material>& _material);
            void setActive(bool value);
            void render(const glm::mat4& modelMatrix, bool showAnchor, Shader* shader);

        private:
            std::string name;
            std::shared_ptr<Material> material;
            std::shared_ptr<GpuMesh> gpuMesh;
            bool active = true;
    };
}
