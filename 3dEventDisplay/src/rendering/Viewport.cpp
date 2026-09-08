/**
 * @file Viewport.cpp
 * @brief Implementation of viewport and camera/projection management.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#include "rendering/Viewport.hpp"

#include "core/Constants.hpp"
#include "rendering/camera/OrthographicProjection.hpp"
#include "rendering/camera/PerspectiveProjection.hpp"

namespace snd3D {

    Viewport::Viewport(WindowManager& winMan, Gui& guiMan, bool _ortographic) :
        windowManager(winMan), guiManager(guiMan) {

        this->camera = std::make_unique<Camera>(glm::vec3(-237, 235, 248), glm::vec3(-39, 37, 446));
        this->ortographic = _ortographic;
        this->projection = std::unique_ptr<Projection>(this->ortographic ? (Projection *)
            new OrthographicProjection(this->windowManager.getAspectRatio(), constants::defaults::PROJ_FOVY, glm::distance(camera->getPosition(), camera->getTarget())) :
            new PerspectiveProjection(this->windowManager.getAspectRatio(), constants::defaults::PROJ_FOVY)
        );
    }

    void Viewport::update() {
        if (this->windowManager.isFramebufferChanged()) this->projection->setAspectRatio(this->windowManager.getAspectRatio());

        // ROTATION AND MOVING LOGIC
        if (glfwGetMouseButton(this->windowManager.getWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !this->guiManager.isPointerUsedByGui()) {
            double xPos, yPos;
            glfwGetCursorPos(this->windowManager.getWindow(), &xPos, &yPos);

            if (glfwGetKey(this->windowManager.getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                glfwGetKey(this->windowManager.getWindow(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {

                float deltaX = this->windowManager.lastMousePosition[0] - xPos;
                float deltaY = yPos - this->windowManager.lastMousePosition[1];
                this->moveParallel(deltaX, deltaY);

            } else {

                // Get the two positions on the unit sphere
                glm::vec3 destination = this->cursorToUnitSphere(xPos, yPos);
                glm::vec3 origin = this->cursorToUnitSphere(this->windowManager.lastMousePosition[0], this->windowManager.lastMousePosition[1]);
                this->rotateTrackball(origin, destination);
            }

            // Update last mouse position
            this->windowManager.lastMousePosition[0] = xPos;
            this->windowManager.lastMousePosition[1] = yPos;
        }
    }

    bool Viewport::isCameraChanged() {
        return this->camera->isChanged();
    }

    bool Viewport::isOrthographic() {
        return this->ortographic;
    }

    const glm::vec3 Viewport::getCameraTarget() const {
        return this->camera->getTarget();
    }

    const glm::vec3 Viewport::getCameraPosition() const {
        return this->camera->getPosition();
    }

    const glm::mat4& Viewport::getProjectionMatrix() const {
        return this->projection->getMatrix();
    }

    const glm::mat4& Viewport::getViewMatrix() const {
        return this->camera->getMatrix();
    }

    void Viewport::moveParallel(float deltaX, float deltaY) {
        this->camera->moveParallel(deltaX, deltaY);
    }

    void Viewport::movePerpendicular(float deltaX, float deltaY) {
        this->camera->movePerpendicular(deltaX, deltaY);
    }

    void Viewport::rotateByAngles(float deltaAngleX, float deltaAngleY) {
        this->camera->rotateByAngles(deltaAngleX, deltaAngleY);
    }

    void Viewport::rotateTrackball(glm::vec3 origin, glm::vec3 destination) {
        this->camera->rotateTrackball(origin, destination);
    }

    void Viewport::setDirection(Camera::Directions direction) {
        this->camera->setDirection(direction);
        if (!this->ortographic) {
            this->projection = std::unique_ptr<Projection>(
                new OrthographicProjection(this->windowManager.getAspectRatio(), constants::defaults::PROJ_FOVY, glm::distance(camera->getPosition(), camera->getTarget()))
            );
            this->ortographic = true;
        }
    }

    void Viewport::toggleProjectionType() {
        this->ortographic = !this->ortographic;
        this->projection = std::unique_ptr<Projection>(this->ortographic ? (Projection*)
            new OrthographicProjection(this->windowManager.getAspectRatio(), constants::defaults::PROJ_FOVY, glm::distance(camera->getPosition(), camera->getTarget())) :
            new PerspectiveProjection(this->windowManager.getAspectRatio(), constants::defaults::PROJ_FOVY)
        );
    }

    void Viewport::zoom(float offset) {
        this->camera->zoom(offset);
        if (this->ortographic) {
            auto* ortho = dynamic_cast<OrthographicProjection*>(this->projection.get());
            if (ortho) {
                ortho->setCameraDistance(glm::distance(camera->getPosition(), camera->getTarget()));
            }
        }
    }

    void Viewport::reset() {
        this->camera->reset();
        if (this->ortographic) {
            auto* ortho = dynamic_cast<OrthographicProjection*>(this->projection.get());
            if (ortho) {
                ortho->setCameraDistance(glm::distance(camera->getPosition(), camera->getTarget()));
            }
        }
    }

    glm::vec3 Viewport::cursorToUnitSphere(int x, int y) {
        int width = (int)this->windowManager.getCurrentResolution().x;
        int height = (int)this->windowManager.getCurrentResolution().y;

        // Map to [-1;1]
        glm::vec3 point;

        point.x = (2.0f * x - width) / width;
        point.y = (height - 2.0f * y) / height;

        float tmp = glm::dot(glm::vec2(point), glm::vec2(point)); // Square of the distance of the projected point from the center in the XY plane (radius^2)
        float delta = 1.0f - tmp; // Amount missing from r^2 to reach 1

        if (delta > 0.0f) {
            point.z = sqrt(delta);
        }
        else {
            // The click is outside the semisphere: there's no z that makes  x^2 + y^2 + z^2 = 1
            point.z = 0;
        }

        return normalize(point);
    }

}
