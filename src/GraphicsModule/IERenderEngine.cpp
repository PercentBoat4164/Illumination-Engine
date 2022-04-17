/* Include this file's header. */
#include "IERenderEngine.hpp"

/* Include dependencies within this module. */
#include "IERenderable.hpp"

/* Include dependencies from Core. */
#include "Core/LogModule/IELogger.hpp"
#include "Core/AssetModule/IEAsset.hpp"
#include "Core/IEWindowUser.hpp"

/* Include external dependencies. */
#define GLEW_IMPLEMENTATION  // Must precede GLEW inclusion.
#include <GL/glew.h>  // Not required by this file, but must be included before GLFW which is required.

#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

/* Include system dependencies. */
#include <filesystem>


vkb::Instance IERenderEngine::createVulkanInstance() {
    vkb::InstanceBuilder builder;

    // Set engine properties
    builder.set_engine_name(ILLUMINATION_ENGINE_NAME);
    builder.set_engine_version(ILLUMINATION_ENGINE_VERSION_MAJOR, ILLUMINATION_ENGINE_VERSION_MINOR, ILLUMINATION_ENGINE_VERSION_PATCH);

    // Set application properties
    builder.set_app_name(settings->applicationName.c_str());
    builder.set_app_version(settings->applicationVersion.number);

    // If debugging and components are available, add validation layers and a debug messenger
    #ifndef NDEBUG
    if (systemInfo->validation_layers_available) {
        builder.request_validation_layers();
    }
    if (systemInfo->debug_utils_available) {
        builder.use_default_debug_messenger();  /**@todo Make a custom messenger that uses the logging system.*/
    }
    #endif

    // Build the instance and check for errors.
    vkb::detail::Result<vkb::Instance> instanceBuilder = builder.build();
    if (!instanceBuilder) {
        settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create Vulkan instance. Error: " + instanceBuilder.error().message());
    }
    deletionQueue.insert(deletionQueue.begin(), [&] { vkb::destroy_instance(instance); });
    instance = instanceBuilder.value();
    return instance;
}

GLFWwindow *IERenderEngine::createWindow() const {
    // Specify all window hints for the window
    /**@todo Optional - Make a convenient way to change these programmatically based on some settings.*/
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* pWindow = glfwCreateWindow(settings->defaultWindowResolution[0], settings->defaultWindowResolution[1], settings->applicationName.c_str(), nullptr, nullptr);
    return pWindow;
}

void IERenderEngine::setWindowIcons(const std::string &path) const {
    int width;
    int height;
    int channels;
    std::vector<GLFWimage> icons{};
    std::string filename = path.substr(path.find_last_of('/'));  // filename component of path

    // iterate over all files and directories within path recursively
    for (const std::filesystem::directory_entry& file : std::filesystem::recursive_directory_iterator(path)) {
        if (path.find(filename) >= 0) {  // if filename to look for is in path
            stbi_uc* pixels = stbi_load(file.path().c_str(), &width, &height, &channels, STBI_rgb_alpha);  // Load image from disk
            if (pixels == nullptr) {
                settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Failed to load icon " + file.path().generic_string() + ". Is this file an image?");
            }
            icons.push_back(GLFWimage{.width=width, .height=height, .pixels=pixels});  // Generate image
        }
    }
    glfwSetWindowIcon(window, static_cast<int>(icons.size()), icons.data());  // Set icons
    for (GLFWimage icon : icons) {
        stbi_image_free(icon.pixels);  // Free all pixel data
    }
}

VkSurfaceKHR IERenderEngine::createWindowSurface() {
    if (glfwCreateWindowSurface(instance.instance, window, nullptr, &surface) != VK_SUCCESS) {
        settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create window surface!");
    }
    deletionQueue.insert(deletionQueue.begin(), [&] {
        vkb::destroy_surface(instance.instance, surface);
    });
    return surface;
}

