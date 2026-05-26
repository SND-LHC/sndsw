#pragma once

#include <memory>

#include "core/state/AppStateManager.hpp"
#include "io/SndswEventManager.hpp"
#include "core/WindowManager.hpp"
#include "core/AppSettings.hpp"
#include "scene/Scene.hpp"
#include "ui/Gui.hpp"

namespace snd3D {
    class App {

        // They need access to WindowManager and Scene
        friend class Callbacks;
        friend class Gui;

        public:
            App();
            void run();

        private:
            AppSettings settings;
            AppStateManager stateManager;
            std::unique_ptr<Callbacks> callbacks;
            std::unique_ptr<Gui> guiManager;
            std::unique_ptr<WindowManager> windowManager;
            std::unique_ptr<Scene> scene;
            SndswEventManager ioManager;
            float lastFrame = 0; // Used to limit FPS

            void update();
            void exportImage();
    };
}
