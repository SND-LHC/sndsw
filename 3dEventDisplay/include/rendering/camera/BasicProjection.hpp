/**
 * @file BasicProjection.hpp
 * @brief Base class for perspective and orthographic projections.
 *
 * BasicProjection is an abstract base class providing common functionality for
 * projection matrices. It manages field-of-view, aspect ratio, near and far planes,
 * and supports dynamic updates to projection parameters. Subclasses implement
 * specific projection types (perspective, orthographic) via the pure virtual
 * computeProjectionMatrix() method.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#pragma once

#include "Projection.hpp"

namespace snd3D {
    class BasicProjection : public Projection {
        public:
            virtual void setAspectRatio(float newAspectRatio) override;
            virtual void setFov(float newFov) override;
            virtual const glm::mat4& getMatrix() override;

        protected:
            BasicProjection(float _aspectRatio, float _fov);
            float fovY;         // Vertical field of view in degrees
            float aspectRatio;  // Aspect ratio (width/height) of the viewport
            float nearPlane;    // Distance to the near clipping plane
            float farPlane;     // Distance to the far clipping plane
            glm::mat4 matrix;

            virtual void computeProjectionMatrix() = 0;
    };
}
