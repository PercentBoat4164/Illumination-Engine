#pragma once

#include "openglSettings.hpp"
#include "openglCamera.hpp"

#define GLEW_IMPLEMENTATION
#include "../../../deps/glew/include/GL/glew.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../../../deps/stb_image.h"

#include <GLFW/glfw3.h>

#include <array>
#include <string>
#include <fstream>
#include <vector>

class OpenGLRenderEngine {
public:
    GLFWwindow *window{};
    OpenGLSettings settings{};
    OpenGLCamera camera{&settings};
    GLuint vertexBuffer{};
    GLuint programID{};
    GLuint modelMatrixID{};

    explicit OpenGLRenderEngine(GLFWwindow *attachWindow = nullptr) {
        if(!glfwInit()) { throw std::runtime_error("failed to initialize GLFW");}
        glfwWindowHint(GLFW_SAMPLES, settings.msaaSamples);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS.
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(settings.resolution[0], settings.resolution[1], settings.applicationName.c_str(), settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr, attachWindow);
        // load icon
        int width, height, channels;
        stbi_uc *pixels = stbi_load("res/CrystalEngineLogo1024x1024.png", &width, &height, &channels, STBI_rgb_alpha);
        if (!pixels) { throw std::runtime_error("failed to load texture image from file: res/CrystalEngineLogo1024x1024.png"); }
        GLFWimage icons[1];
        icons[0].pixels = pixels;
        icons[0].height = height;
        icons[0].width = width;
        glfwSetWindowIcon(window, 1, icons);
        stbi_image_free(pixels);
        glfwSetWindowSizeLimits(window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
        int xPos{settings.windowPosition[0]}, yPos{settings.windowPosition[1]};
        glfwGetWindowPos(window, &xPos, &yPos);
        settings.windowPosition = {xPos, yPos};
        glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, 0);
        glfwSetWindowUserPointer(window, this);
        if (window == nullptr) { throw std::runtime_error("failed to open GLFW window!"); }
        glfwMakeContextCurrent(window);
        #if defined(WIN32)
        glfwSwapInterval(settings.vSync ? 1 : 0);
        #else
        glfwSwapInterval(1)
        #endif
        glewExperimental = true;
        if (glewInit() != GLEW_OK) { throw std::runtime_error("failed to initialize GLEW!"); }
        GLuint VertexArrayID;
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);
        programID = loadShaders({"OpenGLShaders/vertexShader.glsl", "OpenGLShaders/fragmentShader.glsl"});
        modelMatrixID = glGetUniformLocation(programID, "MVP");
        //Create vertex buffer
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    }

    bool update() {
        if (glfwWindowShouldClose(window)) { return true; }
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);
        glUniformMatrix4fv((GLint)modelMatrixID, 1, GL_FALSE, &camera.update()[0][0]);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(0);
        glfwSwapBuffers(window);
        auto currentTime = (float)glfwGetTime();
        frameTime = currentTime - previousTime;
        previousTime = currentTime;
        ++frameNumber;
        return false;
    }

    ~OpenGLRenderEngine() {
        cleanUp();
    }

    void updateSettings(bool updateAll = false) {
        if (settings.fullscreen) {
            glfwGetWindowPos(window, &settings.windowPosition[0], &settings.windowPosition[1]);
            //find monitor that window is on
            int monitorCount{}, i, windowX{}, windowY{}, windowWidth{}, windowHeight{}, monitorX{}, monitorY{}, monitorWidth, monitorHeight, bestMonitorWidth{}, bestMonitorHeight{}, bestMonitorRefreshRate{}, overlap, bestOverlap{0};
            GLFWmonitor **monitors;
            const GLFWvidmode *mode;
            glfwGetWindowPos(window, &windowX, &windowY);
            glfwGetWindowSize(window, &windowWidth, &windowHeight);
            monitors = glfwGetMonitors(&monitorCount);
            for (i = 0; i < monitorCount; i++) {
                mode = glfwGetVideoMode(monitors[i]);
                glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);
                monitorWidth = mode->width;
                monitorHeight = mode->height;
                overlap = std::max(0, std::min(windowX + windowWidth, monitorX + monitorWidth) - std::max(windowX, monitorX)) * std::max(0, std::min(windowY + windowHeight, monitorY + monitorHeight) - std::max(windowY, monitorY));
                if (bestOverlap < overlap) {
                    bestOverlap = overlap;
                    monitor = monitors[i];
                    bestMonitorWidth = monitorWidth;
                    bestMonitorHeight = monitorHeight;
                    bestMonitorRefreshRate = mode->refreshRate;
                }
            }
            //put window in fullscreen on that monitor
            glfwSetWindowMonitor(window, monitor, 0, 0, bestMonitorWidth, bestMonitorHeight, bestMonitorRefreshRate);
        } else { glfwSetWindowMonitor(window, nullptr, settings.windowPosition[0], settings.windowPosition[1], settings.defaultWindowResolution[0], settings.defaultWindowResolution[1], settings.refreshRate); }
        glfwSetWindowTitle(window, settings.applicationName.c_str());
        if (updateAll) { rebuildOpenGLInstance(); }
    }

    void rebuildOpenGLInstance() {
        glfwWindowHint(GLFW_SAMPLES, settings.msaaSamples);
        glViewport(0, 0, (GLsizei)settings.resolution[0], (GLsizei)settings.resolution[1]);
        glScissor(0, 0, (GLsizei)settings.resolution[0], (GLsizei)settings.resolution[1]);
    }

    double frameTime{};
    double previousTime{};
    int frameNumber{};

private:
    constexpr static const GLfloat g_vertex_buffer_data[] = {-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f};

    static void cleanUp() {
        glFinish();
        glfwTerminate();
    }

    static GLuint loadShaders(const std::array<std::string, 2>& paths) {
        GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        std::array<GLuint, 2> shaderIDs = {vertexShaderID, fragmentShaderID};
        GLint Result = GL_FALSE;
        int InfoLogLength{0};
        for (unsigned int i = 0; i < paths.size(); i++) {
            std::ifstream file(paths[i], std::ios::in);
            if (!file.is_open()) { throw std::runtime_error("failed to load shader: " + paths[i]); }
            std::stringstream stringStream;
            stringStream << file.rdbuf();
            std::string shaderCode = stringStream.str();
            file.close();
            char const *sourcePointer = shaderCode.c_str();
            glShaderSource(shaderIDs[i], 1, &sourcePointer, nullptr);
            glCompileShader(shaderIDs[i]);
            glGetShaderiv(shaderIDs[i], GL_COMPILE_STATUS, &Result);
            glGetShaderiv(shaderIDs[i], GL_INFO_LOG_LENGTH, &InfoLogLength);
            if (InfoLogLength > 1) { throw std::runtime_error("failed to compile shader: " + paths[i]); }
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

    GLFWmonitor *monitor{};
};
