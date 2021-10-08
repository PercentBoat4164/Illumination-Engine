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
#include "CommandPool.hpp"
#include "Buffer.hpp"
#include "Image.hpp"
#include "Texture.hpp"


class RenderEngine {
public:
    RenderEngineLink renderEngineLink;

    std::vector<VkFence> inFlightFences{};
    std::vector<VkFence> imagesInFlight{};
    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};

    explicit RenderEngine(const std::string& API, Log *pLog = nullptr) {
        renderEngineLink.log = pLog;
        renderEngineLink.log->addModule("Graphics module");
        renderEngineLink.log->log("Creating window", log4cplus::INFO_LOG_LEVEL, "Graphics module");
        renderEngineLink.api.name = API;
        renderEngineLink.log = pLog;
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
            if (!instanceBuilder) { renderEngineLink.log->log("failed to create Vulkan instance: " + instanceBuilder.error().message() + "\n", log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            renderEngineLink.instance = instanceBuilder.value();
            renderEngineLink.created.instance = true;
        }
        #endif
        if (!glfwInit()) { renderEngineLink.log->log("GLFW failed to initialize", log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
        renderEngineLink.created.glfw = true;
    }

    void create() {
        renderEngineLink.log->log("Initializing " + renderEngineLink.api.name + " API", log4cplus::INFO_LOG_LEVEL, "Graphics module");
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
        renderEngineLink.created.window = true;
        glfwMakeContextCurrent(renderEngineLink.window);
        int width, height, channels, sizes[] = {256, 128, 64, 32, 16};
        GLFWimage icons[sizeof(sizes) / sizeof(int)];
        for (unsigned long i = 0; i < sizeof(sizes) / sizeof(int); ++i) {
            std::string filename = "res/Logos/IlluminationEngineLogo" + std::to_string(sizes[i]) + ".png";
            stbi_uc *pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { renderEngineLink.log->log("Failed to prepare texture image from file: " + filename, log4cplus::WARN_LOG_LEVEL, "Graphics module"); }
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
            if (glfwCreateWindowSurface(renderEngineLink.instance.instance, renderEngineLink.window, nullptr, &renderEngineLink.surface) != VK_SUCCESS) { renderEngineLink.log->log("failed to create window surface", log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            renderEngineLink.created.surface = true;
            vkb::PhysicalDeviceSelector selector{renderEngineLink.instance};
            vkb::detail::Result<vkb::PhysicalDevice> temporaryPhysicalDeviceBuilder = selector.set_surface(renderEngineLink.surface).prefer_gpu_device_type(vkb::PreferredDeviceType::discrete).select();
            if (!temporaryPhysicalDeviceBuilder) { renderEngineLink.log->log("Physical device creation: " + temporaryPhysicalDeviceBuilder.error().message(), log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            vkb::DeviceBuilder temporaryLogicalDeviceBuilder{temporaryPhysicalDeviceBuilder.value()};
            vkb::detail::Result<vkb::Device> temporaryLogicalDevice = temporaryLogicalDeviceBuilder.build();
            if (!temporaryLogicalDevice) { renderEngineLink.log->log("Logical device creation: " + temporaryLogicalDevice.error().message(), log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            renderEngineLink.device = temporaryLogicalDevice.value();
            renderEngineLink.created.device = true;
            renderEngineLink.create();
            renderEngineLink.created.renderEngineLink = true;
            vkb::destroy_device(renderEngineLink.device);
            renderEngineLink.created.device = false;
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
            if (!finalLogicalDevice) { renderEngineLink.log->log("failed to create final device after initial device was successfully created", log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            renderEngineLink.device = finalLogicalDevice.value();
            renderEngineLink.created.device = true;
            VmaAllocatorCreateInfo allocatorInfo{};
            allocatorInfo.physicalDevice = renderEngineLink.device.physical_device.physical_device;
            allocatorInfo.device = renderEngineLink.device.device;
            allocatorInfo.instance = renderEngineLink.instance.instance;
            allocatorInfo.flags = renderEngineLink.physicalDevice.enabledAPIComponents.rayTracing ? VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT : 0;
            vmaCreateAllocator(&allocatorInfo, &renderEngineLink.allocator);
            renderEngineLink.created.allocator = true;
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
            if (glewInit() != GLEW_OK) { renderEngineLink.log->log("Failed to iniialize GLEW!", log4cplus::DEBUG_LOG_LEVEL, "Graphics Module"); }
            renderEngineLink.created.glew = true;
            #ifndef NDEBUG
            int flags;
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // makes sure errors are displayed synchronously
                glDebugMessageCallback(glDebugOutput, &renderEngineLink);
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
            }
            #endif
        }
        #endif
        renderEngineLink.create();
        renderEngineLink.api.getVersion();
        renderEngineLink.physicalDevice.info.generateInfo();
        renderEngineLink.log->log(renderEngineLink.api.name + ' ' + renderEngineLink.api.version.name, log4cplus::INFO_LOG_LEVEL, "Graphics module");
        renderEngineLink.log->log(renderEngineLink.physicalDevice.info.name, log4cplus::INFO_LOG_LEVEL, "Graphics module");
        handleWindowSizeChange();
    }

    void handleWindowSizeChange() {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink.api.name == "Vulkan") {
            vkDeviceWaitIdle(renderEngineLink.device.device);
            // clear recreation deletion queue
            vkb::SwapchainBuilder swapchainBuilder{ renderEngineLink.device };
            vkb::detail::Result<vkb::Swapchain> swapchainBuilderResults = swapchainBuilder.set_desired_present_mode(renderEngineLink.settings.vSync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR).set_desired_extent(renderEngineLink.settings.resolution[0], renderEngineLink.settings.resolution[1]).build();
            if (!swapchainBuilderResults) { renderEngineLink.log->log("failed to create swapchain.", log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            destroy_swapchain(renderEngineLink.swapchain);
            renderEngineLink.swapchain = swapchainBuilderResults.value();
            renderEngineLink.created.swapchain = true;
            renderEngineLink.swapchainImageViews = renderEngineLink.swapchain.get_image_views().value();
            imagesInFlight.clear();
            imagesInFlight.resize(renderEngineLink.swapchain.image_count, VK_NULL_HANDLE);
        }
        #endif
    }

    void changeAPI(const std::string& API) {
        renderEngineLink.destroy();
        if (renderEngineLink.api.name == "OpenGL") { glFinish(); }
        renderEngineLink = RenderEngineLink{};
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
            if (!instanceBuilder) { renderEngineLink.log->log(instanceBuilder.error().message(), log4cplus::DEBUG_LOG_LEVEL, "Graphics module"); }
            renderEngineLink.instance = instanceBuilder.value();
        }
        #endif
        create();
    }

    void destroy() {
        renderEngineLink.destroy();
        glfwTerminate();
    }

    ~RenderEngine() {
        destroy();
    }

private:
    static void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char *message, const void *userParam) {
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes
        std::string sourceText{};
        std::string typeText{};
        std::string severityText{};
        switch (source) {
            case GL_DEBUG_SOURCE_API:               sourceText = "API"; break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     sourceText = "Window System"; break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER:   sourceText = "Shader Compiler"; break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:       sourceText = "Third Party"; break;
            case GL_DEBUG_SOURCE_APPLICATION:       sourceText = "Application"; break;
            case GL_DEBUG_SOURCE_OTHER:             sourceText = "Other"; break;
            default:                                sourceText = "Unknown"; break;
        }
        switch (type) {
            case GL_DEBUG_TYPE_ERROR:               typeText = "Error"; break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeText = "Deprecated Behaviour"; break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeText = "Undefined Behaviour"; break;
            case GL_DEBUG_TYPE_PORTABILITY:         typeText = "Portability"; break;
            case GL_DEBUG_TYPE_PERFORMANCE:         typeText = "Performance"; break;
            case GL_DEBUG_TYPE_MARKER:              typeText = "Marker"; break;
            case GL_DEBUG_TYPE_PUSH_GROUP:          typeText = "Push Group"; break;
            case GL_DEBUG_TYPE_POP_GROUP:           typeText = "Pop Group"; break;
            case GL_DEBUG_TYPE_OTHER:               typeText = "Other"; break;
            default:                                typeText = "Unknown"; break;
        }
        switch (severity) {
            case GL_DEBUG_SEVERITY_HIGH:            severityText = "High"; break;
            case GL_DEBUG_SEVERITY_MEDIUM:          severityText = "Medium"; break;
            case GL_DEBUG_SEVERITY_LOW:             severityText = "Low"; break;
            case GL_DEBUG_SEVERITY_NOTIFICATION:    severityText = "Notification"; break;
            default:                                severityText = "Unknown"; break;
        }
        auto renderEngineLink = static_cast<RenderEngineLink *>(const_cast<void *>(userParam));
        renderEngineLink->log->log("OpenGL Error: " + sourceText + " produced a" + static_cast<std::string>(static_cast<std::string>("aeiouAEIOU").find(typeText[0]) ? "n " : " ") + typeText + " of " + severityText + " severity level which says: " + message, severity == (GL_DEBUG_SEVERITY_HIGH | GL_DEBUG_SEVERITY_MEDIUM) ? log4cplus::DEBUG_LOG_LEVEL : severity == (GL_DEBUG_SEVERITY_LOW) ? log4cplus::WARN_LOG_LEVEL : log4cplus::INFO_LOG_LEVEL, "Graphics Module");
        #endif
    }

    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
        auto renderEngine = (RenderEngine *)glfwGetWindowUserPointer(pWindow);
        renderEngine->renderEngineLink.settings.resolution[0] = width;
        renderEngine->renderEngineLink.settings.resolution[1] = height;
        renderEngine->handleWindowSizeChange();
    }
};