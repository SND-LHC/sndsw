/**
 * @file OrthographicProjection.hpp
 * @brief Orthographic projection matrix computation for parallel viewing.
 *
 * OrthographicProjection computes an orthographic projection matrix that creates
 * a parallel viewing projection. Useful for technical drawings and precise alignment
 * views. Maintains a camera distance parameter that affects the size of the projected area.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#pragma once

#include "BasicProjection.hpp"

namespace snd3D {
    class OrthographicProjection : public BasicProjection {
        public:
            OrthographicProjection(float _aspectRatio, float _fov, float _cameraDistance);
            void setCameraDistance(float _cameraDistance);

        protected:
            float distance;     // Distance of the camera from the target

            virtual void computeProjectionMatrix() override;
    };
}