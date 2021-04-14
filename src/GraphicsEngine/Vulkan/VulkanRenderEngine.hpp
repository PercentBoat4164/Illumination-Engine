#pragma once

#include <deque>
#include <functional>
#include <vector>

#include <VkBootstrap.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "Asset.hpp"
#include "BufferManager.hpp"
#include "Camera.hpp"
#include "CommandBufferManager.hpp"
#include "GPUData.hpp"
#include "ImageManager.hpp"
#include "PipelineManager.hpp"
#include "RenderPassManager.hpp"
#include "Vertex.hpp"
#include "VulkanGraphicsEngineLink.hpp"

//TODO: Add multithreading support throughout the engine
class VulkanRenderEngine {
private:
    vkb::Instance instance{};
    std::deque<std::function<void()>> engineDeletionQueue{};
    VulkanGraphicsEngineLink renderEngineLink{};
    BufferManager stagingBuffer{};
    VmaAllocator allocator{};
    bool framebufferResized{false};
    VkSurfaceKHR surface{};
    std::deque<std::function<void()>> recreationDeletionQueue{};
    std::deque<std::function<void()>> oneTimeOptionalDeletionQueue{};
    std::vector<VkImageView> swapchainImageViews{};
    void *vulkan_library{};

protected:
    virtual bool update() { return false; }

    explicit VulkanRenderEngine(GLFWwindow *attachWindow = nullptr) {
        #if defined _WIN32
        vulkan_library = LoadLibrary("vulkan-1.dll");
        #else
        vulkan_library = dlopen("libvulkan.so.1", RTLD_NOW);
        #endif
        if( vulkan_library == nullptr ) { throw std::runtime_error("Could not connect with a Vulkan Runtime library."); }
        renderEngineLink.device = &device;
        renderEngineLink.swapchain = &swapchain;
        renderEngineLink.settings = &settings;
        renderEngineLink.commandPool = &commandBufferManager.commandPool;
        renderEngineLink.allocator = &allocator;
        framebufferResized = false;
        vkb::detail::Result<vkb::SystemInfo> systemInfo = vkb::SystemInfo::get_system_info();
        //build instance
        vkb::InstanceBuilder builder;
        builder.set_app_name(settings.applicationName.c_str()).set_app_version(settings.applicationVersion[0], settings.applicationVersion[1], settings.applicationVersion[2]).require_api_version(settings.requiredVulkanVersion[0], settings.requiredVulkanVersion[1], settings.requiredVulkanVersion[2]);
        if (systemInfo->validation_layers_available) { builder.request_validation_layers(); }
        if (systemInfo->debug_utils_available) { builder.use_default_debug_messenger(); }
        vkb::detail::Result <vkb::Instance> inst_ret = builder.build();
        if (!inst_ret) { throw std::runtime_error("Failed to create Vulkan instance. Error: " + inst_ret.error().message() + "\n"); }
        instance = inst_ret.value();
        //build window
        engineDeletionQueue.emplace_front([&] { vkb::destroy_instance(instance); });
        if (attachWindow == nullptr) { glfwInit(); }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(settings.resolution[0], settings.resolution[1], settings.applicationName.c_str(), settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr, attachWindow);
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
        for (const char *name : extensionNames) { std::cout << "Using extension: " << name << std::endl; }
        VkPhysicalDeviceFeatures deviceFeatures{}; //require device features here
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;
        vkb::detail::Result <vkb::PhysicalDevice> phys_ret = selector.set_surface(surface).require_dedicated_transfer_queue().add_desired_extensions(extensionNames).set_required_features(deviceFeatures).select();
        if (!phys_ret) { throw std::runtime_error("Failed to select Vulkan Physical Device. Error: " + phys_ret.error().message() + "\n"); }
        //create logical device
        VkPhysicalDeviceBufferDeviceAddressFeaturesEXT physicalDeviceBufferDeviceAddressFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES};
        physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
        vkb::DeviceBuilder device_builder{phys_ret.value()};
        vkb::detail::Result <vkb::Device> dev_ret = device_builder.add_pNext(&physicalDeviceBufferDeviceAddressFeatures).build();
        if (!dev_ret) { throw std::runtime_error("Failed to create Vulkan device. Error: " + dev_ret.error().message() + "\n"); }
        device = dev_ret.value();
        engineDeletionQueue.emplace_front([&] { vkb::destroy_device(device); });
        //get queues
        graphicsQueue = device.get_queue(vkb::QueueType::graphics).value();
        presentQueue = device.get_queue(vkb::QueueType::present).value();
        //create vma allocator
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = device.physical_device.physical_device;
        allocatorInfo.device = device.device;
        allocatorInfo.instance = instance.instance;
        vmaCreateAllocator(&allocatorInfo, &allocator);
        engineDeletionQueue.emplace_front([&] { vmaDestroyAllocator(allocator); });
        //Create commandPool
        commandBufferManager.setup(device, vkb::QueueType::graphics);
        engineDeletionQueue.emplace_front([&] { commandBufferManager.destroy(); });
        //delete staging buffer
        stagingBuffer.setEngineLink(&renderEngineLink);
        engineDeletionQueue.emplace_front([&] { stagingBuffer.destroy(); });
        createSwapchain(true);
    }

    void createSwapchain(bool fullRecreate = false) {
        //Make sure no other GPU operations are on-going
        vkDeviceWaitIdle(device.device);
        //Clear recreationDeletionQueue
        for (std::function<void()>& function : recreationDeletionQueue) { function(); }
        recreationDeletionQueue.clear();
        //Create swapchain
        vkb::SwapchainBuilder swapchainBuilder{ device };
        vkb::detail::Result<vkb::Swapchain> swap_ret = swapchainBuilder.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).build();
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
        //update camera
        camera.resolution = settings.resolution;
        camera.fov = settings.fov;
        camera.renderDistance = settings.renderDistance;
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

