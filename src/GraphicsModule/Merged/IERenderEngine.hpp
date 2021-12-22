#pragma once

#include "Core/LogModule/IELogger.hpp"
#include "IERenderEngineLink.hpp"
#include "IECommandPool.hpp"
#include "IEBuffer.hpp"
#include "IEImage.hpp"
#include "IETexture.hpp"
#include "IEShader.hpp"
#include "IERenderPass.hpp"
#include "IERenderable.hpp"
#include "IEPipeline.hpp"
#include "IEGPUData.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#ifdef ILLUMINATION_ENGINE_VULKAN
#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#endif

#ifdef ILLUMINATION_ENGINE_OPENGL
#ifndef GLEW_IMPLEMENTATION
#define GLEW_IMPLEMENTATION
#include <GL/glew.h>
#endif
#endif
#include "GLFW/glfw3.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

class IERenderEngine {
public:
    IERenderEngineLink* renderEngineLink{};

    #ifdef ILLUMINATION_ENGINE_VULKAN
    std::vector<VkFence> inFlightFences{};
    std::vector<VkFence> imagesInFlight{};
    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    #endif
    IERenderPass renderPass{};
    std::vector<IEFramebuffer> framebuffers{};
    std::vector<IETexture> textures{};
    std::vector<IERenderable *> renderables{};
    IECommandPool renderCommandPool{};
    IEPipeline defaultPipeline{};
    IEDescriptorSet defaultDescriptorSet{};
    std::vector<IEShader> defaultShaders{};

