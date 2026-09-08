/**
 * @file OrthographicProjection.cpp
 * @brief Implementation of orthographic projection camera.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#include "rendering/camera/OrthographicProjection.hpp"

#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

namespace snd3D {

    OrthographicProjection::OrthographicProjection(float _aspectRatio, float _fov, float _cameraDistance) : BasicProjection(_aspectRatio, _fov) {
        this->distance = _cameraDistance;
        this->computeProjectionMatrix();
    }

    void OrthographicProjection::computeProjectionMatrix() {
        // ortho() requires the vertices of the front plane: we compute them centering the view
        float halfHeight = this->distance * std::tan(glm::radians(this->fovY * 0.5f)); // Use the fov to scale the front plane
        float halfWidth = this->aspectRatio * halfHeight;
        this->matrix = glm::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, this->nearPlane, this->farPlane);
    }

    void OrthographicProjection::setCameraDistance(float _cameraDistance) {
        this->distance = _cameraDistance;
        this->computeProjectionMatrix();
    }
}
