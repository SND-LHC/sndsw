/**
 * @file Texture.hpp
 * @brief Manages GPU texture resources loaded from image files.
 *
 * Texture encapsulates OpenGL texture objects created from image files using stb_image.
 * Tracks texture metadata (name, dimensions) and provides access to the OpenGL handle
 * for binding during rendering.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-28
 */

#pragma once

#include <string>
#include <vector>
#include <glad/glad.h>

namespace snd3D {
    class Texture {
        public:
            Texture(std::string _name, std::string path);
            ~Texture();
            std::string getName();
            void setName(std::string _name);
            GLuint getProgramId();
            int getWidth();
            int getHeight();

        private:
            std::string name;
            GLuint programId;
            int width, height;
    };
}
