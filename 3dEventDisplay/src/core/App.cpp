#include "core/App.hpp"

#include <iostream>
#include <string>
#include <chrono>
#include <filesystem>

#include <glad/glad.h>
#include <stb_image_write.h>

#include "core/Constants.hpp"
#include "core/Callbacks.hpp"

namespace snd3D {

    App::App() {
        this->callbacks = std::make_unique<Callbacks>(*this);

        this->windowManager = std::make_unique<WindowManager>(
            snd3D::constants::defaults::window::WIDTH,
            snd3D::constants::defaults::window::HEIGHT,
            snd3D::constants::defaults::window::NAME,
            snd3D::constants::defaults::window::VSYNC,
            this->callbacks.get()
        );

        this->guiManager = std::make_unique<Gui>(*this, constants::sizes::GUI_FONT);
        this->scene = std::make_unique<Scene>(*this->windowManager, this->stateManager, this->settings, *this->guiManager);
    }

    void App::run() {

        // Game Loop
        while (this->stateManager.getCurrentState() != AppState::CLOSED) {
            glfwPollEvents();
            if (glfwWindowShouldClose(this->windowManager->getWindow())) this->stateManager.close();

            this->update();

            // Rendering
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            this->scene->render();
            this->guiManager->render();
            if (this->stateManager.getCurrentState() == AppState::EXPORT_IMAGE) this->exportImage();

            glfwSwapBuffers(this->windowManager->getWindow());
        }
    }

    void App::update() {

        this->stateManager.update();

        if (this->settings.isBackgroundColorChanged()) {
            this->windowManager->setBackgroundColor(this->settings.getBackgroundColor());
        }

        switch(this->stateManager.getCurrentState()) {
            case AppState::RUN_LOAD:
                try {
                    this->stateManager.runLoaded(
                        this->ioManager.loadRun(
                            this->stateManager.getPendingNumber()
                        )
                    );
                } catch (const std::exception& e) { this->stateManager.errorInitializing(e.what()); }
                break;

            case AppState::ROOT_GEOMETRY_LOAD:
                try {
                    this->ioManager.loadGeometry();
                } catch (const std::exception& e) { this->stateManager.errorInitializing(e.what()); }
                break;

            // When changing the run the geometry, configuation, ... also need to be reloaded
            case AppState::CHANGE_RUN_LOAD:
                try {
                    this->stateManager.runLoaded(
                        this->ioManager.loadRun(
                            this->stateManager.getPendingNumber()
                        )
                    );
                    this->ioManager.loadGeometry();
                } catch (const std::exception& e) { this->stateManager.errorInitializing(e.what()); }
                break;


            case AppState::EVENT_LOAD:
            case AppState::CHANGE_EVENT_LOAD:
                try {
                    this->stateManager.eventLoaded(
                        this->ioManager.loadEvent(
                            this->stateManager.getPendingNumber(),
                            this->stateManager.getClusterConfiguration()
                        )
                    );
                    this->scene->setEvent(this->stateManager.getEvent());
                } catch (const std::exception& e) { this->stateManager.errorInitializing(e.what()); }
                break;

            case AppState::DEFAULT_GEOMETRY_LOAD:
            case AppState::USER_GEOMETRY_LOAD:
            case AppState::CHANGE_GEOMETRY_LOAD:
                try {
                    this->scene->loadGeometry(this->stateManager.getDetectorPath());
                    this->stateManager.geometryLoaded();
                } catch (...) {
                    this->stateManager.errorInitializing();
                }
                break;

            case AppState::INTERACTION:
                this->scene->update();
                break;

            default:
                break;
        }

        this->guiManager->update();
    }

    void App::exportImage() {
        int width, height;
        glfwGetFramebufferSize(this->windowManager->getWindow(), &width, &height);

        int nrChannels = 3;
        int stride = width * nrChannels;
        std::vector<unsigned char> buffer(stride * height);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        // Read data written until now in the Back Buffer
        glReadBuffer(GL_BACK);
        glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, buffer.data());

        stbi_flip_vertically_on_write(true);

        auto epochTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        auto path = std::string(constants::paths::SCREENSHOT) + std::to_string(epochTime) + ".png";

        try {
            if (!std::filesystem::exists(constants::paths::SCREENSHOT)) {
                if (std::filesystem::create_directory(constants::paths::SCREENSHOT)) {
                    std::cout << "Created output folder\n";
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "System error: " << e.what() << "\n";
        }

        if (stbi_write_png(path.c_str(), width, height, nrChannels, buffer.data(), stride)) {
            std::cout << "Screenshot saved as " << path << std::endl;
        } else {
            std::cerr << "Error saving image" << std::endl;
        }

        this->stateManager.toggleImageExport();
    }
}
