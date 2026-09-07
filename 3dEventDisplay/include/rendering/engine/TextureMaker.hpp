/**
 * @file TextureMaker.hpp
 * @brief Utility class for loading texture images from disk into GPU memory.
 *
 * TextureMaker provides a static factory method for loading image files (using stb_image)
 * and uploading them to OpenGL texture objects with appropriate filtering and wrapping.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-28
 */

#pragma once

#include <string>
#include <glad/glad.h>

namespace snd3D {
    class TextureMaker {
        public:
            static GLuint loadTexture(std::string path, int flip = 0);

        private:
            TextureMaker() { }
    };
}
