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
#include <set>

class Settings {
public:
    int msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    int anisotropicFilterLevel = 1;
};

class VulkanRenderEngine {
public:
    VulkanRenderEngine(std::vector<const char*>& validationLayers, std::vector<const char*>& requestedExtensions, std::array<int, 2>& size, std::string& name, std::array<int, 3>& version) {
        createWindow(size, name);
        createVulkanInstance(validationLayers, requestedExtensions, name, version);
        createWindowSurface();
        findBestSuitableDevice(requestedExtensions);
        settings = findMaxSettings();
        createLogicalDevice();
    }

    void start() {

    }

    void update() {

    }

    void cleanUp() {

    }

private:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    VkInstance instance{};
    GLFWwindow* window{};
    VkDebugUtilsMessengerEXT debugMessenger{};
    bool framebufferResized = false;
    VkSurfaceKHR surface{};
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{};
    Settings settings{};
    QueueFamilyIndices indices{};
    VkDevice device{};

    VkDevice createLogicalDevice() {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {indices.graphicsFamily = i;}
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if (presentSupport) {indices.presentFamily = i;}
            if (indices.presentFamily.has_value() && indices.graphicsFamily.has_value()) {break;}
            i++;
        }
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = (settings.anisotropicFilterLevel > 1) ? VK_TRUE: VK_FALSE;
        deviceFeatures.sampleRateShading = VK_TRUE;

        return device;
    }

    Settings findMaxSettings() {
        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                    physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) { settings.msaaSamples = VK_SAMPLE_COUNT_64_BIT; }
        else if (counts & VK_SAMPLE_COUNT_32_BIT) {settings.msaaSamples = VK_SAMPLE_COUNT_32_BIT;}
        else if (counts & VK_SAMPLE_COUNT_16_BIT) {settings.msaaSamples = VK_SAMPLE_COUNT_16_BIT;}
        else if (counts & VK_SAMPLE_COUNT_8_BIT) {settings.msaaSamples = VK_SAMPLE_COUNT_8_BIT;}
        else if (counts & VK_SAMPLE_COUNT_4_BIT) {settings.msaaSamples = VK_SAMPLE_COUNT_4_BIT;}
        else if (counts & VK_SAMPLE_COUNT_2_BIT) {settings.msaaSamples = VK_SAMPLE_COUNT_2_BIT;}
        else if (counts & VK_SAMPLE_COUNT_1_BIT) {settings.msaaSamples = VK_SAMPLE_COUNT_1_BIT;}
        if (physicalDeviceFeatures.samplerAnisotropy) {
            settings.anisotropicFilterLevel = static_cast<int>(physicalDeviceProperties.limits.maxSamplerAnisotropy);
        }
        return settings;
    }

    VkPhysicalDevice findBestSuitableDevice(std::vector<const char*>& requestedExtensions) {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        long unsigned int highestMemorySize = 0;
        long unsigned int deviceMemorySize;
        for (auto & singleDevice : devices) {
            uint32_t extensionCount;
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(singleDevice, nullptr, &extensionCount, availableExtensions.data());
            std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());
            for (const auto& extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }
            if (!requiredExtensions.empty()) {continue;}
            vkGetPhysicalDeviceProperties(singleDevice, &physicalDeviceProperties);
            if (physicalDeviceProperties.deviceType == 2) {
                vkGetPhysicalDeviceMemoryProperties(singleDevice, &physicalDeviceMemoryProperties);
                deviceMemorySize = physicalDeviceMemoryProperties.memoryHeaps[0].size;
                if (deviceMemorySize > highestMemorySize) {
                    highestMemorySize = deviceMemorySize;
                    physicalDevice = singleDevice;
                }
            }
        }
        if (physicalDevice == VK_NULL_HANDLE) {throw std::runtime_error("None of the GPUs in your system support Vulkan. If you know this to be false try updating your drivers.");}
        return physicalDevice;
    }

    VkSurfaceKHR createWindowSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {throw std::runtime_error("failed to create window surface!");}
        return surface;
    }

    VkInstance createVulkanInstance(std::vector<const char*>& validationLayers, std::vector<const char*>& requestedExtensions, std::string& name, std::array<int, 3>& version) {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = name.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(version[0], version[1], version[2]);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion = VK_API_VERSION_1_2;
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (!validationLayers.empty()) {extensions.push_back("VK_EXT_debug_utils");}
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (!validationLayers.empty()) {
            //Check for validation layer support
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
            for (const char* layerName : validationLayers) {
                bool layerFound = false;
                for (const auto& layerProperties : availableLayers) {
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }
                if (!layerFound) {throw std::runtime_error("validation layers requested, but not available!");}
            }
            //Setup debugger
            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = debugCallback;
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {throw std::runtime_error("failed to create Vulkan instance!");}
        if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {throw std::runtime_error("failed to setup debug messenger!");}
        return instance;
    }

    GLFWwindow* createWindow(std::array<int, 2>& size, std::string& name) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(size[0], size[1], name.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        return window;
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        reinterpret_cast<VulkanRenderEngine*>(glfwGetWindowUserPointer(window))->framebufferResized = true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {return func(instance, pCreateInfo, pAllocator, pDebugMessenger);}
        else {return VK_ERROR_EXTENSION_NOT_PRESENT;}
    }
};

int main() {
    std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char*> requestedExtensions = {"VK_KHR_swapchain"};
    std::array<int, 2> size = {800, 600};
    std::string name = "RenderEngine";
    std::array<int, 3> version = {0, 0, 1};
    VulkanRenderEngine app(validationLayers, requestedExtensions, size, name, version);
    try {
        app.start();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}