#include "scene/Scene.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "core/Constants.hpp"

namespace snd3D {

    Scene::Scene(WindowManager& winMan, AppStateManager& stateMan, AppSettings& appSettings, Gui& guiMan) :
        windowManager(winMan), stateManager(stateMan), settings(appSettings) {

        this->viewport = std::make_unique<Viewport>(winMan, guiMan, constants::defaults::ORTHOGRAPHIC_PROJECTION);
        this->flat = std::make_shared<Shader>("Flat", "flat.vert", "flat.frag");
        this->transparent = std::make_shared<Shader>("Transparent", "transparent.vert", "transparent.frag", "transparent.geom");
        this->pivot = std::unique_ptr<Object>(this->objectFactory.getSphere());
        this->pivot->setShader(this->flat);
        this->axis = std::make_unique<AxisWidget>(this->flat, constants::defaults::AXIS_WIDGET_SIZE, constants::defaults::AXIS_WIDGET_MARGIN);
    }

    void Scene::update() {
        this->viewport->update(); // Updates camera and projection

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

        if (this->stateManager.getCurrentState() == AppState::INTERACTION && this->settings.isTransparencyChanged()) {
            if (this->settings.isTransparencyEnabled()) {
                this->detector->setShader(this->transparent);
            } else {
                this->detector->setShader(this->flat);
            }
        }
    }

    void Scene::render() {
        switch (this->stateManager.getCurrentState()) {
            case AppState::INTERACTION:
            case AppState::EXPORT_IMAGE:

                // SOLID MESHES RENDERING
                this->flat->use();
                glDepthMask(GL_TRUE);  // Write depth
                glDisable(GL_BLEND);   // Don't use transparency

                if (this->settings.isCameraPivotActive() && this->stateManager.getCurrentState() != AppState::EXPORT_IMAGE) {
                    this->pivot->render(*this->viewport, false);
                }

                this->hits->render(*this->viewport, false);

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
            auto hitMesh= std::unique_ptr<Object>(this->objectFactory.getHits(event));
            hitMesh->setShader(this->transparent);
            this->hits = std::move(hitMesh);
        }
    }

}
