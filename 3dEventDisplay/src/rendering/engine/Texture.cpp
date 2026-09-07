/**
 * @file Texture.cpp
 * @brief Implementation of GPU texture resource management.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-28
 */

#include "rendering/engine/Texture.hpp"

#include <stb_image.h>

#include "rendering/engine/TextureMaker.hpp"

namespace snd3D {

    Texture::Texture(std::string _name, std::string path) {
        this->programId = TextureMaker::loadTexture(path);
        this->name = std::string(_name);
        int channels;
        stbi_info(path.c_str(), &this->width, &this->height, &channels);
    }

    Texture::~Texture() {
        glDeleteTextures(1, &this->programId);
    }

    std::string Texture::getName() {
        return name;
    }

    void Texture::setName(std::string _name) {
        this->name = _name;
    }

    GLuint Texture::getProgramId() {
        return programId;
    }

    int Texture::getWidth() {
        return this->width;
    }

    int Texture::getHeight() {
        return this->height;
    }
}
