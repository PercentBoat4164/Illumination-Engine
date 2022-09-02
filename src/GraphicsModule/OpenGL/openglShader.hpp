#pragma once

#include <fstream>
#include <sstream>
#include <deque>
#include <functional>
#include <iostream>

#ifndef GLEW_IMPLEMENTATION
#define GLEW_IMPLEMENTATION
#include <GL/glew.h>
#endif

class OpenGLShader {
public:
    struct CreateInfo {
        //Required
        const char *filename{};

        //Recommended - will be filled in based on shader file extension if not provided. This will fail if no extension is recognized.
        GLenum shaderType{};
    };

    CreateInfo createdWith{};
    std::string data{};
    uint32_t ID{};
    bool compiled{false};
    std::string oldData{};

    void create(CreateInfo *createInfo) {
        createdWith = *createInfo;
        if (createdWith.shaderType == 0) {
            std::string filenameString{createdWith.filename};
            std::string extension = filenameString.substr(filenameString.find_last_of('.') + 1, 4);
            if (extension == "vert") { createdWith.shaderType = GL_VERTEX_SHADER; }
            if (extension == "tesc") { createdWith.shaderType = GL_TESS_CONTROL_SHADER; }
            if (extension == "tese") { createdWith.shaderType = GL_TESS_EVALUATION_SHADER; }
            if (extension == "geom") { createdWith.shaderType = GL_GEOMETRY_SHADER; }
            if (extension == "frag") { createdWith.shaderType = GL_FRAGMENT_SHADER; }
            if (extension == "comp") { createdWith.shaderType = GL_COMPUTE_SHADER; }
        }
        std::ifstream file(createdWith.filename, std::ios::in);
        if (!file.is_open()) { throw std::runtime_error("failed to prepare shader: " + static_cast<std::string>(createdWith.filename)); }
        std::stringstream shaderCode{};
        shaderCode << file.rdbuf();
        file.close();
        oldData = data;
        data = shaderCode.str();
        ID = glCreateShader(createdWith.shaderType);
        deletionQueue.emplace_front([&] { glDeleteShader(ID); });
    }

    void compile() {
        int infoLogLength{};
        const char *shaderSource = data.c_str();
        GLint compiledThisTime{false};
        glShaderSource(ID, 1, &shaderSource, nullptr);
        glCompileShader(ID);
        glGetShaderiv(ID, GL_COMPILE_STATUS, &compiledThisTime);
        if (!compiledThisTime) {
            glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &infoLogLength);
            std::vector<char> errorMessage(infoLogLength + 1);
            glGetShaderInfoLog(ID, infoLogLength, &infoLogLength, &errorMessage[0]);
            printf("%s\n", &errorMessage[0]);
            if (compiled) {
                std::cout << "Could not compile shaders...using pre-compiled shaders instead." << std::endl;
                data = oldData;
                compile();
            } else { throw std::runtime_error("failed to compile shaders: " + static_cast<std::string>(createdWith.filename)); }
        }
        compiled = compiled | (bool) compiledThisTime;
    }

    void destroy() {
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};