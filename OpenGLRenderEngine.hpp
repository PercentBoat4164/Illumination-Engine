#pragma once

#define GLEW_IMPLEMENTATION
#include <glew/include/GL/glew.h>

#include <GLFW/glfw3.h>

#include <array>
#include <string>
#include <fstream>
#include <vector>


#include "Settings.hpp"

class OpenGLRenderEngine {
public:
    Settings settings{};
    GLFWwindow *window{};
    GLuint vertexBuffer{};
    GLuint programID{};

    explicit OpenGLRenderEngine(Settings &initialSettings = *new Settings{}) {
        settings = initialSettings;
        if(!glfwInit()) { throw std::runtime_error("failed to initialize GLFW"); }
        glfwWindowHint(GLFW_SAMPLES, settings.msaaSamples);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(settings.resolution[0], settings.resolution[1], settings.applicationName.c_str(), nullptr, nullptr);
        if (window == nullptr) { throw std::runtime_error("failed to open GLFW window!"); }
        glfwMakeContextCurrent(window);
        glewExperimental = true;
        if (glewInit() != GLEW_OK) { throw std::runtime_error("failed to initialize GLEW!"); }
        GLuint VertexArrayID;
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);
        //Create vertex buffer
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
         programID = loadShaders({"shaders/vertexShader.glsl", "shaders/fragmentShader.glsl"});
    }

    [[nodiscard]] int update() const {
        if (glfwWindowShouldClose(window)) {
            return 1;
        }
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(0);
        glfwSwapBuffers(window);
        return 0;
    }

    ~OpenGLRenderEngine() {
        cleanUp();
    }

private:
    constexpr static const GLfloat g_vertex_buffer_data[] = {-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f};

    static void cleanUp() {
        glFinish();
        glfwTerminate();
    }

    [[nodiscard]] GLuint loadShaders(const std::array<std::string, 2>& paths) const {
        GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        std::array<GLuint, 2> shaderIDs = {vertexShaderID, fragmentShaderID};
        GLint Result = GL_FALSE;
        int InfoLogLength;
        for (unsigned int i = 0; i < paths.size(); i++) {
            std::ifstream file(settings.absolutePath + paths[i], std::ios::in);
            if (!file.is_open()) { throw std::runtime_error("failed to load shader: " + settings.absolutePath + paths[i]); }
            std::stringstream stringStream;
            stringStream << file.rdbuf();
            std::string shaderCode = stringStream.str();
            file.close();
            char const * sourcePointer = shaderCode.c_str();
            glShaderSource(shaderIDs[i], 1, &sourcePointer, nullptr);
            glCompileShader(shaderIDs[i]);
            glGetShaderiv(shaderIDs[i], GL_COMPILE_STATUS, &Result);
            glGetShaderiv(shaderIDs[i], GL_INFO_LOG_LENGTH, &InfoLogLength);
            if (InfoLogLength > 1) { throw std::runtime_error("failed to compile shader: " + settings.absolutePath + paths[i]); }
        }
        GLuint ProgramID = glCreateProgram();
        glAttachShader(ProgramID, vertexShaderID);
        glAttachShader(ProgramID, fragmentShaderID);
        glLinkProgram(ProgramID);
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0){
            std::vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
            printf("%s\n", &ProgramErrorMessage[0]);
        }
        glDetachShader(ProgramID, vertexShaderID);
        glDetachShader(ProgramID, fragmentShaderID);
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        return ProgramID;
    }
};
