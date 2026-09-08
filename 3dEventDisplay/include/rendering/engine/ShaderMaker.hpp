/**
 * @file ShaderMaker.hpp
 * @brief Utility class for compiling and linking OpenGL shader programs.
 *
 * ShaderMaker provides static methods for reading shader source files from disk,
 * compiling individual shader stages (vertex, fragment, geometry), and linking
 * them into complete OpenGL programs. Handles error reporting during compilation.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#pragma once

#include <string>

#include <glad/glad.h>

namespace snd3D {
    class ShaderMaker {
        public:
            static GLuint createProgram(std::string vertexfilename, std::string fragmentfilename, std::string geometryfilename = "");
            static char* readShaderSource(std::string shaderFile);

        private:
            ShaderMaker() {}
    };
}