    IERenderEngine(const std::string& API, IERenderEngineLink* engineLink) {
        renderEngineLink = engineLink;
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, "Creating window");
        renderEngineLink->api.name = API;
        renderEngineLink->textures = &textures;
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
            renderEngineLink->api.getVersion();
            vkb::detail::Result<vkb::SystemInfo> systemInfo = vkb::SystemInfo::get_system_info();
            vkb::InstanceBuilder builder;
            builder.set_engine_name("Illumination Engine");
            builder.set_app_name(renderEngineLink->settings.applicationName.c_str()).set_app_version(renderEngineLink->settings.applicationVersion.major, renderEngineLink->settings.applicationVersion.minor, renderEngineLink->settings.applicationVersion.patch).require_api_version(1, 1, 0);
            #ifndef NDEBUG
            if (systemInfo->validation_layers_available) { builder.request_validation_layers(); }
            if (systemInfo->debug_utils_available) { builder.use_default_debug_messenger(); }
            #endif
            vkb::detail::Result<vkb::Instance> instanceBuilder = builder.build();
            if (!instanceBuilder) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, "failed to create Vulkan instance: " + instanceBuilder.error().message() + "\n");
            }
            renderEngineLink->instance = instanceBuilder.value();
            renderEngineLink->created.instance = true;
        }
        #endif
        if (!glfwInit()) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "GLFW failed to initialize");
        }
        renderEngineLink->created.glfw = true;
    }

    void create() {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, "Initializing " + renderEngineLink->api.name + " API");
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) { glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (renderEngineLink->api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
            #ifdef __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
            #endif
            #ifndef NDEBUG
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
            #endif
        }
        #endif
        glfwMakeContextCurrent(nullptr);
        renderEngineLink->window = glfwCreateWindow(renderEngineLink->settings.resolution[0], renderEngineLink->settings.resolution[1], (renderEngineLink->settings.applicationName + " v" + renderEngineLink->settings.applicationVersion.name).c_str(), renderEngineLink->settings.monitor, nullptr);
        renderEngineLink->created.window = true;
        glfwMakeContextCurrent(renderEngineLink->window);
        int32_t width, height, channels, sizes[] = {256, 128, 64, 32, 16};
        GLFWimage icons[sizeof(sizes) / sizeof(int32_t)];
        for (uint64_t i = 0; i < sizeof(sizes) / sizeof(int32_t); ++i) {
            std::string filename = "res/Logos/IlluminationEngineLogo" + std::to_string(sizes[i]) + ".png";
            stbi_uc *pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Failed to prepare logo from file: " + filename);
            }
            icons[i].pixels = pixels;
            icons[i].height = height;
            icons[i].width = width;
        }
        glfwSetWindowIcon(renderEngineLink->window, sizeof(icons) / sizeof(GLFWimage), icons);
        for (GLFWimage icon: icons) {
            stbi_image_free(icon.pixels);
        }
        glfwSetWindowSizeLimits(renderEngineLink->window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwSetWindowAttrib(renderEngineLink->window, GLFW_AUTO_ICONIFY, 0);
        glfwSetWindowUserPointer(renderEngineLink->window, this);
        glfwSetFramebufferSizeCallback(renderEngineLink->window, framebufferResizeCallback);
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
            if (glfwCreateWindowSurface(renderEngineLink->instance.instance, renderEngineLink->window, nullptr, &renderEngineLink->surface) != VK_SUCCESS) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create window surface!");
            }
            renderEngineLink->created.surface = true;
            vkb::PhysicalDeviceSelector selector{renderEngineLink->instance};
            vkb::detail::Result<vkb::PhysicalDevice> temporaryPhysicalDeviceBuilder = selector.set_surface(renderEngineLink->surface).prefer_gpu_device_type(vkb::PreferredDeviceType::discrete).select();
            if (!temporaryPhysicalDeviceBuilder) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Physical device creation: " + temporaryPhysicalDeviceBuilder.error().message());
            }
            vkb::DeviceBuilder temporaryLogicalDeviceBuilder{temporaryPhysicalDeviceBuilder.value()};
            vkb::detail::Result<vkb::Device> temporaryLogicalDevice = temporaryLogicalDeviceBuilder.build();
            if (!temporaryLogicalDevice) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Logical device creation: " + temporaryLogicalDevice.error().message());
            }
            renderEngineLink->device = temporaryLogicalDevice.value();
            renderEngineLink->created.device = true;
            renderEngineLink->create();
            renderEngineLink->created.renderEngineLink = true;
            vkb::destroy_device(renderEngineLink->device);
            renderEngineLink->created.device = false;
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
                    reinterpret_cast<VkBool32 *>(&renderEngineLink->physicalDevice.enabledAPIComponents.anisotropicFiltering),
                    &renderEngineLink->physicalDevice.enabledAPIComponents.features.samplerAnisotropy
            };
            std::vector<VkBool32 *> msaaSmoothingFeatures{
                    reinterpret_cast<VkBool32 *>(&renderEngineLink->physicalDevice.enabledAPIComponents.msaaSmoothing),
                    &renderEngineLink->physicalDevice.enabledAPIComponents.features.sampleRateShading
            };
            std::vector<std::vector<VkBool32 *>> deviceFeatureGroups{
                    anisotropicFilteringFeatures,
                    msaaSmoothingFeatures
            };
            //EXTENSION FEATURE SELECTION
            //---------------------------
            std::vector<VkBool32 *> rayTracingFeatures{
                    (VkBool32*)&renderEngineLink->physicalDevice.enabledAPIComponents.rayTracing,
                    &renderEngineLink->physicalDevice.enabledAPIComponents.bufferDeviceAddressFeatures.bufferDeviceAddress,
                    &renderEngineLink->physicalDevice.enabledAPIComponents.accelerationStructureFeatures.accelerationStructure,
                    &renderEngineLink->physicalDevice.enabledAPIComponents.rayQueryFeatures.rayQuery,
                    &renderEngineLink->physicalDevice.enabledAPIComponents.rayTracingPipelineFeatures.rayTracingPipeline
            };
            std::vector<std::vector<VkBool32 *>> extensionFeatureGroups{
                    rayTracingFeatures
            };
            //===========================
            for (const std::vector<VkBool32 *> &deviceFeatureGroup : deviceFeatureGroups) {
                if (renderEngineLink->physicalDevice.testFeature(std::vector<VkBool32 *>(deviceFeatureGroup.begin() + 1, deviceFeatureGroup.end()))[0]) {
                    *deviceFeatureGroup[0] = VK_TRUE;
                    *(deviceFeatureGroup[0] - (VkBool32 *)&renderEngineLink->physicalDevice.enabledAPIComponents + (VkBool32 *)&renderEngineLink->physicalDevice.supportedAPIComponents) = VK_TRUE;
                    renderEngineLink->physicalDevice.enableFeature(deviceFeatureGroup);
                }
            }
            for (const std::vector<VkBool32 *> &extensionFeatureGroup : extensionFeatureGroups) {
                if (renderEngineLink->physicalDevice.testFeature(std::vector<VkBool32 *>(extensionFeatureGroup.begin() + 1, extensionFeatureGroup.end()))[0]) {
                    *extensionFeatureGroup[0] = VK_TRUE;
                    *(extensionFeatureGroup[0] - (VkBool32 *)&renderEngineLink->physicalDevice.enabledAPIComponents + (VkBool32 *)&renderEngineLink->physicalDevice.supportedAPIComponents) = VK_TRUE;
                    renderEngineLink->physicalDevice.enableFeature(extensionFeatureGroup);
                }
            }
            if (!extensionGroups.empty()) { selector.add_desired_extensions(*extensionGroups.data()); }
            vkb::detail::Result<vkb::PhysicalDevice> finalPhysicalDeviceBuilder = selector.set_surface(renderEngineLink->surface).set_required_features(renderEngineLink->physicalDevice.enabledAPIComponents.features).prefer_gpu_device_type(vkb::PreferredDeviceType::discrete).select();
            vkb::DeviceBuilder finalLogicalDeviceBuilder{finalPhysicalDeviceBuilder.value()};
            finalLogicalDeviceBuilder.add_pNext(renderEngineLink->physicalDevice.enabledAPIComponents.pNextHighestFeature);
            vkb::detail::Result<vkb::Device> finalLogicalDevice = finalLogicalDeviceBuilder.build();
            if (!finalLogicalDevice) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Failed to create final device after initial device was successfully created");
            }
            renderEngineLink->device = finalLogicalDevice.value();
            renderEngineLink->created.device = true;
            VmaAllocatorCreateInfo allocatorInfo{};
            allocatorInfo.physicalDevice = renderEngineLink->device.physical_device.physical_device;
            allocatorInfo.device = renderEngineLink->device.device;
            allocatorInfo.instance = renderEngineLink->instance.instance;
            allocatorInfo.flags = renderEngineLink->physicalDevice.enabledAPIComponents.rayTracing ? VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT : 0;
            vmaCreateAllocator(&allocatorInfo, &renderEngineLink->allocator);
            renderEngineLink->created.allocator = true;
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (renderEngineLink->api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
            glewInit();
            #if defined(_WIN32)
            glfwSwapInterval(renderEngineLink->settings.vSync ? 1 : 0);
            #else
            glfwSwapInterval(1);
            #endif
            glewExperimental = true;
            if (glewInit() != GLEW_OK) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Failed to initialize GLEW!");
            }
            renderEngineLink->created.glew = true;
            #ifndef NDEBUG
            int32_t flags;
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
        renderEngineLink->create();
        renderEngineLink->api.getVersion();
        renderEngineLink->physicalDevice.info.generate();
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, renderEngineLink->api.name + ' ' + renderEngineLink->api.version.name + " running on " + renderEngineLink->physicalDevice.info.name);
        handleWindowSizeChange();
    }

    bool update() {
        return !glfwWindowShouldClose(renderEngineLink->window);
        uint32_t imageIndex{};
        for (IERenderable *renderable : renderables) {
            if (renderable->render) {
                #ifdef ILLUMINATION_ENGINE_VULKAN
                if (renderEngineLink->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
                    vkCmdBindVertexBuffers(renderCommandPool[imageIndex], 0, 1, &std::get<VkBuffer>(renderable->meshes[0].vertexBuffer.buffer), nullptr);
                    vkCmdBindIndexBuffer(renderCommandPool[imageIndex], std::get<VkBuffer>(renderable->meshes[0].indexBuffer.buffer), 0, VK_INDEX_TYPE_UINT32);
                    vkCmdBindPipeline(renderCommandPool[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline.pipeline);
                    vkCmdBindDescriptorSets(renderCommandPool[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, defaultPipeline.pipelineLayout, 0, 1, &defaultDescriptorSet.descriptorSet, 0, nullptr);
                    vkCmdDrawIndexed(renderCommandPool[imageIndex], renderable->meshes[0].indices.size(), 1, 0, 0, 0);
                }
                #endif
            }
        }
    }

    void loadRenderable(IERenderable* renderable) {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
            IEBuffer::CreateInfo bufferCreateInfo{
                    .bufferSize=sizeof(IEGPUData::IEModelBuffer),
                    .usage=VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                    .allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU
            };
            renderable->modelBuffer.create(renderEngineLink, &bufferCreateInfo);
            IEGPUData::IEModelBuffer modelBufferData{};
            renderable->modelBuffer.upload(&modelBufferData, sizeof(glm::mat4));
            IEDescriptorSet::CreateInfo descriptorSetCreateInfo{
                    .poolSizes={{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}},
                    .shaderStages={static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)},
                    .data={&renderable->modelBuffer}
            };
            renderable->descriptorSet.create(renderEngineLink, &descriptorSetCreateInfo);
            IEPipeline::CreateInfo pipelineCreateInfo{
                    .shaders=defaultShaders,
                    .descriptorSet=&defaultDescriptorSet,
                    .renderPass=&renderPass
            };
        }
        #endif
    }

    void handleWindowSizeChange() {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
            vkDeviceWaitIdle(renderEngineLink->device.device);
            // clear recreation deletion queue
            vkb::SwapchainBuilder swapchainBuilder{renderEngineLink->device};
            vkb::detail::Result<vkb::Swapchain> swapchainBuilderResults = swapchainBuilder
                    .set_desired_present_mode(renderEngineLink->settings.vSync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR)
                    .set_desired_extent(renderEngineLink->settings.resolution[0], renderEngineLink->settings.resolution[1]).build();
            if (!swapchainBuilderResults) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Failed to create swapchain!");
            }
            destroy_swapchain(renderEngineLink->swapchain);
            renderEngineLink->swapchain = swapchainBuilderResults.value();
            renderEngineLink->created.swapchain = true;
            renderEngineLink->swapchainImageViews = renderEngineLink->swapchain.get_image_views().value();
            // Generate Framebuffer contentsString.
            std::vector<IEFramebuffer::CreateInfo> framebufferCreateInfos{renderEngineLink->swapchain.image_count};
            for (uint32_t i = 0; i < framebufferCreateInfos.size(); ++i) {
                framebufferCreateInfos[i] = {
                        .aspects=IE_FRAMEBUFFER_ASPECT_DEPTH_AND_COLOR,
                        .msaaSamples=renderEngineLink->settings.msaaSamples,
                        .swapchainImageView=renderEngineLink->swapchainImageViews[i],
                        .format=IE_IMAGE_FORMAT_SRGB_RGBA_8BIT,
                        .subpass=1
                };
            }
            IERenderPass::CreateInfo renderPassCreateInfo{.framebufferCreateInfos=framebufferCreateInfos};
            renderPass.create(renderEngineLink, &renderPassCreateInfo);
            framebuffers.resize(framebufferCreateInfos.size());
            for (uint32_t i = 0; i < framebuffers.size(); ++i) {
                framebuffers[i].create(renderEngineLink, &framebufferCreateInfos[i]);
                framebuffers[i].linkToRenderPass(&renderPass);
            }
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (renderEngineLink->api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {

        }
        #endif
    }

    void changeAPI(const std::string& API) {
        if (renderEngineLink->api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
            glFinish();
        }
        renderEngineLink->destroy();
        renderEngineLink->api.name = API;
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
            renderEngineLink->api.getVersion();
            vkb::detail::Result<vkb::SystemInfo> systemInfo = vkb::SystemInfo::get_system_info();
            vkb::InstanceBuilder builder;
            builder.set_app_name(renderEngineLink->settings.applicationName.c_str()).set_app_version(renderEngineLink->settings.applicationVersion.major, renderEngineLink->settings.applicationVersion.minor, renderEngineLink->settings.applicationVersion.patch).require_api_version(1, 1, 0);
            #ifndef NDEBUG
            if (systemInfo->validation_layers_available) {
                builder.request_validation_layers();
            }
            if (systemInfo->debug_utils_available) {
                builder.use_default_debug_messenger();
            }
            #endif
            vkb::detail::Result<vkb::Instance> instanceBuilder = builder.build();
            if (!instanceBuilder) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, instanceBuilder.error().message());
            }
            renderEngineLink->instance = instanceBuilder.value();
        }
        #endif
        create();
    }

    void closeWindow() const {
        glfwSetWindowShouldClose(renderEngineLink->window, 1);
    }

    static void destroy() {
        glFinish();
        glfwTerminate();
    }

    ~IERenderEngine() {
        destroy();
    }

