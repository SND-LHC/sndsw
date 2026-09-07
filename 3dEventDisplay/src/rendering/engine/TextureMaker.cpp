/**
 * @file TextureMaker.cpp
 * @brief Implementation of texture loading utilities.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-28
 */

#include "rendering/engine/TextureMaker.hpp"

#include <stb_image.h>
#include <stdexcept>

namespace snd3D {

    GLuint TextureMaker::loadTexture(std::string path, int flip) {
        GLuint textureId;

        glGenTextures(1, &textureId);

        int width, height, nrComponents;
        stbi_set_flip_vertically_on_load(flip);
        unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
        if (!data) {
            stbi_image_free(data);
            throw std::runtime_error("Texture failed to load at path: " + path);
        }

        GLenum format;

        switch (nrComponents) {
            case 1: format = GL_RED; break;
            case 3: format = GL_RGB; break;
            case 4: format = GL_RGBA; break;
            default:
                stbi_image_free(data);
                throw std::runtime_error("Invalid texture num components: " + std::to_string(nrComponents));
        }

        glBindTexture(GL_TEXTURE_2D, textureId);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);

        return textureId;
    }
}
