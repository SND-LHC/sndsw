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
    };
}
