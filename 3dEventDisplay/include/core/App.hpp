/**
 * @file App.hpp
 * @brief Main application class that manages the overall 3D event display application.
 *
 * The App class serves as the central orchestrator for the entire 3D visualization system.
 * It initializes and manages the window, GUI, scene, state machine, and event I/O components.
 * Acts as a bridge between the rendering engine and the event data management system.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

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