vkb::Device IERenderEngine::setUpDevice(std::vector<std::vector<const char *>> *desiredExtensions, void *desiredExtensionFeatures) {
    vkb::PhysicalDeviceSelector selector{instance};
    // Note: The physical device selection stage is used to add extensions while the logical device building stage is used to add extension features.

    // Add desired extensions if any are listed.
    if (desiredExtensions != nullptr && !desiredExtensions->empty()) {
        selector.add_desired_extensions(*desiredExtensions->data());
    }

    // Set surface for physical device.
    vkb::detail::Result<vkb::PhysicalDevice> physicalDeviceBuilder = selector.set_surface(surface).select();

    // Prepare to build logical device
    vkb::DeviceBuilder logicalDeviceBuilder{physicalDeviceBuilder.value()};

    // Add extension features if any are listed.
    if (desiredExtensionFeatures != nullptr) {
        logicalDeviceBuilder.add_pNext(desiredExtensionFeatures);
    }

    // Build logical device.
    vkb::detail::Result<vkb::Device> logicalDevice = logicalDeviceBuilder.build();
    if (!logicalDevice) {
        // Failed? Report the error.
        settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create Vulkan device! Error: " + logicalDevice.error().message());
    }

    // get the device and add its destruction to the deletion queue.
    device = logicalDevice.value();
    deletionQueue.insert(deletionQueue.begin(), [&] {
        vkb::destroy_device(device);
    });
    return device;
}

VmaAllocator IERenderEngine::setUpGPUMemoryAllocator() {
    VmaAllocatorCreateInfo allocatorInfo{
            .physicalDevice = device.physical_device.physical_device,
            .device = device.device,
            .instance = instance.instance,
    };
    if (settings->rayTracing) {
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    }
//    allocatorInfo.vulkanApiVersion = api.version.number;
    vmaCreateAllocator(&allocatorInfo, &allocator);
    deletionQueue.insert(deletionQueue.begin(), [&] {
        vmaDestroyAllocator(allocator);
    });
    return allocator;
}

vkb::Swapchain IERenderEngine::createSwapchain(bool useOldSwapchain) {
    // Prepare the fullscreen
    if (settings->fullscreen) {
        handleFullscreenSettingsChange();
    }

    // Create swapchain builder
    vkb::SwapchainBuilder swapchainBuilder{device};
    swapchainBuilder.set_desired_present_mode(settings->vSync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR)
            .set_desired_extent(settings->resolution[0], settings->resolution[1])
            .set_desired_format({VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR})  // This may have to change in the event that HDR is to be supported.
            .set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    if (useOldSwapchain) {  // Use the old swapchain if it exists and that was requested.
        swapchainBuilder.set_old_swapchain(swapchain);
    }
    vkb::detail::Result<vkb::Swapchain> thisSwapchain = swapchainBuilder.build();
    if (!thisSwapchain) {
        // Failure! Log it then continue without deleting the old swapchain.
        settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create swapchain! Error: " + thisSwapchain.error().message());
    }
    else {
        // Success! Delete the old swapchain and images and replace them with the new ones.
        destroySwapchain();
        swapchain = thisSwapchain.value();
        swapchainImageViews = swapchain.get_image_views().value();
    }
    return swapchain;
}

void IERenderEngine::createSyncObjects() {
    // Avoid potential major issues when threading by waiting for the device to stop using any sync objects
    vkDeviceWaitIdle(device.device);

    // Prepare for creation
    VkSemaphoreCreateInfo semaphoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};

    // Ensure that the vectors have the appropriate size as to avoid errors in the next step.
    imageAvailableSemaphores.resize(swapchain.image_count);
    renderFinishedSemaphores.resize(swapchain.image_count);
    inFlightFences.resize(swapchain.image_count);
    imagesInFlight.resize(swapchain.image_count);

    // Create all the semaphores and fences.
    for (int i = 0; i < swapchain.image_count; ++i) {
        vkCreateSemaphore(device.device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]);
        vkCreateSemaphore(device.device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]);
        vkCreateFence(device.device, &fenceCreateInfo, nullptr, &inFlightFences[i]);
    }
}

