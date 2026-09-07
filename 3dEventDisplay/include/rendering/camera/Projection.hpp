/**
 * @file Projection.hpp
 * @brief Abstract base class for camera projection types.
 *
 * Projection is a pure virtual interface that defines the contract for different
 * projection matrix implementations (perspective, orthographic). Provides methods
 * to update projection parameters and retrieve the computed projection matrix.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#pragma once

#include <glm/glm.hpp>

namespace snd3D {
    class Projection {
        public:
            virtual void setAspectRatio(float newAspectRatio) = 0;
            virtual void setFov(float newFov) = 0;
            virtual const glm::mat4& getMatrix() = 0;
    };
}