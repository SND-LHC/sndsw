/**
 * @file Scene.hpp
 * @brief Manages the complete 3D scene hierarchy and rendering state.
 *
 * Scene is the central container for all 3D objects, lighting, and rendering state.
 * It maintains a scene graph of detector geometry, hit data visualization, lighting
 * configuration, and viewport management. Handles interaction with the state machine
 * to load event data and update visualization accordingly.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#pragma once

#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "ui/Gui.hpp"
#include "io/EventData.hpp"
#include "scene/Object.hpp"
#include "scene/PointLight.hpp"
#include "scene/ObjectFactory.hpp"
#include "scene/colors/ColorPalette.hpp"
#include "rendering/Viewport.hpp"
#include "rendering/AxisWidget.hpp"
#include "core/AppSettings.hpp"
#include "core/Constants.hpp"
#include "core/WindowManager.hpp"
#include "core/state/AppStateManager.hpp"

namespace snd3D {
    class Scene {

        // Must be able to access the camera
        friend class Callbacks;
        friend class Gui;

        public:
            Scene(WindowManager& winMan, AppStateManager& stateMan, AppSettings& appSettings, Gui& guiMan);
            void update();
            void render();
            void loadGeometry(std::string path);
            void setEvent(const EventData* event);

        private:
            std::unique_ptr<Viewport> viewport;
            std::unique_ptr<Object> detector;
            std::unique_ptr<Object> pivot;
            std::unique_ptr<Object> hits;
            std::unique_ptr<AxisWidget> axis;
            std::shared_ptr<Shader> flat;
            std::shared_ptr<Shader> phong;
            std::shared_ptr<Shader> flatMaterial;
            std::shared_ptr<Shader> transparent;
            std::shared_ptr<Shader> transparentPhong;
            std::vector<std::unique_ptr<PointLight>> lights{constants::graphics::lights::NUM};

            ObjectFactory objectFactory;
            std::unique_ptr<ColorPalette> colorPalette;

            // Owned by App, keep reference to spped up access
            WindowManager& windowManager;
            AppStateManager& stateManager;
            AppSettings& settings;
    };
}
