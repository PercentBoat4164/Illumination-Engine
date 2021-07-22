#pragma once

#include "openglShader.hpp"

class OpenGLProgram {
public:
    struct CreateInfo {
        //REQUIRED
        std::vector<OpenGLShader> shaders{};
    };

    CreateInfo createdWith{};
    bool linked{false};
    uint32_t ID{};

    void create(CreateInfo *createInfo) {
        createdWith = *createInfo;
        int infoLogLength{};
        ID = glCreateProgram();
        for (OpenGLShader shader : createdWith.shaders) {
            if (!shader.compiled) { shader.compile(); }
            glAttachShader(ID, shader.ID);
        }
        glLinkProgram(ID);
        glGetProgramiv(ID, GL_LINK_STATUS, reinterpret_cast<GLint *>(&linked));
        if (!linked) {
            glGetProgramiv(ID, GL_INFO_LOG_LENGTH, &infoLogLength);
            std::vector<char> errorMessage(infoLogLength + 1);
            glGetProgramInfoLog(ID, infoLogLength, &infoLogLength, &errorMessage[0]);
            printf("%s\n", &errorMessage[0]);
            throw std::runtime_error("failed to link program!");
        }
    }

    void destroy() {
        for (const OpenGLShader& shader : createdWith.shaders) {
            glDetachShader(ID, shader.ID);
            shader.destroy();
        }
    }

    void setValue(const char *name, glm::mat4 data) const {
        GLint oldID{};
        glGetIntegerv(GL_CURRENT_PROGRAM,  &oldID);
        glUseProgram(ID);
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &data[0][0]);
        glUseProgram(oldID);
    }

    void setValue(const char *name, GLint data) const {
        GLint oldID{};
        glGetIntegerv(GL_CURRENT_PROGRAM,  &oldID);
        glUseProgram(ID);
        glUniform1i(glGetUniformLocation(ID, name), data);
        glUseProgram(oldID);
    }
};