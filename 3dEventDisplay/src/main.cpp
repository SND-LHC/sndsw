/**
 * @file main.cpp
 * @brief Application entry point.
 *
 * Initializes the application object and enters the main event loop.
 * Handles top-level exception catching and graceful error reporting.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#include <cstdlib>
#include <iostream>

#include "core/App.hpp"

int main() {

    std::cout << "Starting 3dEventDisplay" << std::endl;

    try {
        snd3D::App app;
        app.run();
    }
    catch (const std::exception& ex) {
        std::cerr << "An Exception Occurred:\n" << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
