#pragma once

#include "openglSettings.hpp"
#include "openglCamera.hpp"
#include "openglRenderable.hpp"
#include "openglFramebuffer.hpp"
#include "openglGraphicsEngineLink.hpp"

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
    OpenGLCamera camera{};
    std::vector<OpenGLRenderable *> renderables;
    OpenGLGraphicsEngineLink renderEngineLink{};

    explicit OpenGLRenderEngine() {
        if (!glfwInit()) { throw std::runtime_error("failed to initialize GLFW"); }
        window = glfwCreateWindow(1, 1, "Finding OpenGL version...", nullptr, nullptr);
        glfwMakeContextCurrent(window);
        renderEngineLink.openglVersion = std::string(reinterpret_cast<const char *const>(glGetString(GL_VERSION)));
        glGetIntegerv(GL_MAX_SAMPLES, &renderEngineLink.maxMSAASamples);
        glfwDestroyWindow(window);
        glfwTerminate();
        glfwInit();
        deletionQueue.emplace_front([&] {
            glFinish();
            glfwTerminate();
        });
        glfwWindowHint(GLFW_SAMPLES, settings.msaaSamples);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, std::stoi(renderEngineLink.openglVersion.substr(0, 1)));
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, std::stoi(renderEngineLink.openglVersion.substr(2, 1)));
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, settings.vSync);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
#ifndef NDEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
        window = glfwCreateWindow(settings.resolution[0], settings.resolution[1], settings.applicationName.c_str(), settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
        int width, height, channels, sizes[] = {256, 128, 64, 32, 16};
        GLFWimage icons[sizeof(sizes) / sizeof(int)];
        for (unsigned long i = 0; i < sizeof(sizes) / sizeof(int); ++i) {
            std::string filename = "res/Logos/IlluminationEngineLogo" + std::to_string(sizes[i]) + ".png";
            stbi_uc *pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { throw std::runtime_error("failed to prepare texture image from file: " + filename); }
            icons[i].pixels = pixels;
            icons[i].height = height;
            icons[i].width = width;
        }
        glfwSetWindowIcon(window, sizeof(icons) / sizeof(GLFWimage), icons);
        for (GLFWimage icon: icons) { stbi_image_free(icon.pixels); }
        glfwSetWindowSizeLimits(window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
        int xPos{settings.windowPosition[0]}, yPos{settings.windowPosition[1]};
        glfwGetWindowPos(window, &xPos, &yPos);
        settings.windowPosition = {xPos, yPos};
        glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, 0);
        glfwSetWindowUserPointer(window, this);
        if (window == nullptr) { throw std::runtime_error("failed to open GLFW window!"); }
        glfwMakeContextCurrent(window);
        glfwSetWindowSizeLimits(window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
#if defined(_WIN32)
        glfwSwapInterval(settings.vSync ? 1 : 0);
#else
        glfwSwapInterval(1);
#endif
        glewExperimental = true;
        if (glewInit() != GLEW_OK) { throw std::runtime_error("failed to initialize GLEW!"); }
#ifndef NDEBUG
        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }
#endif
        renderEngineLink.settings = &settings;
        camera.create(&renderEngineLink);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
    }

    void reloadRenderables() {
        for (OpenGLRenderable *renderable : renderables) { renderable->reprepare(); }
    }

    bool update() {
        if (glfwWindowShouldClose(window)) { return true; }
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        camera.update();
        for (OpenGLRenderable *renderable : renderables) {
            if (renderable->render) {
                renderable->update();
                for (const OpenGLRenderable::OpenGLMesh& mesh : renderable->meshes) {
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, renderable->textures[mesh.diffuseTexture].ID);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, renderable->textures[mesh.emissionTexture].ID);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, renderable->textures[mesh.heightTexture].ID);
                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, renderable->textures[mesh.metallicTexture].ID);
                    glActiveTexture(GL_TEXTURE4);
                    glBindTexture(GL_TEXTURE_2D, renderable->textures[mesh.normalTexture].ID);
                    glActiveTexture(GL_TEXTURE5);
                    glBindTexture(GL_TEXTURE_2D, renderable->textures[mesh.roughnessTexture].ID);
                    glActiveTexture(GL_TEXTURE6);
                    glBindTexture(GL_TEXTURE_2D, renderable->textures[mesh.specularTexture].ID);
                    renderable->program.setValue("projection", camera.proj);
                    renderable->program.setValue("normalMatrix", glm::mat3(glm::transpose(glm::inverse(renderable->model))));
                    renderable->program.setValue("viewModel", camera.view * renderable->model);
                    renderable->program.setValue("model", renderable->model);
                    renderable->program.setValue("diffuseTexture", 0);
                    renderable->program.setValue("emissionTexture", 1);
                    renderable->program.setValue("heightTexture", 2);
                    renderable->program.setValue("metallicTexture", 3);
                    renderable->program.setValue("normalTexture", 4);
                    renderable->program.setValue("roughnessTexture", 5);
                    renderable->program.setValue("specularTexture", 6);
                    renderable->program.setValue("cameraPosition", camera.position);
                    glBindVertexArray(mesh.vertexArrayObject);
                    glUseProgram(renderable->program.ID);
                    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, nullptr);
                }
            }
        }
        glfwSwapBuffers(window);
        auto currentTime = (float)glfwGetTime();
        frameTime = currentTime - previousTime;
        previousTime = currentTime;
        ++frameNumber;
        return false;
    }

    void updateSettings() {
        if (settings.fullscreen) {
            glfwGetWindowPos(window, &settings.windowPosition[0], &settings.windowPosition[1]);
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
            glfwSetWindowMonitor(window, monitor, 0, 0, bestMonitorWidth, bestMonitorHeight, bestMonitorRefreshRate);
            settings.resolution = {bestMonitorWidth, bestMonitorHeight};
        } else {
            glfwSetWindowMonitor(window, nullptr, settings.windowPosition[0], settings.windowPosition[1], settings.defaultWindowResolution[0], settings.defaultWindowResolution[1], settings.refreshRate);
            settings.resolution = settings.defaultWindowResolution;
        }
        glfwSetWindowTitle(window, settings.applicationName.c_str());
        glViewport(0, 0, (GLsizei)settings.resolution[0], (GLsizei)settings.resolution[1]);
        glScissor(0, 0, (GLsizei)settings.resolution[0], (GLsizei)settings.resolution[1]);
    }

    double frameTime{};
    double previousTime{};
    int frameNumber{};

    void destroy() {
        for (OpenGLRenderable *renderable : renderables) {
            renderable->destroy();
            renderable->program.destroy();
        }
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

private:
    static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam) {
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes
        std::cout << "---------------" << std::endl;
        std::cout << "Debug message (" << id << "): " <<  message << std::endl;
        switch (source) {
            case GL_DEBUG_SOURCE_API:               std::cout << "Source: API"; break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     std::cout << "Source: Window System"; break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER:   std::cout << "Source: Shader Compiler"; break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:       std::cout << "Source: Third Party"; break;
            case GL_DEBUG_SOURCE_APPLICATION:       std::cout << "Source: Application"; break;
            case GL_DEBUG_SOURCE_OTHER:             std::cout << "Source: Other"; break;
            default:                                std::cout << "Source: Unknown"; break;
        }
        std::cout << std::endl;
        switch (type) {
            case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
            case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
            case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
            case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
            case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
            case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
            case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
            default:                                std::cout << "Type: Unknown"; break;
        }
        std::cout << std::endl;
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:            std::cout << "Severity: high"; break;
            case GL_DEBUG_SEVERITY_MEDIUM:          std::cout << "Severity: medium"; break;
            case GL_DEBUG_SEVERITY_LOW:             std::cout << "Severity: low"; break;
            case GL_DEBUG_SEVERITY_NOTIFICATION:    std::cout << "Severity: notification"; break;
            default:                                std::cout << "Source: Unknown"; break;
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }

    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
        auto pOpenGlRenderEngine = (OpenGLRenderEngine *)glfwGetWindowUserPointer(pWindow);
        pOpenGlRenderEngine->settings.resolution[0] = width;
        pOpenGlRenderEngine->settings.resolution[1] = height;
        pOpenGlRenderEngine->camera.updateSettings();
        glViewport(0, 0, (GLsizei)pOpenGlRenderEngine->settings.resolution[0], (GLsizei)pOpenGlRenderEngine->settings.resolution[1]);
        glScissor(0, 0, (GLsizei)pOpenGlRenderEngine->settings.resolution[0], (GLsizei)pOpenGlRenderEngine->settings.resolution[1]);
    }

    GLFWmonitor *monitor{};
    std::deque<std::function<void()>> deletionQueue{};
};
