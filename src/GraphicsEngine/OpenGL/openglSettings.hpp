#pragma once

#define GLEW_IMPLEMENTATION
#include <glew/include/GL/glew.h>

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

/** This class holds the settings used in the OpenglRenderEngine.*/
class OpenGLSettings {
public:
    bool pathTracing{false};
    std::string applicationName{"Crystal Engine"};
    std::array<int, 3> applicationVersion{0, 0, 1};
    int msaaSamples{8};
    std::array<int, 2> defaultWindowResolution{800, 600};
    std::array<int, 2> windowPosition{10, 10};
    bool fullscreen{false};
    int refreshRate{60};
    std::array<int, 2> resolution{defaultWindowResolution};
    float fov{90};
    double renderDistance{1000000};

    /** This method tries sets the window settings to the primary monitor's settings.
     * @return *this*/
    OpenGLSettings findMaxSettings() {
        const GLFWvidmode* mode{glfwGetVideoMode(glfwGetPrimaryMonitor())};
        refreshRate = mode->refreshRate;
        resolution[0] = mode->width;
        resolution[1] = mode->height;
        fullscreen = true;
        return *this;
    }
};