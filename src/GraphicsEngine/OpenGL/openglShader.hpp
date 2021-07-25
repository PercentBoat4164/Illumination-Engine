#pragma once

#include <fstream>
#include <sstream>

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
        if (!file.is_open()) { throw std::runtime_error("failed to load shader: " + static_cast<std::string>(createdWith.filename)); }
        std::stringstream shaderCode{};
        shaderCode << file.rdbuf();
        file.close();
        data = shaderCode.str();
        ID = glCreateShader(createdWith.shaderType);
        deletionQueue.emplace_back([&] { glDeleteShader(ID); });
    }

    void compile() {
        int infoLogLength{};
        const char *shaderSource = data.c_str();
        glShaderSource(ID, 1, &shaderSource, nullptr);
        glCompileShader(ID);
        glGetShaderiv(ID, GL_COMPILE_STATUS, reinterpret_cast<GLint *>(&compiled));
        if (!compiled) {
            glGetShaderiv(ID, GL_INFO_LOG_LENGTH, &infoLogLength);
            std::vector<char> errorMessage(infoLogLength + 1);
            glGetShaderInfoLog(ID, infoLogLength, &infoLogLength, &errorMessage[0]);
            printf("%s\n", &errorMessage[0]);
            throw std::runtime_error("failed to compile shader: " + static_cast<std::string>(createdWith.filename));
        }
    }

    void destroy() {
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};