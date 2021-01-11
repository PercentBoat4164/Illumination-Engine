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
    std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char*> requestedExtensions  = {"VK_KHR_swapchain"};
    std::string name = "RenderEngine";
    std::array<int, 3> version = {0, 0, 1};
    int msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    int anisotropicFilterLevel = 1;
    bool fullscreen = false;
    int refreshRate = 60;
    std::array<int, 2> resolution = {800, 600};

    Settings findMaxSettings(VkPhysicalDevice physicalDevice) {
        VkPhysicalDeviceFeatures physicalDeviceFeatures{};
        vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
        VkPhysicalDeviceProperties physicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        refreshRate = mode->refreshRate;
        resolution[0] = mode->width;
        resolution[1] = mode->height;
        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                    physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) {msaaSamples = VK_SAMPLE_COUNT_64_BIT;}
        else if (counts & VK_SAMPLE_COUNT_32_BIT) {msaaSamples = VK_SAMPLE_COUNT_32_BIT;}
        else if (counts & VK_SAMPLE_COUNT_16_BIT) {msaaSamples = VK_SAMPLE_COUNT_16_BIT;}
        else if (counts & VK_SAMPLE_COUNT_8_BIT) {msaaSamples = VK_SAMPLE_COUNT_8_BIT;}
        else if (counts & VK_SAMPLE_COUNT_4_BIT) {msaaSamples = VK_SAMPLE_COUNT_4_BIT;}
        else if (counts & VK_SAMPLE_COUNT_2_BIT) {msaaSamples = VK_SAMPLE_COUNT_2_BIT;}
        else if (counts & VK_SAMPLE_COUNT_1_BIT) {msaaSamples = VK_SAMPLE_COUNT_1_BIT;}
        if (physicalDeviceFeatures.samplerAnisotropy) {
            anisotropicFilterLevel = static_cast<int>(physicalDeviceProperties.limits.maxSamplerAnisotropy);
        }
        return *this;
    }
};

class VulkanRenderEngine {
public:
    GLFWwindow* window{};
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    explicit VulkanRenderEngine(const Settings& startSettings) {
        settings = startSettings;
        settings.fullscreen = false;
        createWindow();
        setSettings(startSettings);
        createVulkanInstance();
        createWindowSurface();
        findBestSuitableDevice();
        createLogicalDevice();
        createSwapChain();
    }

    void start() {

    }

    void setSettings(const Settings& newSettings) {
        if (!settings.fullscreen & newSettings.fullscreen) {glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, newSettings.resolution[0], newSettings.resolution[0], newSettings.refreshRate);}
        else if (settings.fullscreen & !newSettings.fullscreen) {glfwSetWindowMonitor(window, nullptr, 0, 0, 800, 600, settings.refreshRate);}
        else {glfwSetWindowSize(window, newSettings.resolution[0], newSettings.resolution[1]);}
        settings = newSettings;
    }

    int update() {
        if (glfwWindowShouldClose(window)) {
            cleanUp();
            vkDeviceWaitIdle(logicalDevice);
            return 1;}
        return 0;
    }
private:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    VkInstance instance{};
    VkDebugUtilsMessengerEXT debugMessenger{};
    bool framebufferResized = false;
    VkSurfaceKHR surface{};
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{};
    Settings settings{};
    QueueFamilyIndices indices{};
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};
    VkDevice logicalDevice{};
    VkSwapchainKHR swapChain{};
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};

    void cleanUp() const {
        glfwSetWindowMonitor(window, nullptr, 0, 0, 800, 600, 0);
    }

    VkSwapchainKHR createSwapChain() {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        if(presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());
        }
        //Choose best format
        VkSurfaceFormatKHR surfaceFormat;
        bool chosen = false;
        for (const auto& availableFormat : details.formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surfaceFormat = availableFormat;
                chosen = true;
                break;
            }
        }
        if (!chosen) {surfaceFormat = details.formats[0];}
        //Choose best present mode
        VkPresentModeKHR presentMode;
        chosen = false;
        for (const auto& availablePresentMode : details.presentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                presentMode = availablePresentMode;
                chosen = true;
                break;
            }
        }
        if (!chosen) {presentMode = VK_PRESENT_MODE_FIFO_KHR;}
        //Choose swap chain extent
        VkExtent2D extent;
        if (details.capabilities.currentExtent.width != UINT32_MAX) {extent = details.capabilities.currentExtent;}
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            extent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
            };
            extent.width = (std::max)(details.capabilities.minImageExtent.width, (std::min)(details.capabilities.maxImageExtent.width, extent.width));
            extent.height = (std::max)(details.capabilities.minImageExtent.height, (std::min)(details.capabilities.maxImageExtent.height, extent.height));
        }
        //Create swap chian
        uint32_t imageCount = details.capabilities.minImageCount + 1;
        if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {imageCount = details.capabilities.maxImageCount;}
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;}
        createInfo.preTransform = details.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {throw std::runtime_error("failed to create swap chain!");}
        vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
        return swapChain;
    }

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
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(settings.requestedExtensions.size());
        createInfo.ppEnabledExtensionNames = settings.requestedExtensions.data();
        if (!settings.validationLayers.empty()) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(settings.validationLayers.size());
            createInfo.ppEnabledLayerNames = settings.validationLayers.data();
        } else {createInfo.enabledLayerCount = 0;}
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {throw std::runtime_error("failed to create logical device!");}
        vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
        return logicalDevice;
    }

    VkPhysicalDevice findBestSuitableDevice() {
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
            std::set<std::string> requiredExtensions(settings.requestedExtensions.begin(), settings.requestedExtensions.end());
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

    VkInstance createVulkanInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = settings.name.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(settings.version[0], settings.version[1], settings.version[2]);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion = VK_API_VERSION_1_2;
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (!settings.validationLayers.empty()) {extensions.push_back("VK_EXT_debug_utils");}
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (!settings.validationLayers.empty()) {
            //Check for validation layer support
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
            for (const char* layerName : settings.validationLayers) {
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
            createInfo.enabledLayerCount = static_cast<uint32_t>(settings.validationLayers.size());
            createInfo.ppEnabledLayerNames = settings.validationLayers.data();
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {throw std::runtime_error("failed to create Vulkan instance!");}
        if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {throw std::runtime_error("failed to setup debug messenger!");}
        return instance;
    }

    GLFWwindow* createWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(settings.resolution[0], settings.resolution[1], settings.name.c_str(), nullptr, nullptr);
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

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {glfwSetWindowShouldClose(window, 1);}
}

int main() {
    Settings settings{};
    VulkanRenderEngine RenderEngine(settings);
    settings.fullscreen = true;
    RenderEngine.setSettings(settings.findMaxSettings(RenderEngine.physicalDevice));
    try {
        RenderEngine.start();
        glfwSetKeyCallback(RenderEngine.window, keyCallback);
        while (RenderEngine.update() != 1) {
            glfwPollEvents();
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}