void IERenderEngine::createCommandPools() {
    IECommandPool::CreateInfo commandPoolCreateInfo{};
    vkb::detail::Result<VkQueue> graphicsQueueDetails = device.get_queue(vkb::QueueType::graphics);
    if (graphicsQueueDetails.has_value()) {
        graphicsQueue = graphicsQueueDetails.value();
    }
    if (graphicsQueue != nullptr) {
        commandPoolCreateInfo.commandQueue = vkb::QueueType::graphics;
        graphicsCommandPool.create(this, &commandPoolCreateInfo);
    }
    vkb::detail::Result<VkQueue> presentQueueDetails = device.get_queue(vkb::QueueType::present);
    if (presentQueueDetails.has_value()) {
        presentQueue = presentQueueDetails.value();
    }
    if (presentQueue != nullptr) {
        commandPoolCreateInfo.commandQueue = vkb::QueueType::present;
        presentCommandPool.create(this, &commandPoolCreateInfo);
    }
    vkb::detail::Result<VkQueue> transferQueueDetails = device.get_queue(vkb::QueueType::transfer);
    if (transferQueueDetails.has_value()) {
        transferQueue = transferQueueDetails.value();
    }
    if (transferQueue != nullptr) {
        commandPoolCreateInfo.commandQueue = vkb::QueueType::transfer;
        transferCommandPool.create(this, &commandPoolCreateInfo);
    }
    vkb::detail::Result<VkQueue> computeQueueDetails = device.get_queue(vkb::QueueType::compute);
    if (computeQueueDetails.has_value()) {
        computeQueue = computeQueueDetails.value();
    }
    if (computeQueue != nullptr) {
        commandPoolCreateInfo.commandQueue = vkb::QueueType::compute;
        computeCommandPool.create(this, &commandPoolCreateInfo);
    }
}

void IERenderEngine::createRenderPass() {
    // Create the renderPass
    graphicsCommandPool.prepareCommandBuffers(swapchainImageViews.size());
    IERenderPass::CreateInfo renderPassCreateInfo {
            .msaaSamples=1
    };
    renderPass.create(this, &renderPassCreateInfo);
    deletionQueue.insert(deletionQueue.begin(), [&] {
        renderPass.destroy();
    });
}

IEAPI *IERenderEngine::autoDetectAPI() {
    api = IEAPI{IE_RENDER_ENGINE_API_NAME_VULKAN};
    api.version = api.getHighestSupportedVersion(this);
    return &api;
}

void IERenderEngine::buildFunctionPointers() {
    vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device.device, "vkGetBufferDeviceAddressKHR"));
    vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device.device, "vkCmdBuildAccelerationStructuresKHR"));
    vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device.device, "vkCreateAccelerationStructureKHR"));
    vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device.device, "vkDestroyAccelerationStructureKHR"));
    vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device.device, "vkGetAccelerationStructureBuildSizesKHR"));
    vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device.device, "vkGetAccelerationStructureDeviceAddressKHR"));
    vkAcquireNextImageKhr = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(device.device, "vkAcquireNextImageKHR"));
}

void IERenderEngine::destroySyncObjects() {
    for (uint32_t i = 0; i < swapchain.image_count; ++i) {
        vkDestroySemaphore(device.device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device.device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device.device, inFlightFences[i], nullptr);
    }
}

void IERenderEngine::destroySwapchain() {
    swapchain.destroy_image_views(swapchainImageViews);
    vkb::destroy_swapchain(swapchain);
}

void IERenderEngine::destroyCommandPools() {
    graphicsCommandPool.destroy();
    presentCommandPool.destroy();
    transferCommandPool.destroy();
    computeCommandPool.destroy();
}

