#pragma once

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#ifdef ILLUMINATION_ENGINE_VULKAN
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#endif

#include "LogModule/Log.hpp"
#include "RenderEngineLink.hpp"


class RenderEngine {
public:
    RenderEngineLink renderEngineLink;
    Log *log{};

    explicit RenderEngine(const std::string& API, Log *pLog = nullptr) {
        log = pLog;
        log->addModule("Graphics module");
        log->log("Creating render engine...", log4cplus::INFO_LOG_LEVEL, "Graphics module");
        renderEngineLink.api.name = API;
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink.api.name == "Vulkan") {
            renderEngineLink.api.getVersion();
            vkb::detail::Result<vkb::SystemInfo> systemInfo = vkb::SystemInfo::get_system_info();
            vkb::InstanceBuilder builder;
            builder.set_app_name(renderEngineLink.settings.applicationName.c_str()).set_app_version(renderEngineLink.settings.applicationVersion.major, renderEngineLink.settings.applicationVersion.minor, renderEngineLink.settings.applicationVersion.patch).require_api_version(1, 1, 0);
            #ifndef NDEBUG
            if (systemInfo->validation_layers_available) { builder.request_validation_layers(); }
            if (systemInfo->debug_utils_available) { builder.use_default_debug_messenger(); }
            #endif
            vkb::detail::Result<vkb::Instance> instanceBuilder = builder.build();
            if (!instanceBuilder) { log->log("failed to create Vulkan instance: " + instanceBuilder.error().message() + "\n", log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            renderEngineLink.instance = instanceBuilder.value();
        }
        #endif
        if (!glfwInit()) { log->log("GLFW failed to initialize", log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
    }

    void create() {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink.api.name == "Vulkan") { glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (renderEngineLink.api.name == "OpenGL") {
            #ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            #endif
            #ifndef NDEBUG
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
            #endif
        }
        #endif
        glfwMakeContextCurrent(nullptr);
        renderEngineLink.window = glfwCreateWindow(renderEngineLink.settings.resolution[0], renderEngineLink.settings.resolution[1], (renderEngineLink.settings.applicationName + " v" + renderEngineLink.settings.applicationVersion.name).c_str(), renderEngineLink.settings.monitor, nullptr);
        glfwMakeContextCurrent(renderEngineLink.window);
        int width, height, channels, sizes[] = {256, 128, 64, 32, 16};
        GLFWimage icons[sizeof(sizes) / sizeof(int)];
        for (unsigned long i = 0; i < sizeof(sizes) / sizeof(int); ++i) {
            std::string filename = "res/Logos/IlluminationEngineLogo" + std::to_string(sizes[i]) + ".png";
            stbi_uc *pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { log->log("failed to prepare texture image from file: " + filename, log4cplus::WARN_LOG_LEVEL, "Graphics module"); }
            icons[i].pixels = pixels;
            icons[i].height = height;
            icons[i].width = width;
        }
        glfwSetWindowIcon(renderEngineLink.window, sizeof(icons) / sizeof(GLFWimage), icons);
        for (GLFWimage icon: icons) { stbi_image_free(icon.pixels); }
        glfwSetWindowSizeLimits(renderEngineLink.window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwSetWindowAttrib(renderEngineLink.window, GLFW_AUTO_ICONIFY, 0);
        glfwSetWindowUserPointer(renderEngineLink.window, this);
        glfwSetFramebufferSizeCallback(renderEngineLink.window, framebufferResizeCallback);
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink.api.name == "Vulkan") {
            if (glfwCreateWindowSurface(renderEngineLink.instance.instance, renderEngineLink.window, nullptr, &renderEngineLink.surface) != VK_SUCCESS) { log->log("failed to create window surface", log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            vkb::PhysicalDeviceSelector selector{renderEngineLink.instance};
            vkb::detail::Result<vkb::PhysicalDevice> temporaryPhysicalDeviceBuilder = selector.set_surface(renderEngineLink.surface).prefer_gpu_device_type(vkb::PreferredDeviceType::discrete).select();
            if (!temporaryPhysicalDeviceBuilder) { log->log("Physical device creation: " + temporaryPhysicalDeviceBuilder.error().message(), log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            vkb::DeviceBuilder temporaryLogicalDeviceBuilder{temporaryPhysicalDeviceBuilder.value()};
            vkb::detail::Result<vkb::Device> temporaryLogicalDevice = temporaryLogicalDeviceBuilder.build();
            if (!temporaryLogicalDevice) { log->log("Logical device creation: " + temporaryLogicalDevice.error().message(), log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            renderEngineLink.device = temporaryLogicalDevice.value();
            renderEngineLink.create();
            vkb::destroy_device(renderEngineLink.device);
            //EXTENSION SELECTION
            //-------------------
            std::vector<const char *> rayTracingExtensions{
                    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                    VK_KHR_RAY_QUERY_EXTENSION_NAME,
                    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
                    VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
            };
            std::vector<std::vector<const char *>> extensionGroups{
                    rayTracingExtensions
            };
            //DEVICE FEATURE SELECTION
            //------------------------
            std::vector<VkBool32 *> anisotropicFilteringFeatures{
                    reinterpret_cast<VkBool32 *>(&renderEngineLink.physicalDevice.enabledAPIComponents.anisotropicFiltering),
                    &renderEngineLink.physicalDevice.enabledAPIComponents.features.samplerAnisotropy
            };
            std::vector<VkBool32 *> msaaSmoothingFeatures{
                    reinterpret_cast<VkBool32 *>(&renderEngineLink.physicalDevice.enabledAPIComponents.msaaSmoothing),
                    &renderEngineLink.physicalDevice.enabledAPIComponents.features.sampleRateShading
            };
            std::vector<std::vector<VkBool32 *>> deviceFeatureGroups{
                    anisotropicFilteringFeatures,
                    msaaSmoothingFeatures
            };
            //EXTENSION FEATURE SELECTION
            //---------------------------
            std::vector<VkBool32 *> rayTracingFeatures{
                    (VkBool32*)&renderEngineLink.physicalDevice.enabledAPIComponents.rayTracing,
                    &renderEngineLink.physicalDevice.enabledAPIComponents.bufferDeviceAddressFeatures.bufferDeviceAddress,
                    &renderEngineLink.physicalDevice.enabledAPIComponents.accelerationStructureFeatures.accelerationStructure,
                    &renderEngineLink.physicalDevice.enabledAPIComponents.rayQueryFeatures.rayQuery,
                    &renderEngineLink.physicalDevice.enabledAPIComponents.rayTracingPipelineFeatures.rayTracingPipeline
            };
            std::vector<std::vector<VkBool32 *>> extensionFeatureGroups{
                    rayTracingFeatures
            };
            //===========================
            for (const std::vector<VkBool32 *> &deviceFeatureGroup : deviceFeatureGroups) {
                if (renderEngineLink.physicalDevice.testFeature(std::vector<VkBool32 *>(deviceFeatureGroup.begin() + 1, deviceFeatureGroup.end()))[0]) {
                    *deviceFeatureGroup[0] = VK_TRUE;
                    *(deviceFeatureGroup[0] - (VkBool32 *)&renderEngineLink.physicalDevice.enabledAPIComponents + (VkBool32 *)&renderEngineLink.physicalDevice.supportedAPIComponents) = VK_TRUE;
                    renderEngineLink.physicalDevice.enableFeature(deviceFeatureGroup);
                }
            }
            for (const std::vector<VkBool32 *> &extensionFeatureGroup : extensionFeatureGroups) {
                if (renderEngineLink.physicalDevice.testFeature(std::vector<VkBool32 *>(extensionFeatureGroup.begin() + 1, extensionFeatureGroup.end()))[0]) {
                    *extensionFeatureGroup[0] = VK_TRUE;
                    *(extensionFeatureGroup[0] - (VkBool32 *)&renderEngineLink.physicalDevice.enabledAPIComponents + (VkBool32 *)&renderEngineLink.physicalDevice.supportedAPIComponents) = VK_TRUE;
                    renderEngineLink.physicalDevice.enableFeature(extensionFeatureGroup);
                }
            }
            if (!extensionGroups.empty()) { selector.add_desired_extensions(*extensionGroups.data()); }
            vkb::detail::Result<vkb::PhysicalDevice> finalPhysicalDeviceBuilder = selector.set_surface(renderEngineLink.surface).set_required_features(renderEngineLink.physicalDevice.enabledAPIComponents.features).prefer_gpu_device_type(vkb::PreferredDeviceType::discrete).select();
            vkb::DeviceBuilder finalLogicalDeviceBuilder{finalPhysicalDeviceBuilder.value()};
            finalLogicalDeviceBuilder.add_pNext(renderEngineLink.physicalDevice.enabledAPIComponents.pNextHighestFeature);
            vkb::detail::Result<vkb::Device> finalLogicalDevice = finalLogicalDeviceBuilder.build();
            if (!finalLogicalDevice) { log->log("failed to create final device after initial device was successfully created", log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            renderEngineLink.device = finalLogicalDevice.value();
            VmaAllocatorCreateInfo allocatorInfo{};
            allocatorInfo.physicalDevice = renderEngineLink.device.physical_device.physical_device;
            allocatorInfo.device = renderEngineLink.device.device;
            allocatorInfo.instance = renderEngineLink.instance.instance;
            allocatorInfo.flags = renderEngineLink.physicalDevice.enabledAPIComponents.rayTracing ? VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT : 0;
            vmaCreateAllocator(&allocatorInfo, &renderEngineLink.allocator);
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (renderEngineLink.api.name == "OpenGL") {
            glewInit();
            #if defined(_WIN32)
            glfwSwapInterval(settings.vSync ? 1 : 0);
            #else
            glfwSwapInterval(1);
            #endif
            glewExperimental = true;
            if (glewInit() != GLEW_OK) { throw std::runtime_error("failed to initialize GLEW!"); }
            #ifndef NDEBUG
            int flags;
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
                glDebugMessageCallback(glDebugOutput, nullptr);
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            }
            #endif
        }
        #endif
        renderEngineLink.create();
        renderEngineLink.api.getVersion();
        renderEngineLink.physicalDevice.info.generateInfo();
        log->log(renderEngineLink.api.name + ' ' + renderEngineLink.api.version.name, log4cplus::INFO_LOG_LEVEL, "Graphics module");
        log->log(renderEngineLink.physicalDevice.info.name, log4cplus::INFO_LOG_LEVEL, "Graphics module");
    }

    void changeAPI(const std::string& API) {
        if (renderEngineLink.api.name == "OpenGL") { glFinish(); }
        renderEngineLink.~RenderEngineLink();
        renderEngineLink = RenderEngineLink();
        log->log("Creating render engine...", log4cplus::INFO_LOG_LEVEL, "Graphics module");
        renderEngineLink.api.name = API;
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink.api.name == "Vulkan") {
            renderEngineLink.api.getVersion();
            vkb::detail::Result<vkb::SystemInfo> systemInfo = vkb::SystemInfo::get_system_info();
            vkb::InstanceBuilder builder;
            builder.set_app_name(renderEngineLink.settings.applicationName.c_str()).set_app_version(renderEngineLink.settings.applicationVersion.major, renderEngineLink.settings.applicationVersion.minor, renderEngineLink.settings.applicationVersion.patch).require_api_version(1, 1, 0);
            #ifndef NDEBUG
            if (systemInfo->validation_layers_available) { builder.request_validation_layers(); }
            if (systemInfo->debug_utils_available) { builder.use_default_debug_messenger(); }
            #endif
            vkb::detail::Result<vkb::Instance> instanceBuilder = builder.build();
            if (!instanceBuilder) { log->log("Failed to create Vulkan instance. Error: " + instanceBuilder.error().message() + "\n", log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            renderEngineLink.instance = instanceBuilder.value();
        }
        #endif
        create();
    }

    ~RenderEngine() {
        if (renderEngineLink.api.name == "OpenGL") { glFinish(); }
        glfwTerminate();
    }

private:
#ifdef ILLUMINATION_ENGINE_OPENGL
    static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam) {
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes
        std::cout << "---------------" << std::endl;
        std::cout << "Debug message (" << id << "): " <<  message << std::endl;
        switch (source) {
            case GL_DEBUG_SOURCE_API:               std::cout << "Source: API"; break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     std::cout << "Source: Window System"; break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER:   std::cout << "Source: Shader Compiler"; break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:       std::cout << "Source: Third Party"; break;
            case GL_DEBUG_SOURCE_APPLICATION:       std::cout << "Source: Application"; break;
            case GL_DEBUG_SOURCE_OTHER:             std::cout << "Source: Other"; break;
            default:                                std::cout << "Source: Unknown"; break;
        }
        std::cout << std::endl;
        switch (type) {
            case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
            case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
            case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
            case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
            case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
            case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
            case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
            default:                                std::cout << "Type: Unknown"; break;
        }
        std::cout << std::endl;
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:            std::cout << "Severity: high"; break;
            case GL_DEBUG_SEVERITY_MEDIUM:          std::cout << "Severity: medium"; break;
            case GL_DEBUG_SEVERITY_LOW:             std::cout << "Severity: low"; break;
            case GL_DEBUG_SEVERITY_NOTIFICATION:    std::cout << "Severity: notification"; break;
            default:                                std::cout << "Source: Unknown"; break;
        }
        std::cout << std::endl;
        std::cout << std::endl;
    }
#endif

    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
        auto renderEngine = (RenderEngine *)glfwGetWindowUserPointer(pWindow);
        renderEngine->renderEngineLink.framebufferResized = true;
        renderEngine->renderEngineLink.settings.resolution[0] = width;
        renderEngine->renderEngineLink.settings.resolution[1] = height;
    }
};