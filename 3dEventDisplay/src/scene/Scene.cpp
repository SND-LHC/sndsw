#include "scene/Scene.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "scene/colors/ValueMapper.hpp"
#include "scene/colors/LinearValueMapper.hpp"
#include "scene/colors/LogarithmicValueMapper.hpp"
#include "scene/colors/FixedValueMapper.hpp"
#include "scene/colors/VariableGetter.hpp"
#include "scene/colors/EnergyVariableGetter.hpp"
#include "scene/colors/TimeVariableGetter.hpp"

namespace snd3D {

    Scene::Scene(WindowManager& winMan, AppStateManager& stateMan, AppSettings& appSettings, Gui& guiMan) :
        windowManager(winMan), stateManager(stateMan), settings(appSettings) {

        this->viewport = std::make_unique<Viewport>(winMan, guiMan, constants::defaults::ORTHOGRAPHIC_PROJECTION);
        this->flat = std::make_shared<Shader>("Flat", "flat.vert", "flat.frag");
        this->phong = std::make_shared<Shader>("Phong", "phong.vert", "phong.frag");
        this->flatMaterial = std::make_shared<Shader>("Flat Material", "flat_material.vert", "flat.frag");
        this->transparent = std::make_shared<Shader>("Transparent", "transparent.vert", "transparent.frag", "transparent.geom");
        this->transparentPhong = std::make_shared<Shader>("Transparent Phong", "transparent_phong.vert", "transparent_phong.frag", "transparent_phong.geom");
        this->pivot = std::unique_ptr<Object>(this->objectFactory.getSphere());
        this->pivot->setShader(this->settings.isLightingEnabled() ? this->phong : this->flat);
        this->axis = std::make_unique<AxisWidget>(this->flat, constants::defaults::AXIS_WIDGET_SIZE, constants::defaults::AXIS_WIDGET_MARGIN);
        this->lights[constants::graphics::lights::pointlight1::ID] = std::make_unique<PointLight>(
                glm::vec3(constants::graphics::lights::pointlight1::POS_X, constants::graphics::lights::pointlight1::POS_Y, constants::graphics::lights::pointlight1::POS_Z),
                glm::vec3(constants::graphics::lights::pointlight1::COLOR_R, constants::graphics::lights::pointlight1::COLOR_G, constants::graphics::lights::pointlight1::COLOR_B),
                constants::graphics::lights::pointlight1::POWER
        );
        this->lights[constants::graphics::lights::pointlight2::ID] = std::make_unique<PointLight>(
                glm::vec3(constants::graphics::lights::pointlight2::POS_X, constants::graphics::lights::pointlight2::POS_Y, constants::graphics::lights::pointlight2::POS_Z),
                glm::vec3(constants::graphics::lights::pointlight2::COLOR_R, constants::graphics::lights::pointlight2::COLOR_G, constants::graphics::lights::pointlight2::COLOR_B),
                constants::graphics::lights::pointlight2::POWER
        );
        this->lights[constants::graphics::lights::camera::ID] = std::make_unique<PointLight>(
            this->viewport->getCameraPosition(),
            glm::vec3(constants::graphics::lights::camera::COLOR_R, constants::graphics::lights::camera::COLOR_G, constants::graphics::lights::camera::COLOR_B),
            constants::graphics::lights::camera::POWER
        );
    }

