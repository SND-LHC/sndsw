/**
 * @file WindowManager.hpp
 * @brief Manages GLFW window creation, configuration, and framebuffer events.
 *
 * WindowManager encapsulates all GLFW window operations including creation,
 * context management, fullscreen toggling, VSync control, and resolution tracking.
 * Handles framebuffer resize events and provides access to the underlying GLFWwindow.
 * Integrates with the Callbacks system for event handling.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#pragma once

#define GLFW_INCLUDE_NONE // Include only GLFW data types
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "core/Callbacks.hpp"

namespace snd3D {
    class WindowManager {
        public:
            WindowManager(int width, int height, const char* title, bool enableVsync, Callbacks* callbacksHandler);
            ~WindowManager();
            GLFWwindow* getWindow();
            bool isVsyncActive();
            void toggleVsync();
            bool isFullScreen();
            void toggleFullScreen();
            void frameBufferChanged();
            bool isFramebufferChanged();
            glm::vec2 getCurrentResolution();
            float getAspectRatio();
            void setBackgroundColor(const glm::vec3& newColor);
            int lastMousePosition[2];

        private:
            GLFWwindow* window;
            bool framebufferChanged = false, vsync, fullScreen = false;
            int lastWindowPosition[2] = {0, 0}, lastWindowSize[2] = {0, 0}; // Used to toggle between full screen and windowed view

            GLFWmonitor* getCurrentMonitor();
    };
}
