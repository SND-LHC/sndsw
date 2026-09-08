/**
 * @file WindowManager.cpp
 * @brief Implementation of window management using GLFW.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#include "core/WindowManager.hpp"

#include <stdexcept>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "core/Callbacks.hpp"
#include "core/Constants.hpp"

namespace snd3D {

    WindowManager::WindowManager(int width, int height, const char* title, bool enableVsync, Callbacks* callbacksHandler) {
        // Init GLFW
        if (!glfwInit()) {
            throw std::runtime_error("GLFW initialization error");
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

        this->window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (!this->window) {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }
        glfwMakeContextCurrent(this->window);
        glfwSwapInterval(enableVsync ? 1 : 0);
        this->vsync = enableVsync;

        // Init GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwDestroyWindow(this->window);
            glfwTerminate();
            throw std::runtime_error("GLAD initialization error");
        }

        // Bind callbacks
        glfwSetWindowUserPointer(this->window, callbacksHandler); // Save the callbacks object instance into the window
        //glfwSetCursorPosCallback(this->window, Callbacks::cursorPosition); Using polling so no flood of events is generated
        glfwSetKeyCallback(this->window, Callbacks::keyAction);
        glfwSetMouseButtonCallback(this->window, Callbacks::mouseButton);
        glfwSetFramebufferSizeCallback(this->window, Callbacks::framebufferSize);
        glfwSetScrollCallback(this->window, Callbacks::scroll);
        glfwSetWindowCloseCallback(this->window, Callbacks::close);

        // Enable transparency management and depth test
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Function to mix overlapping colors
        glEnable(GL_DEPTH_TEST);

        // Set base color
        glClearColor(constants::defaults::colors::BACKGROUND_R, constants::defaults::colors::BACKGROUND_G, constants::defaults::colors::BACKGROUND_B, 1.0f);

        // Set polygon mode
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    WindowManager::~WindowManager() {
        glfwDestroyWindow(this->window);
        glfwTerminate();
    }

    GLFWwindow* WindowManager::getWindow() {
        return this->window;
    }

    bool WindowManager::isVsyncActive() {
        return this->vsync;
    }

    void WindowManager::toggleVsync() {
        this->vsync = !this->vsync;
        glfwMakeContextCurrent(this->window);
        glfwSwapInterval(this->vsync ? 1 : 0);
    }

    bool WindowManager::isFullScreen() {
        return this->fullScreen;
    }

    void WindowManager::toggleFullScreen() {
        if (this->fullScreen) {
            // Restore last positioning
            glfwSetWindowMonitor(this->window, nullptr, this->lastWindowPosition[0], this->lastWindowPosition[1], this->lastWindowSize[0], this->lastWindowSize[1], GLFW_DONT_CARE);
        } else {
            // Backup old positioning
            glfwGetWindowPos(this->window, &this->lastWindowPosition[0], &this->lastWindowPosition[1]);
            glfwGetWindowSize(this->window, &this->lastWindowSize[0], &this->lastWindowSize[1]);

            // Select the monitor to use as fullscreen and get the size
            GLFWmonitor* currentMonitor = this->getCurrentMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(currentMonitor);

            // Switch to full screen
            glfwSetWindowMonitor(this->window, currentMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);

            // Force vsync when enabling fullscreen
            this->vsync = true;
            glfwMakeContextCurrent(this->window);
            glfwSwapInterval(1);
        }
        this->fullScreen = !this->fullScreen;
    }

    void WindowManager::frameBufferChanged() {
        // We want to use the full screen but mantaining the proportions
        glm::vec2 res = this->getCurrentResolution();
        glViewport(0, 0, (int)res.x, (int)res.y);
        this->framebufferChanged = true;
    }

    bool WindowManager::isFramebufferChanged() {
        bool tmp = this->framebufferChanged;
        this->framebufferChanged = false;
        return tmp;
    }

    glm::vec2 WindowManager::getCurrentResolution() {
        int width, height;
        glfwGetWindowSize(this->window, &width, &height);
        return glm::vec2(width, height);
    }

    float WindowManager::getAspectRatio() {
        glm::vec2 res = this->getCurrentResolution();
        return res.x / res.y;
    }

    GLFWmonitor* WindowManager::getCurrentMonitor() {
        int nmonitors, i;
        int wx, wy, ww, wh;
        int mx, my, mw, mh;
        int overlap, bestoverlap;
        GLFWmonitor* bestmonitor;
        GLFWmonitor** monitors;
        const GLFWvidmode* mode;

        bestoverlap = 0;
        bestmonitor = NULL;

        glfwGetWindowPos(this->window, &wx, &wy);
        glfwGetWindowSize(this->window, &ww, &wh);
        monitors = glfwGetMonitors(&nmonitors);

        for (i = 0; i < nmonitors; i++) {
            mode = glfwGetVideoMode(monitors[i]);
            glfwGetMonitorPos(monitors[i], &mx, &my);
            mw = mode->width;
            mh = mode->height;

            overlap =
                std::max(0, std::min(wx + ww, mx + mw) - std::max(wx, mx)) *
                std::max(0, std::min(wy + wh, my + mh) - std::max(wy, my));

            if (bestoverlap < overlap) {
                bestoverlap = overlap;
                bestmonitor = monitors[i];
            }
        }

        return bestmonitor != NULL ? bestmonitor : glfwGetPrimaryMonitor();
    }

    void WindowManager::setBackgroundColor(const glm::vec3& newColor) {
        glClearColor(newColor.r, newColor.g, newColor.b, 1.0f);
    }
}