IERenderEngine::IERenderEngine(IESettings *settings) {
    this->settings = settings;

    // Create a Vulkan instance
    createVulkanInstance();

    // Initialize GLFW then create and setup window
    /**@todo Clean up this section of the code as it is still quite messy. Optimally this would be done with a GUI abstraction.*/
    glfwInit();
    window = createWindow();
    setWindowIcons("res/icons");
    glfwSetWindowSizeLimits(window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwGetWindowPos(window, &settings->windowPosition[0], &settings->windowPosition[1]);
    glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, 0);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetWindowUserPointer(window, this);

    // Create surface
    createWindowSurface();

    // Set up the device
    std::vector<std::vector<const char *>> extensions{};
    if (settings->rayTracing) {
        extensions.push_back(extensionAndFeatureInfo.queryEngineFeatureExtensionRequirements(IE_ENGINE_FEATURE_RAY_QUERY_RAY_TRACING, &api));
        /**@todo Find a better way to handle specifying features. Perhaps use a similar method as was used for extensions.*/
        extensionAndFeatureInfo.accelerationStructureFeatures.accelerationStructure = VK_TRUE;
        extensionAndFeatureInfo.bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
        extensionAndFeatureInfo.rayQueryFeatures.rayQuery = VK_TRUE;
        extensionAndFeatureInfo.rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
    }
    setUpDevice(&extensions, extensionAndFeatureInfo.pNextHighestFeature);

    // Get API Version
    autoDetectAPI();

    // Build function pointers and generate queues
    buildFunctionPointers();

    // Set up GPU Memory allocator
    setUpGPUMemoryAllocator();

    // Create swapchain
    createSwapchain(false);
    deletionQueue.insert(deletionQueue.begin(), [&] {
        destroySwapchain();
    });

    // Create sync objects
    /**@todo Create an abstraction for sync objects if necessary.*/
    createSyncObjects();
    deletionQueue.insert(deletionQueue.begin(), [&] {
        destroySyncObjects();
    });

    // Create command pools
    createCommandPools();
    deletionQueue.insert(deletionQueue.begin(), [&] {
        destroyCommandPools();
    });

    // Create render pass
    createRenderPass();

    graphicsCommandPool[0].execute(nullptr);
    camera.create(this);
    settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, device.physical_device.properties.deviceName);
    settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, api.name + " v" + api.version.name);
    deletionQueue.insert(deletionQueue.begin(), [&] {
        topLevelAccelerationStructure.destroy();
    });
}

void IERenderEngine::addAsset(IEAsset *asset) {
    assets.push_back(asset);
    for (IEAspect *aspect : asset->aspects) {
        if (aspect->childType == IE_CHILD_TYPE_RENDERABLE) {
            loadRenderable((IERenderable*)aspect);
        }
    }
}

void IERenderEngine::loadRenderable(IERenderable *renderable) {
    // Create model buffer
    renderable->createModelBuffer();

    // Create vertex buffer
    renderable->createVertexBuffer();

    // Create index buffer
    renderable->createIndexBuffer();

    // Create mesh transformation buffer and acceleration structure if ray tracing

    // Create descriptor set
    renderable->createDescriptorSet();

    // Create shaders
    renderable->createShaders();

    // Create pipeline
    renderable->createPipeline();

    // Execute all commands to generate this renderable
    graphicsCommandPool[0].execute();

    // Record the destruction of this renderable
    renderableDeletionQueue.emplace_back([renderable] { renderable->destroy(); });
}

void IERenderEngine::handleResolutionChange() {
    createSwapchain(true);
    renderPass.destroy();
    createRenderPass();
}

