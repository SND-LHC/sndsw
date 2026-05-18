#include "core/AppSettings.hpp"

#include <glm/glm.hpp>

namespace snd3D {
    bool AppSettings::isCameraPivotActive() {
        return this->cameraPivotActive;
    }

    void AppSettings::toggleCameraPivot() {
        this->cameraPivotActive = !this->cameraPivotActive;
    }

    bool AppSettings::isAxisWidgetActive() {
        return this->axisWidgetActive;
    }

    void AppSettings::toggleAxisWidget() {
        this->axisWidgetActive = !this->axisWidgetActive;
    }

    bool AppSettings::isSceneInspectorActive() {
        return this->sceneInspectorActive;
    }

    void AppSettings::toggleSceneInspector() {
        this->sceneInspectorActive = !this->sceneInspectorActive;
    }

    bool AppSettings::isRenderOptionsActive() {
        return this->renderOptionsActive;
    }

    void AppSettings::toggleRenderOptions() {
        this->renderOptionsActive = !this->renderOptionsActive;
    }

    bool AppSettings::isEventInfoActive() {
        return this->eventInfoActive;
    }

    void AppSettings::toggleEventInfo() {
        this->eventInfoActive = !this->eventInfoActive;
    }

    bool AppSettings::isClusterConfigInfoActive() {
        return this->clusterConfigInfoActive;
    }

    void AppSettings::toggleClusterConfigInfo() {
        this->clusterConfigInfoActive = !this->clusterConfigInfoActive;
    }

    bool AppSettings::isColorScaleActive() {
        return this->colorScaleActive;
    }

    void AppSettings::toggleColorScale() {
        this->colorScaleActive = !this->colorScaleActive;
    }

    bool AppSettings::isTransparencyEnabled() {
        return this->transparency;
    }

    bool AppSettings::isTransparencyChanged() {
        bool tmp = this->transparencyChanged;
        this->transparencyChanged = false;
        return tmp;
    }

    void AppSettings::toggleTransparency() {
        this->transparency = !this->transparency;
        this->transparencyChanged = true;
    }

    float AppSettings::getEdgeAlphaValue() {
        return this->edgeAlphaValue;
    }

    void AppSettings::setEdgeAlphaValue(const float alpha) {
        this->edgeAlphaValue = glm::clamp(alpha, constants::limits::ALPHA_VALUE_MIN, constants::limits::ALPHA_VALUE_MAX);
    }

    float AppSettings::getFaceAlphaValue() {
        return this->faceAlphaValue;
    }

    void AppSettings::setFaceAlphaValue(const float alpha) {
        this->faceAlphaValue = glm::clamp(alpha, constants::limits::ALPHA_VALUE_MIN, constants::limits::ALPHA_VALUE_MAX);
    }

    float AppSettings::getEdgeThickness() {
        return this->edgeThickness;
    }

    void AppSettings::setEdgeThickness(const float thickness) {
        this->edgeThickness = glm::clamp(thickness, constants::limits::EDGE_THICKNESS_MIN, constants::limits::EDGE_THICKNESS_MAX);
    }

    void AppSettings::setBackgroundColor(const glm::vec3& newColor) {
        this->backgroundColor = newColor;
        this->backgroundColorChanged = true;
    }
    
    glm::vec3 AppSettings::getBackgroundColor() {
        return this->backgroundColor;
    }

    bool AppSettings::isBackgroundColorChanged() {
        bool tmp = this->backgroundColorChanged;
        this->backgroundColorChanged = false;
        return tmp;
    }

    void AppSettings::setColorVariable(ColorVariable variable) {
        this->colorVariable = variable; 
    }
    
    ColorVariable AppSettings::getColorVariable() {
        return this->colorVariable;
    }

    void AppSettings::setColorScalingMode(ColorScalingMode mode) {
        this->colorScalingMode = mode;
    }

    ColorScalingMode AppSettings::getColorScalingMode() {
        return this->colorScalingMode;
    }
}
