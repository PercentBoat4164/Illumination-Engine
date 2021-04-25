#pragma once

#define GLEW_IMPLEMENTATION
#include <glew/include/GL/glew.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <array>
#include <filesystem>

#if defined(_WIN32)
#define NOMINMAX
#include <windows.h>
#else
#include <climits>
#include <unistd.h>
#endif

class Settings {
public:
    bool pathTracing{true};
    std::string applicationName{"Crystal Engine"};
    std::array<int, 3> applicationVersion{0, 0, 1};
    std::array<int, 3> requiredVulkanVersion{1, 2, 0};
    VkSampleCountFlagBits msaaSamples{VK_SAMPLE_COUNT_8_BIT};
    std::array<int, 2> defaultWindowResolution{800, 600};
    std::array<int, 2> windowPosition{10, 10};
    float anisotropicFilterLevel{16};
    int mipLevels{1};
    bool fullscreen{false};
    int refreshRate{60};
    std::array<int, 2> resolution{defaultWindowResolution};
    int MAX_FRAMES_IN_FLIGHT{2};
    float fov{90};
    double renderDistance{1000000};

    Settings findMaxSettings(VkPhysicalDevice physicalDevice) {
        VkPhysicalDeviceFeatures physicalDeviceFeatures{};
        vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
        VkPhysicalDeviceProperties physicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        const GLFWvidmode* mode{glfwGetVideoMode(glfwGetPrimaryMonitor())};
        refreshRate = mode->refreshRate;
        resolution[0] = mode->width;
        resolution[1] = mode->height;
        fullscreen = true;
        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) {msaaSamples = VK_SAMPLE_COUNT_64_BIT;}
        else if (counts & VK_SAMPLE_COUNT_32_BIT) {msaaSamples = VK_SAMPLE_COUNT_32_BIT;}
        else if (counts & VK_SAMPLE_COUNT_16_BIT) {msaaSamples = VK_SAMPLE_COUNT_16_BIT;}
        else if (counts & VK_SAMPLE_COUNT_8_BIT) {msaaSamples = VK_SAMPLE_COUNT_8_BIT;}
        else if (counts & VK_SAMPLE_COUNT_4_BIT) {msaaSamples = VK_SAMPLE_COUNT_4_BIT;}
        else if (counts & VK_SAMPLE_COUNT_2_BIT) {msaaSamples = VK_SAMPLE_COUNT_2_BIT;}
        else if (counts & VK_SAMPLE_COUNT_1_BIT) {msaaSamples = VK_SAMPLE_COUNT_1_BIT;}
        if (physicalDeviceFeatures.samplerAnisotropy) {anisotropicFilterLevel = physicalDeviceProperties.limits.maxSamplerAnisotropy;}
        return *this;
    }
};