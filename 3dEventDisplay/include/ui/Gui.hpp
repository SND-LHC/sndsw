/**
 * @file Gui.hpp
 * @brief ImGui interface for application settings and visualization controls.
 *
 * Gui manages the entire ImGui-based user interface including menus, dialogs,
 * settings panels, and visualization controls. Handles event loading, geometry
 * selection, rendering option toggles, and real-time scene inspection. Integrates
 * with ImGuiFileDialog for file browsing and provides visual feedback to the user
 * throughout the application lifecycle.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#pragma once

#include <string>
#include <cstdint>
#include <memory>

#include <imgui.h>

#include "scene/Node.hpp"
#include "rendering/engine/Texture.hpp"
#include "io/ClusterConfiguration.hpp"

namespace snd3D {
    class App;
    class Object;

    class Gui {
        public:
            Gui(App& _app, float _fontSize);
            ~Gui();
            void update();
            void render();
            void changeFontSize(float factor);
            bool isPointerUsedByGui();
            bool isKeyboardUsedByGui();

        private:
            App& app;
            float fontSize;
            float menuBarHeight = 0;
            int64_t runInputNumber = 0;     // Used as a buffer for run number input
            int64_t eventInputNumber = 0;   // Used as a buffer for event number input
            std::unique_ptr<Texture> logo;
            ImFont* italicFont;

            bool needsFocus = true;
            std::unique_ptr<ClusterConfiguration> clusterConfig; // Used as a buffer for cluster configuration

            void drawMenuBar();
            void drawInspector();
            void drawRenderOptions();
            void drawObjectTree(const std::string& label, Object* obj);
            void drawNodeTree(Node* node, bool parentActive);
            void drawRunDialog();
            void drawLoadingData();
            void drawEventDialog();
            void drawDefaultGeometryFailed();
            void drawGeometryFileDialog();
            void drawInitializationError();
            void drawEventDetails();
            void drawColorScale();
            void drawClusterConfiguration();
    };
}
