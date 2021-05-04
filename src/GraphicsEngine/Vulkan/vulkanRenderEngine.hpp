#pragma once

#include "renderPassManager.hpp"
#include "vulkanGraphicsEngineLink.hpp"
#include "bufferManager.hpp"
#include "asset.hpp"
#include "gpuData.hpp"
#include "vulkanSettings.hpp"
#include "camera.hpp"
#include "commandBufferManager.hpp"

#include <VkBootstrap.h>
#define GLEW_IMPLEMENTATION
#include "../../../deps/glew/include/GL/glew.h"
#include <GLFW/glfw3.h>

#include <deque>
#include <functional>

//TODO: Add multithreading support throughout the engine - LOW PRIORITY
class VulkanRenderEngine {
private:
    vkb::Instance instance{};
    std::deque<std::function<void()>> engineDeletionQueue{};
    VmaAllocator allocator{};
    bool framebufferResized{false};
    VkSurfaceKHR surface{};
    std::deque<std::function<void()>> recreationDeletionQueue{};
    std::deque<std::function<void()>> oneTimeOptionalDeletionQueue{};
    std::vector<VkImageView> swapchainImageViews{};

protected:
    virtual bool update() { return false; }

    explicit VulkanRenderEngine(GLFWwindow *attachWindow = nullptr) {
        camera.settings = &settings;
        renderEngineLink.device = &device;
        renderEngineLink.physicalDeviceInfo = &physicalDeviceInfo;
        renderEngineLink.swapchain = &swapchain;
        renderEngineLink.settings = &settings;
        renderEngineLink.commandPool = &commandBufferManager.commandPool;
        renderEngineLink.allocator = &allocator;
        renderEngineLink.graphicsQueue = &graphicsQueue;
        vkb::detail::Result<vkb::SystemInfo> systemInfo = vkb::SystemInfo::get_system_info();
        //build instance
        vkb::InstanceBuilder builder;
        builder.set_app_name(settings.applicationName.c_str()).set_app_version(settings.applicationVersion[0], settings.applicationVersion[1], settings.applicationVersion[2]).require_api_version(settings.requiredVulkanVersion[0], settings.requiredVulkanVersion[1], settings.requiredVulkanVersion[2]);
        #ifndef _NDEBUG
        /* Validation layers hamper performance. Therefore to eek out extra speed from the GPU they will be turned off if the program is run in 'Release' mode. */
        if (systemInfo->validation_layers_available) { builder.request_validation_layers(); }
        #endif
        if (systemInfo->debug_utils_available) { builder.use_default_debug_messenger(); }
        vkb::detail::Result <vkb::Instance> inst_ret = builder.build();
        if (!inst_ret) { throw std::runtime_error("Failed to create Vulkan instance. Error: " + inst_ret.error().message() + "\n"); }
        instance = inst_ret.value();
        //build window
        engineDeletionQueue.emplace_front([&] { vkb::destroy_instance(instance); });
        if (attachWindow == nullptr) { glfwInit(); }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(settings.resolution[0], settings.resolution[1], settings.applicationName.c_str(), settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr, attachWindow);
        glfwSetWindowSizeLimits(window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
        int xPos{settings.windowPosition[0]}, yPos{settings.windowPosition[1]};
        glfwGetWindowPos(window, &xPos, &yPos);
        settings.windowPosition = {xPos, yPos};
        glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, 0);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetWindowUserPointer(window, this);
        if (glfwCreateWindowSurface(instance.instance, window, nullptr, &surface) != VK_SUCCESS) { throw std::runtime_error("failed to create window surface!"); }
        engineDeletionQueue.emplace_front([&] { vkDestroySurfaceKHR(instance.instance, surface, nullptr); });
        //select physical device
        vkb::PhysicalDeviceSelector selector{instance};
        std::vector<const char *> extensionNames{};
        if (settings.pathTracing) {
            extensionNames.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
            extensionNames.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
            extensionNames.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
            extensionNames.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
            extensionNames.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
            extensionNames.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
            extensionNames.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
        }
        VkPhysicalDeviceFeatures deviceFeatures{}; //require device features here
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;
        vkb::detail::Result<vkb::PhysicalDevice> phys_ret = selector.set_surface(surface).require_dedicated_transfer_queue().add_required_extensions(extensionNames).set_required_features(deviceFeatures).prefer_gpu_device_type(vkb::PreferredDeviceType::discrete).select();
        if (!phys_ret) { throw std::runtime_error("Failed to select Vulkan Physical Device. Error: " + phys_ret.error().message() + "\n"); }
        //create logical device
        vkb::DeviceBuilder device_builder{phys_ret.value()};
        if (settings.pathTracing) {
            VkPhysicalDeviceBufferDeviceAddressFeaturesEXT physicalDeviceBufferDeviceAddressFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES};
            physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
            VkPhysicalDeviceRayTracingPipelineFeaturesKHR physicalDeviceRayTracingPipelineFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR};
            physicalDeviceRayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
            VkPhysicalDeviceAccelerationStructureFeaturesKHR physicalDeviceAccelerationStructureFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
            physicalDeviceAccelerationStructureFeatures.accelerationStructure = VK_TRUE;
            vkb::detail::Result<vkb::Device> dev_ret = device_builder.add_pNext(&physicalDeviceBufferDeviceAddressFeatures).add_pNext(&physicalDeviceRayTracingPipelineFeatures).add_pNext(&physicalDeviceAccelerationStructureFeatures).build();
            if (!dev_ret) { throw std::runtime_error("Failed to create Vulkan device. Error: " + dev_ret.error().message() + "\n"); }
            device = dev_ret.value();
            engineDeletionQueue.emplace_front([&] { vkb::destroy_device(device); });
            //get physical device features and properties
            VkPhysicalDeviceProperties2 deviceProperties2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
            deviceProperties2.pNext = &physicalDeviceInfo.physicalDeviceRayTracingPipelineProperties;
            vkGetPhysicalDeviceProperties2(device.physical_device.physical_device, &deviceProperties2);
            VkPhysicalDeviceFeatures2 deviceFeatures2{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
            deviceFeatures2.pNext = &physicalDeviceInfo.physicalDeviceAccelerationStructureFeatures;
            vkGetPhysicalDeviceFeatures2(device.physical_device.physical_device, &deviceFeatures2);
        } else {
            vkb::detail::Result<vkb::Device> dev_ret = device_builder.build();
            if (!dev_ret) { throw std::runtime_error("Failed to create Vulkan device. Error: " + dev_ret.error().message() + "\n"); }
            device = dev_ret.value();
            engineDeletionQueue.emplace_front([&] { vkb::destroy_device(device); });
        }
        //get queues
        graphicsQueue = device.get_queue(vkb::QueueType::graphics).value();
        presentQueue = device.get_queue(vkb::QueueType::present).value();
        //create vma allocator
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = device.physical_device.physical_device;
        allocatorInfo.device = device.device;
        allocatorInfo.instance = instance.instance;
        allocatorInfo.flags = settings.pathTracing ? VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT : 0;
        vmaCreateAllocator(&allocatorInfo, &allocator);
        engineDeletionQueue.emplace_front([&] { vmaDestroyAllocator(allocator); });
        //Create commandPool
        commandBufferManager.setup(device, vkb::QueueType::graphics);
        engineDeletionQueue.emplace_front([&] { commandBufferManager.destroy(); });
        //delete scratch buffer
        scratchBuffer.setEngineLink(&renderEngineLink);
        engineDeletionQueue.emplace_front([&] { scratchBuffer.destroy(); });
        createSwapchain(true);
        renderEngineLink.build();
    }

    void createSwapchain(bool fullRecreate = false) {
        //Make sure no other GPU operations are ongoing
        vkDeviceWaitIdle(device.device);
        //Clear recreationDeletionQueue
        for (std::function<void()>& function : recreationDeletionQueue) { function(); }
        recreationDeletionQueue.clear();
        //Create swapchain
        vkb::SwapchainBuilder swapchainBuilder{ device };
        vkb::detail::Result<vkb::Swapchain> swap_ret = swapchainBuilder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR).set_desired_extent(settings.resolution[0], settings.resolution[1]).set_desired_format({settings.pathTracing ? VK_FORMAT_B8G8R8A8_UNORM : VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR}).build();
        if (!swap_ret) { throw std::runtime_error(swap_ret.error().message()); }
        swapchain = swap_ret.value();
        recreationDeletionQueue.emplace_front([&]{ vkb::destroy_swapchain(swapchain); });
        swapchainImageViews = swapchain.get_image_views().value();
        recreationDeletionQueue.emplace_front([&]{ swapchain.destroy_image_views(swapchainImageViews); });
        renderEngineLink.swapchainImageViews = &swapchainImageViews;
        //clear images marked as in flight
        imagesInFlight.clear();
        imagesInFlight.resize(swapchain.image_count, VK_NULL_HANDLE);
        //do the other stuff only if needed
        if (fullRecreate) {
            for (std::function<void()>& function : oneTimeOptionalDeletionQueue) { function(); }
            oneTimeOptionalDeletionQueue.clear();
            //Create commandBuffers
            commandBufferManager.createCommandBuffers((int)swapchain.image_count);
            //Create sync objects
            imageAvailableSemaphores.resize(swapchain.image_count);
            renderFinishedSemaphores.resize(swapchain.image_count);
            inFlightFences.resize(swapchain.image_count);
            VkSemaphoreCreateInfo semaphoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            for (unsigned int i = 0; i < swapchain.image_count; i++) {
                if (vkCreateSemaphore(device.device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create semaphores!"); }
                if (vkCreateSemaphore(device.device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create semaphores!"); }
                if (vkCreateFence(device.device, &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create fences!"); }
            }
            oneTimeOptionalDeletionQueue.emplace_front([&]{ for (VkSemaphore imageAvailableSemaphore : imageAvailableSemaphores) { vkDestroySemaphore(device.device, imageAvailableSemaphore, nullptr); } });
            oneTimeOptionalDeletionQueue.emplace_front([&]{ for (VkSemaphore renderFinishedSemaphore : renderFinishedSemaphores) { vkDestroySemaphore(device.device, renderFinishedSemaphore, nullptr); } });
            oneTimeOptionalDeletionQueue.emplace_front([&]{ for (VkFence inFlightFence : inFlightFences) { vkDestroyFence(device.device, inFlightFence, nullptr); } });
            //Create render pass
            renderPassManager.setup(&renderEngineLink);
            oneTimeOptionalDeletionQueue.emplace_front([&]{ renderPassManager.destroy(); });
            //re-upload assets
            for (Asset *asset : assets) { uploadAsset(asset, false); }
        }
        //recreate framebuffers
        renderPassManager.recreateFramebuffers();
        camera.update();
    }

    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
        auto vulkanRenderEngine = (VulkanRenderEngine *)glfwGetWindowUserPointer(pWindow);
        vulkanRenderEngine->framebufferResized = true;
        vulkanRenderEngine->settings.resolution[0] = width;
        vulkanRenderEngine->settings.resolution[1] = height;
    }

    vkb::Device device{};
    std::vector<VkFence> inFlightFences{};
    vkb::Swapchain swapchain{};
    std::vector<VkFence> imagesInFlight{};
    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};
    VkPipelineLayout pipelineLayout{};
    RenderPassManager renderPassManager{};
    VulkanGraphicsEngineLink renderEngineLink{};
    BufferManager scratchBuffer{};

public:
    virtual void uploadAsset(Asset *asset, bool append) {
        //destroy previously created asset if any
        asset->destroy();
        //upload mesh, vertex, and transformation data
        asset->vertexBuffer.setEngineLink(&renderEngineLink);
        memcpy(asset->vertexBuffer.create(sizeof(asset->vertices[0]) * asset->vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), asset->vertices.data(), sizeof(asset->vertices[0]) * asset->vertices.size());
        asset->deletionQueue.emplace_front([&](Asset thisAsset){ thisAsset.vertexBuffer.destroy(); });
        asset->indexBuffer.setEngineLink(&renderEngineLink);
        memcpy(asset->indexBuffer.create(sizeof(asset->indices[0]) * asset->indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), asset->indices.data(), sizeof(asset->indices[0]) * asset->indices.size());
        asset->deletionQueue.emplace_front([&](Asset thisAsset){ thisAsset.indexBuffer.destroy(); });
        if (settings.pathTracing) {
            asset->transformationBuffer.setEngineLink(&renderEngineLink);
            memcpy(asset->transformationBuffer.create(sizeof(asset->transformationMatrix), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_CPU_TO_GPU), &asset->transformationMatrix, sizeof(asset->transformationMatrix));
            asset->deletionQueue.emplace_front([&](Asset thisAsset) { thisAsset.transformationBuffer.destroy(); });
        }
        //upload textures
        asset->textureImages.resize(asset->textures.size());
        for (unsigned int i = 0; i < asset->textures.size(); ++i) {
            scratchBuffer.destroy();
            memcpy(scratchBuffer.create(asset->width * asset->height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), asset->textures[i], asset->width * asset->height * 4);
            asset->textureImages[i].setEngineLink(&renderEngineLink);
            asset->textureImages[i].create(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 1, asset->width, asset->height, TEXTURE, &scratchBuffer);
        }
        //build uniform buffers
        asset->uniformBuffer.setEngineLink(&renderEngineLink);
        memcpy(asset->uniformBuffer.create(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), &asset->uniformBufferObject, sizeof(UniformBufferObject));
        asset->deletionQueue.emplace_front([&](Asset thisAsset){ thisAsset.uniformBuffer.destroy(); });
        //build graphics pipeline and descriptor set for this asset
        asset->pipelineManager.setup(&renderEngineLink, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}, {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT}, swapchain.image_count, renderPassManager.renderPass, asset->shaderData);
        asset->pipelineManager.createDescriptorSet({asset->uniformBuffer}, {asset->textureImages[0]}, {BUFFER, IMAGE});
        asset->deletionQueue.emplace_front([&](Asset thisAsset) { thisAsset.pipelineManager.destroy(); });
        if (append) { assets.push_back(asset); }
    }

    void updateSettings(bool updateAll) {
        //TODO: Fix view jerk when exiting fullscreen. - LOW PRIORITY
        if (settings.fullscreen) {
            glfwGetWindowPos(window, &settings.windowPosition[0], &settings.windowPosition[1]);
            //find monitor that window is on
            int monitorCount{}, i, windowX{}, windowY{}, windowWidth{}, windowHeight{}, monitorX{}, monitorY{}, monitorWidth, monitorHeight, bestMonitorWidth{}, bestMonitorHeight{}, bestMonitorRefreshRate{}, overlap, bestOverlap{0};
            GLFWmonitor **monitors;
            const GLFWvidmode *mode;
            glfwGetWindowPos(window, &windowX, &windowY);
            glfwGetWindowSize(window, &windowWidth, &windowHeight);
            monitors = glfwGetMonitors(&monitorCount);
            for (i = 0; i < monitorCount; i++) {
                mode = glfwGetVideoMode(monitors[i]);
                glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);
                monitorWidth = mode->width;
                monitorHeight = mode->height;
                overlap = std::max(0, std::min(windowX + windowWidth, monitorX + monitorWidth) - std::max(windowX, monitorX)) * std::max(0, std::min(windowY + windowHeight, monitorY + monitorHeight) - std::max(windowY, monitorY));
                if (bestOverlap < overlap) {
                    bestOverlap = overlap;
                    monitor = monitors[i];
                    bestMonitorWidth = monitorWidth;
                    bestMonitorHeight = monitorHeight;
                    bestMonitorRefreshRate = mode->refreshRate;
                }
            }
            //put window in fullscreen on that monitor
            glfwSetWindowMonitor(window, monitor, 0, 0, bestMonitorWidth, bestMonitorHeight, bestMonitorRefreshRate);
        } else { glfwSetWindowMonitor(window, nullptr, settings.windowPosition[0], settings.windowPosition[1], settings.defaultWindowResolution[0], settings.defaultWindowResolution[1], settings.refreshRate); }
        glfwSetWindowTitle(window, settings.applicationName.c_str());
        if (updateAll) { createSwapchain(true); }
    }

    void cleanUp() {
        for (Asset *asset : assets) { asset->destroy(); }
        for (std::function<void()>& function : recreationDeletionQueue) { function(); }
        recreationDeletionQueue.clear();
        for (std::function<void()>& function : oneTimeOptionalDeletionQueue) { function(); }
        oneTimeOptionalDeletionQueue.clear();
        for (std::function<void()>& function : engineDeletionQueue) { function(); }
        engineDeletionQueue.clear();
    }

    GLFWmonitor *monitor{};
    VulkanSettings settings{};
    Camera camera{&settings};
    GLFWwindow *window{};
    std::vector<Asset *> assets{};
    CommandBufferManager commandBufferManager{};
    VulkanGraphicsEngineLink::PhysicalDeviceInfo physicalDeviceInfo{};
};