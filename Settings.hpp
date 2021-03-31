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

struct Settings {
public:
    std::string applicationName = "Crystal Engine";
    std::array<int, 3> applicationVersion = {0, 0, 1};
    std::array<int, 2> requiredVulkanVersion = {1, 2};
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_8_BIT;
    GLFWmonitor *monitor{glfwGetPrimaryMonitor()};
    std::array<int, 2> defaultMonitorResolution{2560, 1440};
    std::array<int, 2> defaultWindowResolution{800, 600};
    float anisotropicFilterLevel = 0;
    int mipLevels = 1;
    bool fullscreen = false;
    int refreshRate = 144;
    std::array<int, 2> resolution = defaultWindowResolution;
    int MAX_FRAMES_IN_FLIGHT = 2;
    std::string absolutePath = getProgramPath().string();
    float fov{90};
    double renderDistance{1000};

    static std::filesystem::path getProgramPath() {
        #ifdef _WIN32
        wchar_t buffer[MAX_PATH]{};
        GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        char chars[sizeof(buffer) / 8];
        char defChar = ' ';
        WideCharToMultiByte(CP_ACP, 0, buffer, -1, chars, sizeof(buffer) / 8, &defChar, nullptr);
        std::string path = std::string(chars);
        #else
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        std::string path = std::string(result, (count > 0) ? count : 0);
        #endif
        return path.substr(0, path.find_last_of('\\') + 1);
    }

    Settings findMaxSettings(VkPhysicalDevice physicalDevice) {
        VkPhysicalDeviceFeatures physicalDeviceFeatures{};
        vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
        VkPhysicalDeviceProperties physicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
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