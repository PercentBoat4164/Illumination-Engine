#pragma once

#include "openglSettings.hpp"
#include "openglCamera.hpp"
#include "openglRenderable.hpp"

#ifndef GLEW_IMPLEMENTATION
#define GLEW_IMPLEMENTATION
#include "../../../deps/glew/include/GL/glew.h"
#endif

#include <GLFW/glfw3.h>

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "../../../deps/stb_image.h"
#endif

#include <array>
#include <string>
#include <fstream>
#include <vector>

class OpenGLRenderEngine {
public:
    GLFWwindow *window{};
    OpenGLSettings settings{};
    OpenGLCamera camera{&settings};
    std::vector<OpenGLRenderable *> renderables;

    explicit OpenGLRenderEngine(GLFWwindow *attachWindow = nullptr) {
        if(!glfwInit()) { throw std::runtime_error("failed to initialize GLFW"); }
        glfwWindowHint(GLFW_SAMPLES, settings.msaaSamples);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS.
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(settings.resolution[0], settings.resolution[1], settings.applicationName.c_str(), settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr, attachWindow);
        // load icon
        int width, height, channels, sizes[] = {256, 128, 64, 32, 16};
        GLFWimage icons[sizeof(sizes)/sizeof(int)];
        for (unsigned long i = 0; i < sizeof(sizes)/sizeof(int); ++i) {
            std::string filename = "res/Logos/IlluminationEngineLogo" + std::to_string(sizes[i]) + ".png";
            stbi_uc *pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { throw std::runtime_error("failed to load texture image from file: " + filename); }
            icons[i].pixels = pixels;
            icons[i].height = height;
            icons[i].width = width;
        }
        glfwSetWindowIcon(window, sizeof(icons)/sizeof(GLFWimage), icons);
        for (GLFWimage icon : icons) { stbi_image_free(icon.pixels); }
        glfwSetWindowSizeLimits(window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
        int xPos{settings.windowPosition[0]}, yPos{settings.windowPosition[1]};
        glfwGetWindowPos(window, &xPos, &yPos);
        settings.windowPosition = {xPos, yPos};
        glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, 0);
        glfwSetWindowUserPointer(window, this);
        if (window == nullptr) { throw std::runtime_error("failed to open GLFW window!"); }
        glfwMakeContextCurrent(window);
        #if defined(_WIN32)
        glfwSwapInterval(settings.vSync ? 1 : 0);
        #else
        glfwSwapInterval(1); // VSync is mandatory on Linux in OpenGL due to nVidia driver bugs?
        #endif
        glewExperimental = true;
        if (glewInit() != GLEW_OK) { throw std::runtime_error("failed to initialize GLEW!"); }
    }

    void uploadRenderable(OpenGLRenderable *renderable, bool append = true) {
        renderable->loadModel(renderable->modelFilename);
        renderable->loadShaders(renderable->shaderFilenames);
        renderable->loadTextures(renderable->textureFilenames);
        if (append) { renderables.push_back(renderable); }
    }

    bool update() {
        if (glfwWindowShouldClose(window)) { return true; }
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (OpenGLRenderable *renderable : renderables) {
            glUseProgram(renderable->programID);
            glUniformMatrix4fv((GLint)renderable->modelMatrixID, 1, GL_FALSE, &camera.update()[0][0]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, renderable->textures[0]);
            glBindVertexArray(renderable->vertexArrayObject);
            glDrawElements(GL_TRIANGLES, (GLsizei)renderable->vertices.size(), GL_UNSIGNED_INT, nullptr);
        }
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

    GLFWmonitor *monitor{};
};
