#pragma once

#include "IEVersion.hpp"

#include <vulkan/vulkan.h>

#ifndef GLEW_IMPLEMENTATION
#define GLEW_IMPLEMENTATION
#include <GL/glew.h>
#endif

#define GLFW_INCLUDE_VULKAN  // Needed for glfwCreateWindowSurface
#include <GLFW/glfw3.h>

#include <array>

#if defined(_WIN32)
#define NOMINMAX
#include <Windows.h>
#else
#include <climits>
#include <unistd.h>
#endif

#include <string>

class IESettings {
public:
    bool rayTracing{false};
    std::string applicationName{"Illumination Engine"};
    IEVersion applicationVersion{0, 0, 1};
    IEVersion requiredVulkanVersion{1, 2, 0};
    VkSampleCountFlagBits msaaSamples{VK_SAMPLE_COUNT_8_BIT};
    std::array<uint32_t, 2> defaultWindowResolution{800, 600};
    std::array<int, 2> windowPosition{0, 0};
    float anisotropicFilterLevel{16.0f};
    bool mipMapping{true};
    float mipMapLevel{0.0f};
    bool fullscreen{false};
    int refreshRate{60};
    bool vSync{true};
    std::array<uint32_t, 2> resolution{defaultWindowResolution};
    double fov{90};
    double renderDistance{1000000};
    double mouseSensitivity{0.1};
    float movementSpeed{2.5};
};