/**
 * @file ShaderMaker.cpp
 * @brief Implementation of shader compilation and linking utilities.
 *
 * @author Enrico Bartocetti
 * @date 2026-04-16
 */

#include "rendering/engine/ShaderMaker.hpp"

#include <stdexcept>

namespace snd3D {
    #pragma warning(disable:4996)
    char* ShaderMaker::readShaderSource(std::string shaderFile) {
        FILE* fp = fopen(shaderFile.c_str(), "rb");

        if (fp == NULL) {
            throw std::runtime_error("ERROR READING SHADER FILE.\nFile not found:\n" + shaderFile);
        }

        fseek(fp, 0L, SEEK_END);
        long size = ftell(fp);

        fseek(fp, 0L, SEEK_SET);
        char* buf = new char[size + 1];
        fread(buf, 1, size, fp);

        buf[size] = '\0';
        fclose(fp);

        return buf;
    }

    GLuint ShaderMaker::createProgram(std::string vertexfilename, std::string fragmentfilename, std::string geometryfilename) {
        int success;
        char infoLog[512];

        // --- VERTEX SHADER ---
        GLchar* vertexSource = readShaderSource(vertexfilename); // Read vertex shader source
        GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER); // Generate an ID for the vertex shader
        glShaderSource(vertexShaderId, 1, (const char**)&vertexSource, NULL); // Associate the shader source with the shader object
        glCompileShader(vertexShaderId); // Compile the vertex shader

        glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
            throw std::runtime_error("ERROR::VERTEX::COMPILATION_FAILED\n" + std::string(infoLog));
        }

        // --- GEOMETRY SHADER ---
        GLuint geometryShaderId = 0;
        if (!geometryfilename.empty()) {
            GLchar* geometrySource = readShaderSource(geometryfilename);
            geometryShaderId = glCreateShader(GL_GEOMETRY_SHADER);
            glShaderSource(geometryShaderId, 1, (const char**)&geometrySource, NULL);
            glCompileShader(geometryShaderId);

            glGetShaderiv(geometryShaderId, GL_COMPILE_STATUS, &success);
            if (!success) {
                glGetShaderInfoLog(geometryShaderId, 512, NULL, infoLog);
                throw std::runtime_error("ERROR::GEOMETRY::COMPILATION_FAILED\n" + std::string(infoLog));
            }

            delete[] geometrySource;
        }

        // --- FRAGMENT SHADER ---
        GLchar* fragmentSource = readShaderSource(fragmentfilename);
        GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShaderId, 1, (const char**)&fragmentSource, NULL);
        glCompileShader(fragmentShaderId);

        glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fragmentShaderId, 512, NULL, infoLog);
            throw std::runtime_error("ERROR::FRAGMENT::COMPILATION_FAILED\n" + std::string(infoLog));
        }

        // SHADER CREATION
        GLuint programId = glCreateProgram();
        glAttachShader(programId, vertexShaderId);
        if (geometryShaderId != 0) glAttachShader(programId, geometryShaderId);
        glAttachShader(programId, fragmentShaderId);

        // LINKING
        glLinkProgram(programId);
        glGetProgramiv(programId, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(programId, 512, NULL, infoLog);
            throw std::runtime_error("ERROR::PROGRAM::LINKING_FAILED\n" + std::string(infoLog));
        }

        // CLEANUP
        glDeleteShader(vertexShaderId); // Clean up shader objects after linking it to the program
        glDeleteShader(fragmentShaderId);
        if (geometryShaderId != 0) glDeleteShader(geometryShaderId);

        delete[] vertexSource;
        delete[] fragmentSource;

        return programId;
    }
}