bool IERenderEngine::update() {
    if (window == nullptr) {
        return false;
    }
    if (assets.empty()) {
        return glfwWindowShouldClose(window) != 1;
    }
    vkWaitForFences(device.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex{0};
    VkResult result = vkAcquireNextImageKhr(device.device, swapchain.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        handleResolutionChange();
        return glfwWindowShouldClose(window) != 1;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swapchain image!");
    }
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
        vkWaitForFences(device.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];
    VkDeviceSize offsets[] = {0};
    VkViewport viewport{
            .x = 0.0F,
            .y = 0.0F,
            .width = (float) swapchain.extent.width,
            .height = (float) swapchain.extent.height,
            .minDepth = 0.0F,
            .maxDepth = 1.0F
    };
    graphicsCommandPool[imageIndex].recordSetViewport(0, 1, &viewport);
    VkRect2D scissor{
        .offset = {0, 0},
        .extent = swapchain.extent,
    };
    graphicsCommandPool[imageIndex].recordSetScissor(0, 1, &scissor);
    IERenderPassBeginInfo renderPassBeginInfo = renderPass.beginRenderPass(imageIndex);
    graphicsCommandPool[imageIndex].recordBeginRenderPass(&renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    camera.update();
    auto time = static_cast<float>(glfwGetTime());
    if (settings->rayTracing) {
        topLevelAccelerationStructure.destroy();
        std::vector<VkDeviceAddress> bottomLevelAccelerationStructureDeviceAddresses{};
        bottomLevelAccelerationStructureDeviceAddresses.reserve(assets.size());
        if (settings->rayTracing) {
            for (IEAsset *asset: assets) {
                for (IEAspect *aspect: asset->aspects) {
                    if (aspect->childType == IE_CHILD_TYPE_RENDERABLE) {
                        auto *renderable = reinterpret_cast<IERenderable *>(aspect);
                        if (renderable->render) {
                            bottomLevelAccelerationStructureDeviceAddresses.push_back(renderable->bottomLevelAccelerationStructure.deviceAddress);
                        }
                    }
                }
            }
        }
        IEAccelerationStructure::CreateInfo topLevelAccelerationStructureCreateInfo{};
        topLevelAccelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        topLevelAccelerationStructureCreateInfo.transformationMatrix = &identityTransformMatrix;
        topLevelAccelerationStructureCreateInfo.primitiveCount = 1;
        topLevelAccelerationStructureCreateInfo.bottomLevelAccelerationStructureDeviceAddresses = bottomLevelAccelerationStructureDeviceAddresses;
        topLevelAccelerationStructure.create(this, &topLevelAccelerationStructureCreateInfo);
    }
    for (IEAsset *asset : assets) {
        for (IEAspect *aspect : asset->aspects) {
            if (aspect->childType == IE_CHILD_TYPE_RENDERABLE) {
                auto *renderable = reinterpret_cast<IERenderable *>(aspect);
                if (renderable->render) {
                    renderable->update(asset, camera, time);
                    if (settings->rayTracing) {
                        renderable->descriptorSet.update({&topLevelAccelerationStructure}, {2});
                    }
                    graphicsCommandPool[imageIndex].recordBindVertexBuffers(0, 1, {&renderable->vertexBuffer}, offsets);
                    graphicsCommandPool[imageIndex].recordBindIndexBuffer(&renderable->indexBuffer, 0, VK_INDEX_TYPE_UINT32);
                    graphicsCommandPool[imageIndex].recordBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, &renderable->pipeline);
                    graphicsCommandPool[imageIndex].recordBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, &renderable->pipeline, 0, {&renderable->descriptorSet}, {});
                    graphicsCommandPool[imageIndex].recordDrawIndexed(renderable->indices.size(), 1, 0, 0, 0);
                }
            }
        }
    }
    graphicsCommandPool[imageIndex].recordEndRenderPass();
    graphicsCommandPool[imageIndex].execute(imageAvailableSemaphores[currentFrame], renderFinishedSemaphores[currentFrame], inFlightFences[currentFrame]);
    graphicsCommandPool[imageIndex].wait();
    VkPresentInfoKHR presentInfo{
            .sType=VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &renderFinishedSemaphores[currentFrame],
            .swapchainCount = 1,
            .pSwapchains = &swapchain.swapchain,
            .pImageIndices = &imageIndex,
    };
    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    auto currentTime = (float)glfwGetTime();
    frameTime = currentTime - previousTime;
    previousTime = currentTime;
    frameNumber++;
    vkQueueWaitIdle(presentQueue);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        handleResolutionChange();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swapchain image!");
    }
    currentFrame = (currentFrame + 1) % (int)swapchain.image_count;
    if (frameTime > 1.0 / 30.0) {
        settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Frame #" + std::to_string(frameNumber) + " took " + std::to_string(frameTime) + "ms to compute.");
    }
    return glfwWindowShouldClose(window) != 1;
}

