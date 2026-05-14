#pragma once

#include <string>

#include <glm/glm.hpp>

#include "scene/Object.hpp"
#include "rendering/Viewport.hpp"
#include "core/AppSettings.hpp"
#include "core/WindowManager.hpp"
#include "core/state/AppStateManager.hpp"
#include "scene/ObjectFactory.hpp"
#include "rendering/AxisWidget.hpp"
#include "io/EventData.hpp"
#include "ui/Gui.hpp"
#include "scene/colors/ColorPalette.hpp"

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
            std::shared_ptr<Shader> flatMaterial;
            std::shared_ptr<Shader> transparent;

            ObjectFactory objectFactory;
            std::unique_ptr<ColorPalette> colorPalette;

            // Owned by App, keep reference to spped up access
            WindowManager& windowManager;
            AppStateManager& stateManager;
            AppSettings& settings;
    };
}