public:
    virtual void uploadAsset(Asset *asset, bool append) {
        //destroy previously created asset if any
        asset->destroy();
        //upload mesh and vertex data
        asset->vertexBuffer.setEngineLink(&renderEngineLink);
        memcpy(asset->vertexBuffer.create(sizeof(asset->vertices[0]) * asset->vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), asset->vertices.data(), sizeof(asset->vertices[0]) * asset->vertices.size());
        asset->indexBuffer.setEngineLink(&renderEngineLink);
        memcpy(asset->indexBuffer.create(sizeof(asset->indices[0]) * asset->indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), asset->indices.data(), sizeof(asset->indices[0]) * asset->indices.size());
        //upload textures
        asset->textureImages.resize(asset->textures.size());
        for (unsigned int i = 0; i < asset->textures.size(); ++i) {
            stagingBuffer.destroy();
            memcpy(stagingBuffer.create(asset->width * asset->height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), asset->textures[i], asset->width * asset->height * 4);
            asset->textureImages[i].setEngineLink(&renderEngineLink);
            asset->textureImages[i].create(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 1, asset->width, asset->height, TEXTURE, &stagingBuffer);
        }
        //build uniform buffers
        asset->uniformBuffer.setEngineLink(&renderEngineLink);
        memcpy(asset->uniformBuffer.create(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), &asset->uniformBufferObject, sizeof(UniformBufferObject));
        //build graphics pipeline and descriptor set for this asset
        asset->pipelineManagers.resize(1);
        for (unsigned int i = 0; i < asset->pipelineManagers.size(); ++i) {
            asset->pipelineManagers[i].setup(&renderEngineLink, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}, {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT}, swapchain.image_count, renderPassManager.renderPass, asset->shaderData);
            asset->pipelineManagers[0].createDescriptorSet({asset->uniformBuffer}, {asset->textureImages[0]}, {BUFFER, IMAGE});
        }
        asset->deletionQueue.emplace_front([&](const Asset& thisAsset){ for (PipelineManager pipelineManager : thisAsset.pipelineManagers) { pipelineManager.destroy(); } });
        if (append) { assets.push_back(asset); }
    }

    void updateSettings(bool updateAll) {
        //TODO: Fix fov scaling on fullscreen bug.
        //TODO: Fix view jerk when exiting fullscreen.
        if (settings.fullscreen) {
            glfwGetWindowPos(window, &settings.windowPosition[0], &settings.windowPosition[1]);
            //find monitor that window is on.
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
            glfwSetWindowMonitor(window, monitor, 0, 0, bestMonitorWidth, bestMonitorHeight, bestMonitorRefreshRate);
        }
        else {
            glfwSetWindowMonitor(window, nullptr, settings.windowPosition[0], settings.windowPosition[1], settings.defaultWindowResolution[0], settings.defaultWindowResolution[1], settings.refreshRate);
        }
        glfwSetWindowTitle(window, settings.applicationName.c_str());
        renderEngineLink.settings = &settings;
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
        #if defined _WIN32
        FreeLibrary( vulkan_library );
        #else
        dlclose( vulkan_library );
        #endif
        vulkan_library = nullptr;
    }

    GLFWmonitor *monitor{};
    Settings settings{};
    Camera camera{};
    GLFWwindow *window{};
    std::vector<Asset *> assets{};
    CommandBufferManager commandBufferManager{};
};