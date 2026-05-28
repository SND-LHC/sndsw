#include "core/Callbacks.hpp"

#include "core/App.hpp"
#include "core/Constants.hpp"
#include "core/state/AppState.hpp"

namespace snd3D {
    Callbacks::Callbacks(App& _app) : app(_app) {
    }

    /* ---------- STATIC FUNCTIONS ----------*/

    void Callbacks::keyAction(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Callbacks* callbackInstance = static_cast<Callbacks*>(glfwGetWindowUserPointer(window));
        if (callbackInstance) callbackInstance->keyAction(key, scancode, action, mods);
    }

    void Callbacks::cursorPosition(GLFWwindow* window, double currentMousePosX, double currentMousePosY) {
        Callbacks* callbackInstance = static_cast<Callbacks*>(glfwGetWindowUserPointer(window));
        if (callbackInstance) callbackInstance->cursorPosition(currentMousePosX, currentMousePosY);
    }

    void Callbacks::mouseButton(GLFWwindow * window, int button, int action, int mods) {
        Callbacks* callbackInstance = static_cast<Callbacks*>(glfwGetWindowUserPointer(window));
        if (callbackInstance) callbackInstance->mouseButton(button, action, mods);
    }

    void Callbacks::framebufferSize(GLFWwindow* window, int newWidth, int newHeight) {
        Callbacks* callbackInstance = static_cast<Callbacks*>(glfwGetWindowUserPointer(window));
        if (callbackInstance) callbackInstance->framebufferSize(newWidth, newHeight);
    }

    void Callbacks::scroll(GLFWwindow* window, double xOffset, double yOffset) {
        Callbacks* callbackInstance = static_cast<Callbacks*>(glfwGetWindowUserPointer(window));
        if (callbackInstance) callbackInstance->scroll(xOffset, yOffset);
    }

    void Callbacks::close(GLFWwindow* window) {
        Callbacks* callbackInstance = static_cast<Callbacks*>(glfwGetWindowUserPointer(window));
        if (callbackInstance) callbackInstance->close();
    }

    /* ---------- INSTANCE METHODS ----------*/

