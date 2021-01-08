#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "sources/stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "sources/tiny_obj_loader.h"

#include <iostream>
#include <algorithm>
#include <vector>
#include <cstdlib>

bool enableValidationLayers = false;
#ifdef DEBUG
enableValidationLayers = true;
#endif

class GameEngineRenderer {
VkInstance instance{};
VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
VkPhysicalDeviceProperties physicalDeviceProperties{};
VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{};
public:
    GameEngineRenderer() {
        createVulkanInstance();
        findMaxSettingsFor(findBestSuitableDevice());
        Settings settings{};
    }

    void start() {

    }

private:
    struct Settings {
        int maxMsaaSamples;
    };

    Settings findMaxSettingsFor(VkPhysicalDevice device) {
        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;

    }

    VkPhysicalDevice findBestSuitableDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        long unsigned int highestMemorySize = 0;
        long unsigned int deviceMemorySize;
        for (auto & device : devices) {
            vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);
            if (physicalDeviceProperties.deviceType == 2) {
                vkGetPhysicalDeviceMemoryProperties(device, &physicalDeviceMemoryProperties);
                deviceMemorySize = physicalDeviceMemoryProperties.memoryHeaps[0].size;
                if (deviceMemorySize > highestMemorySize) {
                    highestMemorySize = deviceMemorySize;
                    physicalDevice = device;
                }
            }
        }
        if (physicalDevice == VK_NULL_HANDLE) {throw std::runtime_error("None of the GPUs in your system support Vulkan. If you know this to be false try updating your drivers.");}
        return physicalDevice;
    }

    VkInstance createVulkanInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "GameEngine Renderer";
        appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion = VK_API_VERSION_1_2;
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        vkCreateInstance(&createInfo, nullptr, &instance);
        return instance;
    }
};

int main() {
    GameEngineRenderer app;

    try {
        app.start(/*{"VK_LAYER_KHRONOS_validation"}, {VK_KHR_SWAPCHAIN_EXTENSION_NAME}, {800, 600}, "GameEngine Renderer v0.0.1", {1, 0, 1}*/);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}