    void Scene::update() {
        this->viewport->update(); // Updates camera and projection

        if (this->viewport->isCameraChanged()) {
            glm::mat4 matrix = glm::translate(glm::mat4(1.0f), this->viewport->getCameraTarget());
            matrix = glm::scale(matrix, glm::vec3(constants::sizes::PIVOT));
            this->pivot->updateModelMatrix(matrix);
            this->lights[constants::graphics::lights::camera::ID]->setPosition(this->viewport->getCameraPosition());
            this->detector->sortMeshes(this->viewport->getCameraPosition());
        }

        if (glfwGetKey(this->windowManager.getWindow(), GLFW_KEY_LEFT) == GLFW_PRESS)  this->viewport->rotateByAngles(-constants::factors::ROTATION_SPEED, 0);
        if (glfwGetKey(this->windowManager.getWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS) this->viewport->rotateByAngles(constants::factors::ROTATION_SPEED, 0);
        if (glfwGetKey(this->windowManager.getWindow(), GLFW_KEY_UP) == GLFW_PRESS)  this->viewport->rotateByAngles(0, constants::factors::ROTATION_SPEED);
        if (glfwGetKey(this->windowManager.getWindow(), GLFW_KEY_DOWN) == GLFW_PRESS) this->viewport->rotateByAngles(0, -constants::factors::ROTATION_SPEED);

        if (this->settings.isTransparencyChanged()) {
            if (this->detector.get() != nullptr) {
                if (this->settings.isTransparencyEnabled()) {
                    this->detector->setShader(this->settings.isLightingEnabled() ? this->transparentPhong : this->transparent);
                } else {
                    this->detector->setShader(this->settings.isLightingEnabled() ? this->phong : this->flat);
                }
            }
        }
        if (this->settings.isLightingChanged()) {
            if (this->hits.get() != nullptr) this->hits->setShader(this->settings.isLightingEnabled() ? this->phong : this->flatMaterial);
            if (this->pivot.get() != nullptr) this->pivot->setShader(this->settings.isLightingEnabled() ? this->phong : this->flat);
            if (this->detector.get() != nullptr) {
                if (this->settings.isTransparencyEnabled()) {
                    this->detector->setShader(this->settings.isLightingEnabled() ? this->transparentPhong : this->transparent);
                } else {
                    this->detector->setShader(this->settings.isLightingEnabled() ? this->phong : this->flat);
                }
            }
        }
    }

    void Scene::render() {
        switch (this->stateManager.getCurrentState()) {
            case AppState::INTERACTION:
            case AppState::EXPORT_IMAGE:

                // SOLID MESHES RENDERING
                glDepthMask(GL_TRUE);  // Write depth
                glDisable(GL_BLEND);   // Don't use transparency

                if (this->settings.isCameraPivotActive() && this->stateManager.getCurrentState() != AppState::EXPORT_IMAGE) {
                    this->pivot->render(*this->viewport, false, this->lights);
                }

                this->hits->render(*this->viewport, false, this->lights);

                // TRANSPARENT MESHES RENDERING
                // The transparency function is set in the OpenGL initialization: GL_ONE_MINUS_SRC_ALPHA
                if (this->settings.isTransparencyEnabled()) {
                    glEnable(GL_BLEND);     // Use transparency
                    glDepthMask(GL_FALSE);  // Don't write on the depth-buffer, otherwise further away meshes won't be rendered
                }

                this->detector->renderBuffered(*this->viewport, false, this->lights, this->settings.getEdgeAlphaValue(), this->settings.getFaceAlphaValue(), this->settings.getEdgeThickness());

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
            this->detector->setShader(this->settings.isLightingEnabled() ? this->transparentPhong : this->transparent);
        } else {
            this->detector->setShader(this->settings.isLightingEnabled() ? this->phong : this->flat);
        }
    }

    void Scene::setEvent(const EventData* event) {
        if (event != nullptr) {

            VariableGetter* variableGetter;
            ValueMapper* valueMapper;

            switch (this->settings.getColorVariable()) {
                case ColorVariable::ENERGY:
                    variableGetter = new EnergyVariableGetter(event);
                    break;

                case ColorVariable::TIME:
                    variableGetter = new TimeVariableGetter(event);
                    break;
            }

            switch (this->settings.getColorScalingMode()) {
                case ColorScalingMode::LOGARITHMIC:
                    valueMapper = new LogarithmicValueMapper(
                        glm::vec3(constants::defaults::colors::HIT_ENERGY_MIN_R, constants::defaults::colors::HIT_ENERGY_MIN_G, constants::defaults::colors::HIT_ENERGY_MIN_B),
                        glm::vec3(constants::defaults::colors::HIT_ENERGY_MAX_R, constants::defaults::colors::HIT_ENERGY_MAX_G, constants::defaults::colors::HIT_ENERGY_MAX_B)
                    );
                    break;

                case ColorScalingMode::LINEAR:
                    valueMapper = new LinearValueMapper(
                        glm::vec3(constants::defaults::colors::HIT_ENERGY_MIN_R, constants::defaults::colors::HIT_ENERGY_MIN_G, constants::defaults::colors::HIT_ENERGY_MIN_B),
                        glm::vec3(constants::defaults::colors::HIT_ENERGY_MAX_R, constants::defaults::colors::HIT_ENERGY_MAX_G, constants::defaults::colors::HIT_ENERGY_MAX_B)
                    );
                    break;

                case ColorScalingMode::FIXED:
                    valueMapper = new FixedValueMapper(
                        glm::vec3(constants::defaults::colors::HIT_ENERGY_MAX_R, constants::defaults::colors::HIT_ENERGY_MAX_G, constants::defaults::colors::HIT_ENERGY_MAX_B)
                    );
                    break;
            }

            this->colorPalette = std::make_unique<ColorPalette>(variableGetter, valueMapper);

            auto hitsObject = std::unique_ptr<Object>(this->objectFactory.getHits(event, this->colorPalette));
            hitsObject->setShader(this->settings.isLightingEnabled() ? this->phong : this->flatMaterial);
            this->hits = std::move(hitsObject);
        }
    }

}
