#pragma once

#ifndef GLEW_IMPLEMENTATION
#define GLEW_IMPLEMENTATION
#include <GL/glew.h>
#endif

#define GLFW_INCLUDE_VULKAN  // Needed for glfwCreateWindowSurface
#include <GLFW/glfw3.h>

#include <vector>
#include <array>

#ifndef _WIN32
#include <climits>
#include <unistd.h>
#endif

#include <string>

class OpenGLSettings {
public:
    bool rayTracing{false};
    bool vSync{true};
    std::string applicationName{"Illumination Engine"};
    std::array<int, 3> applicationVersion{0, 0, 1};
    int msaaSamples{8};
    std::array<int, 2> defaultWindowResolution{800, 600};
    std::array<int, 2> windowPosition{10, 10};
    bool mipMapping{true};
    bool fullscreen{false};
    int refreshRate{144};
    std::array<int, 2> resolution{defaultWindowResolution};
    double fov{90};
    double renderDistance{1000000};
    double movementSpeed{2.5};
    double mouseSensitivity{0.1};
};