    void Callbacks::keyAction(int key, int scancode, int action, int mods) {
        if (this->app.guiManager->isKeyboardUsedByGui()) return;
        if (action != GLFW_PRESS && (key != GLFW_KEY_LEFT_SHIFT && key != GLFW_KEY_RIGHT_SHIFT)) return;

        bool interactionState = this->app.stateManager.getCurrentState() == AppState::INTERACTION;

        if (mods & GLFW_MOD_CONTROL) { // CTRL pressed
            switch (key) {

                case GLFW_KEY_KP_0:
                case GLFW_KEY_0:
                    if (interactionState) this->app.scene->viewport->setDirection(Camera::Directions::ISOMETRIC2);
                    break;

                case GLFW_KEY_KP_1:
                case GLFW_KEY_1:
                    if (interactionState) this->app.scene->viewport->setDirection(Camera::Directions::ALIGN_Z_NEG);
                    break;

                case GLFW_KEY_KP_3:
                case GLFW_KEY_3:
                    if (interactionState) this->app.scene->viewport->setDirection(Camera::Directions::ALIGN_X_NEG);
                    break;

                case GLFW_KEY_KP_7:
                case GLFW_KEY_7:
                    if (interactionState) this->app.scene->viewport->setDirection(Camera::Directions::ALIGN_Y_NEG);
                    break;

                case GLFW_KEY_KP_ADD:
                case GLFW_KEY_EQUAL:
                    this->app.guiManager->changeFontSize(constants::factors::GUI_FONT_RESIZE);
                    break;

                case GLFW_KEY_KP_SUBTRACT:
                case GLFW_KEY_MINUS:
                    this->app.guiManager->changeFontSize(1 / constants::factors::GUI_FONT_RESIZE);
                    break;

                case GLFW_KEY_E:
                    if (interactionState) this->app.stateManager.startEventChange();
                    break;

                case GLFW_KEY_G:
                    if (interactionState) this->app.stateManager.openGeometryDialog();
                    break;

                case GLFW_KEY_N:
                    if (interactionState) this->app.stateManager.startRunChange();
                    break;

                case GLFW_KEY_P:
                    if (interactionState) this->app.stateManager.toggleImageExport();
                    break;
            }
        }
        else if (mods == 0) { // No control keys are pressed
            switch (key) {
                case GLFW_KEY_F11:
                    this->app.windowManager->toggleFullScreen();
                    break;

                case GLFW_KEY_KP_0:
                case GLFW_KEY_0:
                    if (interactionState) this->app.scene->viewport->setDirection(Camera::Directions::ISOMETRIC1);
                    break;

                case GLFW_KEY_KP_1:
                case GLFW_KEY_1:
                    if (interactionState) this->app.scene->viewport->setDirection(Camera::Directions::ALIGN_Z);
                    break;

                case GLFW_KEY_KP_3:
                case GLFW_KEY_3:
                    if (interactionState) this->app.scene->viewport->setDirection(Camera::Directions::ALIGN_X);
                    break;

                case GLFW_KEY_KP_7:
                case GLFW_KEY_7:
                    if (interactionState) this->app.scene->viewport->setDirection(Camera::Directions::ALIGN_Y);
                    break;

                case GLFW_KEY_A:
                    if (interactionState) this->app.settings.toggleAxisWidget();
                    break;

                case GLFW_KEY_C:
                    if (interactionState) this->app.settings.toggleColorScale();
                    break;

                case GLFW_KEY_G:
                    if (interactionState) this->app.settings.toggleRenderOptions();
                    break;

                case GLFW_KEY_I:
                    if (interactionState) this->app.settings.toggleEventInfo();
                    break;

                case GLFW_KEY_J:
                    if (interactionState) this->app.stateManager.changeEvent(-1);
                    break;

                case GLFW_KEY_K:
                    if (interactionState) this->app.stateManager.changeEvent(1);
                    break;

                case GLFW_KEY_L:
                    if (interactionState) this->app.settings.toggleLighting();
                    break;

                case GLFW_KEY_P:
                    if (interactionState) this->app.scene->viewport->toggleProjectionType();
                    break;

                case GLFW_KEY_R:
                    if (interactionState) this->app.scene->viewport->reset();
                    break;

                case GLFW_KEY_S:
                    if (interactionState) this->app.settings.toggleSceneInspector();
                    break;

                case GLFW_KEY_T:
                    if (interactionState) this->app.settings.toggleTransparency();
                    break;

                case GLFW_KEY_V:
                    this->app.windowManager->toggleVsync();
                    break;

                case GLFW_KEY_X:
                    if (interactionState) this->app.settings.toggleCameraPivot();
                    break;
            }
        }

    }

    void Callbacks::cursorPosition(double currentMousePosX, double currentMousePosY) {
        // Moved cursor logic into viewport, so that not too many events are created that need to travel across the network
    }

    void Callbacks::mouseButton(int button, int action, int mods) {
        if (this->app.guiManager->isPointerUsedByGui() && action != GLFW_RELEASE) return; // Don't start new actions if the pointer is over the gui

        // Retrieve mouse position
        double xPos, yPos;
        glfwGetCursorPos(this->app.windowManager->getWindow(), &xPos, &yPos);

        switch (this->app.stateManager.getCurrentState()) {
            case AppState::INTERACTION:
                if (action == GLFW_PRESS) {
                    this->app.windowManager->lastMousePosition[0] = (int)xPos;
                    this->app.windowManager->lastMousePosition[1] = (int)yPos;
                }
                break;

            default:
                break;
        }
    }

    void Callbacks::framebufferSize(int newWidth, int newHeight) {
        this->app.windowManager->frameBufferChanged();
    }

    void Callbacks::scroll(double xOffset, double yOffset) {
        if (this->app.guiManager->isPointerUsedByGui()) return;

        switch (this->app.stateManager.getCurrentState()) {
            case AppState::INTERACTION:
                if (glfwGetKey(this->app.windowManager->getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                    glfwGetKey(this->app.windowManager->getWindow(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
                    this->app.scene->viewport->movePerpendicular(xOffset * constants::factors::PAN, yOffset * constants::factors::PAN);
                } else {
                    this->app.scene->viewport->zoom(yOffset);
                }
                break;

            default:
                break;
        }
    }

    void Callbacks::close() {
        this->app.stateManager.close();
    }
}
