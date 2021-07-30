#pragma once

#include "openglShader.hpp"

class OpenGLProgram {
public:
    struct CreateInfo {
        //Required
        std::vector<OpenGLShader> shaders{};
    };

    CreateInfo createdWith{};
    bool linked{false};
    uint32_t ID{};

    void create(CreateInfo *createInfo) {
        createdWith = *createInfo;
        int infoLogLength{};
        ID = glCreateProgram();
        deletionQueue.emplace_front([&] { glDeleteProgram(ID); });
        for (OpenGLShader shader : createdWith.shaders) {
            if (!shader.compiled) { shader.compile(); }
            glAttachShader(ID, shader.ID);
        }
        deletionQueue.emplace_front([&] { for (const OpenGLShader& shader : createdWith.shaders) { glDetachShader(ID, shader.ID); } });
        deletionQueue.emplace_front([&] { for (OpenGLShader& shader : createdWith.shaders) { shader.destroy(); } });
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

    void setValue(const char *name, glm::mat4 data) const {
        GLint oldID{};
        glGetIntegerv(GL_CURRENT_PROGRAM,  &oldID);
        if (ID == oldID) {
            glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &data[0][0]);
            return;
        }
        glUseProgram(ID);
        glUniformMatrix4fv(glGetUniformLocation(ID, name), 1, GL_FALSE, &data[0][0]);
        glUseProgram(oldID);
    }

    void setValue(const char *name, GLint data) const {
        GLint oldID{};
        glGetIntegerv(GL_CURRENT_PROGRAM,  &oldID);
        if (ID == oldID) {
            glUniform1i(glGetUniformLocation(ID, name), data);
            return;
        }
        glUseProgram(ID);
        glUniform1i(glGetUniformLocation(ID, name), data);
        glUseProgram(oldID);
    }

    void setValue(const char *name, glm::vec2 data) const {
        GLint oldID{};
        glGetIntegerv(GL_CURRENT_PROGRAM,  &oldID);
        if (ID == oldID) {
            glUniform2fv(glGetUniformLocation(ID, name), 1, &data[0]);
            return;
        }
        glUseProgram(ID);
        glUniform2fv(glGetUniformLocation(ID, name), 1, &data[0]);
        glUseProgram(oldID);
    }

    void destroy() {
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};