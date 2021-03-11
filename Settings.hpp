#pragma once

#include <glew/include/GL/glew.h>

#define GLFW_INCLUDE_VULKAN
#include "sources/glfw/include/GLFW/glfw3.h"

#include <array>
#include <filesystem>
#include <vector>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <climits>
#include <unistd.h>
#endif


class Settings {
public:
    std::vector<const char*> validationLayers = {};
    std::vector<const char*> requestedExtensions  = {"VK_KHR_swapchain"};
    std::string applicationName = "RenderEngine";
    std::array<int, 3> applicationVersion = {0, 0, 1};
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    int anisotropicFilterLevel = 1;
    int mipLevels = 1;
    bool fullscreen = false;
    int refreshRate = 60;
    std::array<int, 2> resolution = {800, 600};
    int MAX_FRAMES_IN_FLIGHT = 2;
    std::string absolutePath = getProgramPath().string();


    static std::filesystem::path getProgramPath()
    {
        #ifdef _WIN32
        wchar_t buffer[MAX_PATH]{};
        GetModuleFileNameW(nullptr, buffer, MAX_PATH);
        std::string path = std::string(reinterpret_cast<const char *const>(buffer));
        #else
        char result[PATH_MAX];
        ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
        std::string path = std::string(result, (count > 0) ? count : 0);
        #endif
        return path.substr(0, path.find_last_of('/') + 1);
    }

    void set(std::vector<const char*>& layers, std::vector<const char*>& extensions, std::string& name, std::array<int, 3>& version) {
        for (auto& layer : layers) {validationLayers.insert(validationLayers.end(), layer);}
        for (auto& extension : extensions) {requestedExtensions.insert(requestedExtensions.end(), extension);}
        applicationName = name;
        applicationVersion = version;
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
        if (physicalDeviceFeatures.samplerAnisotropy) {anisotropicFilterLevel = static_cast<int>(physicalDeviceProperties.limits.maxSamplerAnisotropy);}
        return *this;
    }
};