void IERenderEngine::handleFullscreenSettingsChange() {
    if (settings->fullscreen) {
        glfwGetWindowPos(window, settings->windowPosition.data(), &settings->windowPosition[1]);
        int monitorCount{};
        int windowX{};
        int windowY{};
        int windowWidth{};
        int windowHeight{};
        int monitorX{};
        int monitorY{};
        int monitorWidth;
        int monitorHeight;
        int bestMonitorWidth{};
        int bestMonitorHeight{};
        int bestMonitorRefreshRate{};
        int overlap;
        int bestOverlap{0};
        GLFWmonitor **monitors;
        const GLFWvidmode *mode;
        glfwGetWindowPos(window, &windowX, &windowY);
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        monitors = glfwGetMonitors(&monitorCount);
        for (int i = 0; i < monitorCount; i++) {
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
        settings->resolution = { bestMonitorWidth, bestMonitorHeight };
        glfwSetWindowMonitor(window, monitor, 0, 0, bestMonitorWidth, bestMonitorHeight, bestMonitorRefreshRate);
    } else {
        glfwSetWindowMonitor(window, nullptr, settings->windowPosition[0], settings->windowPosition[1], static_cast<int>(settings->defaultWindowResolution[0]), static_cast<int>(settings->defaultWindowResolution[1]), static_cast<int>(settings->refreshRate));
    }
    glfwSetWindowTitle(window, settings->applicationName.c_str());
}

void IERenderEngine::destroy() {
    destroySyncObjects();
    destroySwapchain();
    for (std::function<void()> &function : renderableDeletionQueue) {
        function();
    }
    for (std::function<void()> &function : recreationDeletionQueue) {
        function();
    }
    recreationDeletionQueue.clear();
    for (std::function<void()> &function : fullRecreationDeletionQueue) {
        function();
    }
    fullRecreationDeletionQueue.clear();
}

IERenderEngine::~IERenderEngine() {
    destroy();
}

void IERenderEngine::framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
    auto *renderEngine = (IERenderEngine *)((IEWindowUser *)glfwGetWindowUserPointer(pWindow))->IERenderEngine;
    renderEngine->framebufferResized = true;
    renderEngine->settings->resolution[0] = width;
    renderEngine->settings->resolution[1] = height;
}

std::string IERenderEngine::translateVkResultCodes(VkResult result) {
    switch (result) {
        case VK_SUCCESS:
            return "VK_SUCCESS";
        case VK_NOT_READY:
            return "VK_NOT_READY";
        case VK_TIMEOUT:
            return "VK_TIMEOUT";
        case VK_EVENT_SET:
            return "VK_EVENT_SET";
        case VK_EVENT_RESET:
            return "VK_EVENT_RESET";
        case VK_INCOMPLETE:
            return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:
            return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL:
            return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN:
            return "VK_ERROR_UNKNOWN";
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case VK_ERROR_FRAGMENTATION:
            return "VK_ERROR_FRAGMENTATION";
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
            return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case VK_PIPELINE_COMPILE_REQUIRED:
            return "VK_PIPELINE_COMPILE_REQUIRED";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR:
            return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV:
            return "VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case VK_ERROR_NOT_PERMITTED_KHR:
            return "VK_ERROR_NOT_PERMITTED_KHR";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case VK_THREAD_IDLE_KHR:
            return "VK_THREAD_IDLE_KHR";
        case VK_THREAD_DONE_KHR:
            return "VK_THREAD_DONE_KHR";
        case VK_OPERATION_DEFERRED_KHR:
            return "VK_OPERATION_DEFERRED_KHR";
        case VK_OPERATION_NOT_DEFERRED_KHR:
            return "VK_OPERATION_NOT_DEFERRED_KHR";
        case VK_RESULT_MAX_ENUM:
            return "VK_RESULT_MAX_ENUM";
        default:
            return "VK_ERROR_UNKNOWN";
    }
}

bool IERenderEngine::ExtensionAndFeatureInfo::queryFeatureSupport(const std::string &feature) {
    return (this->*extensionAndFeatureSupportQueries[feature])();
}

bool IERenderEngine::ExtensionAndFeatureInfo::queryExtensionSupport(const std::string &extension) {
    return std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end();
}

std::vector<const char *> IERenderEngine::ExtensionAndFeatureInfo::queryEngineFeatureExtensionRequirements(const std::string &engineFeature, IEAPI *API) {
    if (API->name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        return engineFeatureExtensionRequirementQueries[engineFeature][API->version.minor];
    }
    return {};
}

IERenderEngine::ExtensionAndFeatureInfo::ExtensionAndFeatureInfo() {
    uint32_t count;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> extensions(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
    supportedExtensions.reserve(count);
    for (VkExtensionProperties extension : extensions) {
        supportedExtensions.emplace_back(extension.extensionName);
    }
}

bool IERenderEngine::ExtensionAndFeatureInfo::rayTracingWithRayQuerySupportQuery() const {
    return (bufferDeviceAddressFeatures.bufferDeviceAddress & accelerationStructureFeatures.accelerationStructure & rayQueryFeatures.rayQuery & rayTracingPipelineFeatures.rayTracingPipeline) != 0U;
}

bool IERenderEngine::ExtensionAndFeatureInfo::variableDescriptorCountSupportQuery() const {
    return descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount != 0U;
}
