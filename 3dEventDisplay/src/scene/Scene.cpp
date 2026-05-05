#include "scene/Scene.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "core/Constants.hpp"

namespace snd3D {

    Scene::Scene(WindowManager &winMan, AppStateManager &stateMan, AppSettings &appSettings) :
        windowManager(winMan), stateManager(stateMan), settings(appSettings) {

        this->viewport = std::make_unique<Viewport>(this->windowManager.getAspectRatio(), constants::defaults::ORTHOGRAPHIC_PROJECTION);
        this->flat = std::make_shared<Shader>("Flat", "flat.vert", "flat.frag");
        this->transparent = std::make_shared<Shader>("Transparent", "transparent.vert", "transparent.frag", "transparent.geom");
        this->pivot = std::unique_ptr<Object>(this->objectFactory.getSphere());
        this->pivot->setShader(this->flat);
        this->axis = std::make_unique<AxisWidget>(this->flat, constants::defaults::AXIS_WIDGET_SIZE, constants::defaults::AXIS_WIDGET_MARGIN);
    }

    void Scene::update() {
        if (this->windowManager.isFramebufferChanged()) this->viewport->setAspectRatio(this->windowManager.getAspectRatio());

        switch (this->stateManager.getCurrentState()) {

            // Calculation are made here beacuse in the callbacks they would be execute too many times, slowing down the reactivity
            case AppState::MOVING_PAN: {
                float deltaX = this->windowManager.lastMousePosition[0] - this->windowManager.currentMousePosition[0];
                float deltaY = this->windowManager.currentMousePosition[1] - this->windowManager.lastMousePosition[1];

                this->viewport->moveParallel(deltaX, deltaY);
                // Update the last mouse position
                this->windowManager.lastMousePosition[0] = this->windowManager.currentMousePosition[0];
                this->windowManager.lastMousePosition[1] = this->windowManager.currentMousePosition[1];
            } break;

            case AppState::MOVING_TRACKBALL: {
                // Get the two positions on the unit sphere
                glm::vec3 destination = this->cursorToUnitSphere(this->windowManager.currentMousePosition[0], this->windowManager.currentMousePosition[1]);
                glm::vec3 origin = this->cursorToUnitSphere(this->windowManager.lastMousePosition[0], this->windowManager.lastMousePosition[1]);

                // Calculate the vector joining the two points
                glm::vec3 difference = destination - origin;

                if (difference != glm::vec3(0)) {
                    this->viewport->rotateTrackball(origin, destination);
                    // Update the last mouse position
                    this->windowManager.lastMousePosition[0] = this->windowManager.currentMousePosition[0];
                    this->windowManager.lastMousePosition[1] = this->windowManager.currentMousePosition[1];
                }
            } break;

            default:
                break;
        }

        if (this->settings.isCameraPivotActive()) { // Calculations are performed only if the pivot is active
            if (this->viewport->isCameraChanged()) {
                glm::mat4 matrix = glm::translate(glm::mat4(1.0f), this->viewport->getCameraTarget());
                matrix = glm::scale(matrix, glm::vec3(constants::sizes::PIVOT));
                this->pivot->updateModelMatrix(matrix);
            }
        }

        if (glfwGetKey(this->windowManager.getWindow(), GLFW_KEY_LEFT) == GLFW_PRESS)  this->viewport->rotateByAngles(-constants::factors::ROTATION_SPEED, 0);
        if (glfwGetKey(this->windowManager.getWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS) this->viewport->rotateByAngles(constants::factors::ROTATION_SPEED, 0);
        if (glfwGetKey(this->windowManager.getWindow(), GLFW_KEY_UP) == GLFW_PRESS)  this->viewport->rotateByAngles(0, constants::factors::ROTATION_SPEED);
        if (glfwGetKey(this->windowManager.getWindow(), GLFW_KEY_DOWN) == GLFW_PRESS) this->viewport->rotateByAngles(0, -constants::factors::ROTATION_SPEED);

        if (isInteractionState(this->stateManager.getCurrentState()) && this->settings.isTransparencyChanged()) {
            if (this->settings.isTransparencyEnabled()) {
                this->detector->setShader(this->transparent);
            } else {
                this->detector->setShader(this->flat);
            }
        }
    }

    void Scene::render() {
        switch (this->stateManager.getCurrentState()) {
            case AppState::TRACKBALL:
            case AppState::MOVING_TRACKBALL:
            case AppState::PAN:
            case AppState::MOVING_PAN:
            case AppState::EXPORT_IMAGE:

                // SOLID MESHES RENDERING
                this->flat->use();
                glDepthMask(GL_TRUE);  // Write depth
                glDisable(GL_BLEND);   // Don't use transparency

                if (this->settings.isCameraPivotActive()) {
                    this->pivot->render(*this->viewport, false);
                }

                for (const auto& e: this->hits) {
                    e->render(*this->viewport, false);
                }

                // TRANSPARENT MESHES RENDERING
                // The transparency function is set in the OpenGL initialization: GL_ONE_MINUS_SRC_ALPHA
                if (this->settings.isTransparencyEnabled()) {
                    this->transparent->use();
                    glEnable(GL_BLEND);     // Use transparency
                    glDepthMask(GL_FALSE);  // Don't write on the depth-buffer, otherwise further away meshes won't be rendered
                }

                this->detector->render(*this->viewport, false, this->settings.getEdgeAlphaValue(), this->settings.getFaceAlphaValue(), this->settings.getEdgeThickness());

                if (this->settings.isTransparencyEnabled()) {
                    glDepthMask(GL_TRUE);   // Final reset
                }

                if (this->settings.isAxisWidgetActive()) this->axis->draw(this->viewport->getViewMatrix(), this->windowManager.getCurrentResolution().x, this->windowManager.getCurrentResolution().y);
                break;

            default:
                break;
        }
    }

    void Scene::loadGeometry(std::string path) {
        Object* newGeometry = this->objectFactory.getFromFile(path); // If an exception is thrown don't replace old detector
        this->detector = std::unique_ptr<Object>(newGeometry);
        if (this->settings.isTransparencyEnabled()) {
            this->detector->setShader(this->transparent);
        } else {
            this->detector->setShader(this->flat);
        }
    }

    void Scene::setEvent(const EventData* event) {
        if (event != nullptr) {
            this->hits.clear();
            for (const auto& hit : event->getHits()) {
                auto hitMesh= std::unique_ptr<Object>(this->objectFactory.getCube());
                hitMesh->setShader(this->flat);
                glm::vec3 position(
                    static_cast<float>(hit->x),
                    static_cast<float>(hit->y),
                    static_cast<float>(hit->z)
                );
                glm::vec3 scaling(
                    static_cast<float>(hit->radiusX),
                    static_cast<float>(hit->radiusY),
                    static_cast<float>(hit->radiusZ)
                );
                glm::mat4 matrix = glm::translate(glm::mat4(1.0f), position);
                matrix = glm::scale(matrix, scaling);
                hitMesh->updateModelMatrix(matrix);
                this->hits.push_back(std::move(hitMesh));
            }
        }
    }

    glm::vec3 Scene::cursorToUnitSphere(int x, int y) {

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
