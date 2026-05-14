#pragma once

#include <glm/glm.hpp>

#include "core/Constants.hpp"
#include "scene/colors/Modes.hpp"

namespace snd3D {
    class AppSettings {
        public:
            bool isCameraPivotActive();
            void toggleCameraPivot();
            bool isAxisWidgetActive();
            void toggleAxisWidget();
            bool isSceneInspectorActive();
            void toggleSceneInspector();
            bool isRenderOptionsActive();
            void toggleRenderOptions();
            bool isEventInfoActive();
            void toggleEventInfo();
            bool isColorScaleActive();
            void toggleColorScale();
            bool isTransparencyEnabled();
            bool isTransparencyChanged();
            void toggleTransparency();
            float getEdgeAlphaValue();
            void setEdgeAlphaValue(const float alpha);
            float getFaceAlphaValue();
            void setFaceAlphaValue(const float alpha);
            float getEdgeThickness();
            void setEdgeThickness(const float thickness);
            void setBackgroundColor(const glm::vec3& newColor);
            glm::vec3 getBackgroundColor();
            bool isBackgroundColorChanged();
            void setColorVariable(ColorVariable variable);
            ColorVariable getColorVariable();
            void setColorScalingMode(ColorScalingMode mode);
            ColorScalingMode getColorScalingMode();

        private:
            bool cameraPivotActive = constants::defaults::SHOW_PIVOT;
            bool axisWidgetActive = constants::defaults::SHOW_AXIS;
            bool sceneInspectorActive = true;
            bool renderOptionsActive = true;
            bool eventInfoActive = true;
            bool colorScaleActive = true;
            bool transparency = constants::defaults::TRANSPARENCY, transparencyChanged = true;
            float edgeAlphaValue = constants::defaults::EDGE_ALPHA_VALUE;
            float faceAlphaValue = constants::defaults::FACE_ALPHA_VALUE;
            float edgeThickness = constants::defaults::EDGE_THICKNESS;
            glm::vec3 backgroundColor = glm::vec3(constants::defaults::colors::BACKGROUND_R, constants::defaults::colors::BACKGROUND_G, constants::defaults::colors::BACKGROUND_B);
            bool backgroundColorChanged = true;
            ColorVariable colorVariable = ColorVariable::ENERGY;
            ColorScalingMode colorScalingMode = ColorScalingMode::LINEAR;
    };
}
