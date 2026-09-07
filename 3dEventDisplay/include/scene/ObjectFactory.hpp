/**
 * @file ObjectFactory.hpp
 * @brief Factory for creating 3D objects (from files, primitives, or hit data).
 *
 * ObjectFactory provides factory methods for instantiating 3D objects either from
 * Assimp-compatible model files or as generated geometric primitives (sphere, cube).
 * Acts as a centralized point for object creation throughout the application.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "scene/Object.hpp"
#include "io/EventData.hpp"
#include "rendering/engine/GpuMesh.hpp"
#include "rendering/engine/Material.hpp"
#include "scene/colors/ColorPalette.hpp"

namespace snd3D {
    class ObjectFactory {
        public:
            ObjectFactory();
            Object* getFromFile(std::string filePath);
            Object* getSphere();
            Object* getCube();
            Object* getHits(const EventData* event, const std::unique_ptr<ColorPalette>& colorGetter);

        private:
            std::shared_ptr<GpuMesh> sphere;
            std::shared_ptr<GpuMesh> cube;
            std::shared_ptr<Material> sphereMaterial;
    };
}
