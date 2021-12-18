#pragma once

#include "IEVersion.hpp"


#ifdef ILLUMINATION_ENGINE_OPENGL
#ifndef GLEW_IMPLEMENTATION
#define GLEW_IMPLEMENTATION
#include <GL/glew.h>
#include <vulkan/vulkan.h>

#endif
#endif

#include "GLFW/glfw3.h"

class IESettings{
public:
    std::string applicationName{"Illumination Engine"};
    IEVersion applicationVersion{"0.0.0"};
    bool vSync{true};
    int32_t resolution[2] {800, 600};
    double renderResolutionScale {.5};
    int32_t renderResolution[2] = {static_cast<int32_t>(resolution[0] * renderResolutionScale), static_cast<int32_t>(resolution[1] * renderResolutionScale)};
    GLFWmonitor *monitor{};
    uint8_t msaaSamples{};
    uint32_t maxMipLevels{};
    bool rayTracing{};
};