#pragma once

#define GLEW_IMPLEMENTATION
#include "../../../deps/glew/include/GL/glew.h"

#include <GLFW/glfw3.h>

#include <vector>
#include <array>
#include <filesystem>

#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#else
#include <climits>
#include <unistd.h>
#endif

class OpenGLSettings {
public:
    bool rayTracing{false};
    bool vSync{false}; // VSync is mandatory on Linux in OpenGL due to nVidia driver bugs.
    std::string applicationName{"Crystal Engine"};
    std::array<int, 3> applicationVersion{0, 0, 1};
    int msaaSamples{8};
    std::array<int, 2> defaultWindowResolution{800, 600};
    std::array<int, 2> windowPosition{10, 10};
    bool fullscreen{false};
    int refreshRate{60};
    std::array<int, 2> resolution{defaultWindowResolution};
    double fov{90};
    double renderDistance{1000000};
    double movementSpeed{2.5};
    double mouseSensitivity{0.1};

    OpenGLSettings findMaxSettings() {
        const GLFWvidmode* mode{glfwGetVideoMode(glfwGetPrimaryMonitor())};
        refreshRate = mode->refreshRate;
        resolution[0] = mode->width;
        resolution[1] = mode->height;
        fullscreen = true;
        return *this;
    }
};