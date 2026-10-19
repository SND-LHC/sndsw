/**
 * @file PerspectiveProjection.hpp
 * @brief Perspective projection matrix computation for realistic viewing.
 *
 * PerspectiveProjection computes a perspective projection matrix that creates
 * a realistic 3D viewing experience with depth-based scaling. Objects farther away
 * appear smaller, matching human visual perception.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#pragma once

#include "BasicProjection.hpp"

namespace snd3D {
    class PerspectiveProjection : public BasicProjection {
        public:
            PerspectiveProjection(float _aspectRatio, float _fov);

        protected:
            virtual void computeProjectionMatrix() override;
    };
}