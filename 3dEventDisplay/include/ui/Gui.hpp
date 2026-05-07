#pragma once

#include <string>
#include <cstdint>
#include <memory>

#include "scene/Node.hpp"
#include "rendering/engine/Texture.hpp"

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

        private:
            App& app;
            float fontSize;
            float menuBarHeight = 0;
            int64_t runInputNumber = 0;     // Used as a buffer for run number input
            int64_t eventInputNumber = 0;   // Used as a buffer for event number input
            std::unique_ptr<Texture> logo;
            bool needsFocus = true;

            void drawMenuBar();
            void drawInspector();
            void drawRenderOptions();
            void drawObjectTree(const std::string& label, Object* obj);
            void drawNodeTree(Node* node);
            void drawRunDialog();
            void drawLoadingData();
            void drawEventDialog();
            void drawDefaultGeometryFailed();
            void drawGeometryFileDialog();
            void drawInitializationError();
            void drawEventDetails();
    };
}