private:
    static void APIENTRY glDebugOutput(GLenum source, GLenum type, uint32_t id, GLenum severity, GLsizei length, const char *message, const void *userParam) {
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return; // ignore these non-significant error codes
        std::string sourceText{};
        std::string typeText{};
        std::string severityText{};
        switch (source) {
            case GL_DEBUG_SOURCE_API:               sourceText = "IeAPI"; break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     sourceText = "Window System"; break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER:   sourceText = "IEShader Compiler"; break;
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
        auto renderEngineLink = static_cast<IERenderEngineLink *>(const_cast<void *>(userParam));
        IELogger::logDefault(severity == (GL_DEBUG_SEVERITY_HIGH | GL_DEBUG_SEVERITY_MEDIUM) ? ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG : severity == (GL_DEBUG_SEVERITY_LOW) ? ILLUMINATION_ENGINE_LOG_LEVEL_WARN : ILLUMINATION_ENGINE_LOG_LEVEL_INFO, "OpenGL Error: " + sourceText + " produced a" + static_cast<std::string>(static_cast<std::string>("aeiouAEIOU").find(typeText[0]) ? "n " : " ") + typeText + " of " + severityText + " severity level which says: " + message);
        #endif
    }

    static void framebufferResizeCallback(GLFWwindow *pWindow, int32_t width, int32_t height) {
        auto renderEngine = (IERenderEngine *)glfwGetWindowUserPointer(pWindow);
        renderEngine->renderEngineLink->settings.resolution[0] = width;
        renderEngine->renderEngineLink->settings.resolution[1] = height;
        renderEngine->handleWindowSizeChange();
    }
};