#pragma once

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#include <log4cplus/logger.h>
#include <log4cplus/loglevel.h>
#include <log4cplus/layout.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/consoleappender.h>

#include "RenderEngineLink.hpp"

class RenderEngine {
public:
    RenderEngineLink renderEngineLink;

    explicit RenderEngine(const std::string& API = "OpenGL") {
        log4cplus::SharedAppenderPtr consoleAppender{new log4cplus::ConsoleAppender()};
        consoleAppender->setName("consoleAppender");
        log4cplus::SharedAppenderPtr fileAppender{new log4cplus::FileAppender("IlluminationEngineGraphicsModule.log")};
        fileAppender->setName("fileAppender");
        std::unique_ptr<log4cplus::Layout> layout;
        layout = std::unique_ptr<log4cplus::Layout>(new log4cplus::SimpleLayout);
        consoleAppender->setLayout(reinterpret_cast<std::unique_ptr<log4cplus::Layout> &&>(layout));
        layout = std::unique_ptr<log4cplus::Layout>(new log4cplus::TTCCLayout);
        fileAppender->setLayout(reinterpret_cast<std::unique_ptr<log4cplus::Layout> &&>(layout));
        renderEngineLink.graphicsModuleLogger = log4cplus::Logger::getInstance("IlluminationEngineGraphicsModuleLogger");
        renderEngineLink.graphicsModuleLogger.addAppender(consoleAppender);
        renderEngineLink.graphicsModuleLogger.addAppender(fileAppender);
        renderEngineLink.graphicsModuleLogger.log(log4cplus::INFO_LOG_LEVEL, "Creating render engine...");
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
            if (!instanceBuilder) { renderEngineLink.graphicsModuleLogger.log(log4cplus::DEBUG_LOG_LEVEL, "Failed to create Vulkan instance. Error: " + instanceBuilder.error().message() + "\n"); }
            renderEngineLink.instance = instanceBuilder.value();
        }
        #endif
    }

    void create() {
        if (!glfwInit()) { renderEngineLink.graphicsModuleLogger.log(log4cplus::DEBUG_LOG_LEVEL, "GLFW failed to initialize"); }
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
        renderEngineLink.window = glfwCreateWindow(renderEngineLink.settings.resolution[0], renderEngineLink.settings.resolution[1], (renderEngineLink.settings.applicationName + " v" + renderEngineLink.settings.applicationVersion.name).c_str(), renderEngineLink.settings.monitor, nullptr);
        glfwMakeContextCurrent(renderEngineLink.window);
        int width, height, channels, sizes[] = {256, 128, 64, 32, 16};
        GLFWimage icons[sizeof(sizes) / sizeof(int)];
        for (unsigned long i = 0; i < sizeof(sizes) / sizeof(int); ++i) {
            std::string filename = "res/Logos/IlluminationEngineLogo" + std::to_string(sizes[i]) + ".png";
            stbi_uc *pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { renderEngineLink.graphicsModuleLogger.log(log4cplus::WARN_LOG_LEVEL, "failed to prepare texture image from file: " + filename); }
            icons[i].pixels = pixels;
            icons[i].height = height;
            icons[i].width = width;
        }
        glfwSetWindowIcon(renderEngineLink.window, sizeof(icons) / sizeof(GLFWimage), icons);
        for (GLFWimage icon: icons) { stbi_image_free(icon.pixels); }
        glfwSetWindowSizeLimits(renderEngineLink.window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwSetWindowAttrib(renderEngineLink.window, GLFW_AUTO_ICONIFY, 0);
        glfwSetFramebufferSizeCallback(renderEngineLink.window, framebufferResizeCallback);
        glfwSetWindowUserPointer(renderEngineLink.window, this);
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink.api.name == "Vulkan") {
            if (glfwCreateWindowSurface(renderEngineLink.instance.instance, renderEngineLink.window, nullptr, &renderEngineLink.surface) != VK_SUCCESS) { renderEngineLink.graphicsModuleLogger.log(log4cplus::DEBUG_LOG_LEVEL, "failed to create window surface"); }
            vkb::PhysicalDeviceSelector selector{renderEngineLink.instance};
            vkb::detail::Result<vkb::PhysicalDevice> temporaryPhysicalDeviceBuilder = selector.set_surface(renderEngineLink.surface).prefer_gpu_device_type(vkb::PreferredDeviceType::discrete).select();
            if (!temporaryPhysicalDeviceBuilder) { renderEngineLink.graphicsModuleLogger.log(log4cplus::DEBUG_LOG_LEVEL, "Physical device creation: " + temporaryPhysicalDeviceBuilder.error().message()); }
            vkb::DeviceBuilder temporaryLogicalDeviceBuilder{temporaryPhysicalDeviceBuilder.value()};
            vkb::detail::Result<vkb::Device> temporaryLogicalDevice = temporaryLogicalDeviceBuilder.build();
            if (!temporaryLogicalDevice) { renderEngineLink.graphicsModuleLogger.log(log4cplus::DEBUG_LOG_LEVEL, "Logical device creation: " + temporaryLogicalDevice.error().message()); }
            renderEngineLink.device = temporaryLogicalDevice.value();
            renderEngineLink.create();
            //EXTENSION SELECTION
            //-------------------
            std::vector<const char *> rayTracingExtensions{
                    VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                    VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                    VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                    VK_KHR_RAY_QUERY_EXTENSION_NAME,
                    VK_KHR_SPIRV_1_4_EXTENSION_NAME
            };
            std::vector<std::vector<const char *>> extensionGroups{
                    rayTracingExtensions
            };
            //DEVICE FEATURE SELECTION
            //------------------------
            std::vector<VkBool32 *> anisotropicFilteringFeatures{
                    reinterpret_cast<VkBool32 *>(&renderEngineLink.physicalDevice.enabledAPIComponents.anisotropicFiltering),
                    &renderEngineLink.physicalDevice.enabledAPIComponents.physicalDeviceFeatures.samplerAnisotropy
            };
            std::vector<VkBool32 *> msaaSmoothingFeatures{
                    reinterpret_cast<VkBool32 *>(&renderEngineLink.physicalDevice.enabledAPIComponents.msaaSmoothing),
                    &renderEngineLink.physicalDevice.enabledAPIComponents.physicalDeviceFeatures.sampleRateShading
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
        }
        #endif
        glewInit();
        renderEngineLink.create();
        renderEngineLink.api.getVersion();
        renderEngineLink.physicalDevice.info.generateInfo();
        renderEngineLink.graphicsModuleLogger.log(log4cplus::INFO_LOG_LEVEL, renderEngineLink.api.name + ' ' + renderEngineLink.api.version.name);
        renderEngineLink.graphicsModuleLogger.log(log4cplus::INFO_LOG_LEVEL, renderEngineLink.physicalDevice.info.name);
    }

    ~RenderEngine() {
        glfwTerminate();
    }

private:
    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
        auto renderEngine = (RenderEngine *)glfwGetWindowUserPointer(pWindow);
        renderEngine->renderEngineLink.framebufferResized = true;
        renderEngine->renderEngineLink.settings.resolution[0] = width;
        renderEngine->renderEngineLink.settings.resolution[1] = height;
    }
};