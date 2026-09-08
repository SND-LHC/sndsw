/**
 * @file BasicProjection.cpp
 * @brief Implementation of the basic projection camera.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#include "rendering/camera/BasicProjection.hpp"

#include <glm/glm.hpp>

#include "core/Constants.hpp"

namespace snd3D {

    BasicProjection::BasicProjection(float _aspectRatio, float _fov) {
        this->nearPlane = constants::limits::PROJ_NEARPLANE;
        this->farPlane = constants::limits::PROJ_FARPLANE;
        this->aspectRatio = _aspectRatio;
        this->fovY = glm::clamp(_fov, constants::limits::PROJ_FOVY_MIN, constants::limits::PROJ_FOVY_MAX);
        this->matrix = glm::mat4(1.0f);
    }

    void BasicProjection::setAspectRatio(float newAspectRatio) {
        if (std::isnan(newAspectRatio)) return;
        this->aspectRatio = newAspectRatio;
        this->computeProjectionMatrix();
    }

    void BasicProjection::setFov(float newFov) {
        this->fovY = glm::clamp(newFov, constants::limits::PROJ_FOVY_MIN, constants::limits::PROJ_FOVY_MAX);
        this->computeProjectionMatrix();
    }

    const glm::mat4& BasicProjection::getMatrix() {
        return this->matrix;
    }
}