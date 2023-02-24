/* Include this file's header. */
#include "IERenderEngine.hpp"

/* Include dependencies within this module. */
#include "GraphicsModule/Renderable/IERenderable.hpp"

/* Include dependencies from Core. */
#include "Core/AssetModule/IEAsset.hpp"
#include "Core/Core.hpp"
#include "Core/LogModule/IELogger.hpp"

/* Include external dependencies. */
#define GLEW_IMPLEMENTATION
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION

#include <vk_mem_alloc.h>

#define STB_IMAGE_IMPLEMENTATION

#include <../contrib/stb/stb_image.h>

/* Include system dependencies. */
#include <filesystem>

vkb::Instance IERenderEngine::createVulkanInstance() {
    vkb::InstanceBuilder builder;

    // Set engine properties
    builder.set_engine_name(ILLUMINATION_ENGINE_NAME);
    builder.set_engine_version(
      ILLUMINATION_ENGINE_VERSION_MAJOR,
      ILLUMINATION_ENGINE_VERSION_MINOR,
      ILLUMINATION_ENGINE_VERSION_PATCH
    );

    // Set application properties
    builder.set_app_name(settings->applicationName.c_str());
    builder.set_app_version(settings->applicationVersion.number);

// If debugging and components are available, add validation layers and a debug messenger
#ifndef NDEBUG
    if (systemInfo->validation_layers_available) builder.request_validation_layers();
    if (systemInfo->debug_utils_available)
        builder.use_default_debug_messenger(); /**@todo Make a custom messenger that uses the logging system.*/
#endif

    // Build the instance and check for errors.
    vkb::Result<vkb::Instance> instanceBuilder = builder.build();
    if (!instanceBuilder) {
        settings->logger.log(

          "Failed to create Vulkan instance. Error: " + instanceBuilder.error().message(),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }
    deletionQueue.insert(deletionQueue.begin(), [&] { vkb::destroy_instance(instance); });
    instance = instanceBuilder.value();
    return instance;
}

GLFWwindow *IERenderEngine::createWindow() const {
    GLFWwindow *pWindow = glfwCreateWindow(
      settings->defaultResolution[0],
      settings->defaultResolution[1],
      settings->applicationName.c_str(),
      nullptr,
      nullptr
    );
    if (pWindow == nullptr) {
        const char *description;
        int         code = glfwGetError(&description);
        settings->logger.log(
          "Failed to create window! Error: " + std::to_string(code) + " " + description,
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    }
    int width, height;
    glfwGetFramebufferSize(pWindow, &width, &height);
    *settings->currentResolution = {width, height};
    IE::Core::Core::registerWindow(pWindow);
    IE::Core::Core::getWindow(pWindow)->graphicsEngine = const_cast<IERenderEngine *>(this);
    return pWindow;
}

void IERenderEngine::setWindowIcons(const std::filesystem::path &path) const {
    int                    width;
    int                    height;
    int                    channels;
    std::vector<GLFWimage> icons{};

    // iterate over all files and directories within path recursively
    for (const std::filesystem::directory_entry &file : std::filesystem::recursive_directory_iterator(path)) {
        stbi_uc *pixels = stbi_load(
          file.path().string().c_str(),
          &width,
          &height,
          &channels,
          STBI_rgb_alpha
        );  // Load image from disk
        if (pixels == nullptr) {
            settings->logger.log(

              "Failed to load icon " + file.path().generic_string() + ". Is this file an image?",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
            );
        }
        icons.push_back(GLFWimage{.width = width, .height = height, .pixels = pixels});  // Generate image
    }
    glfwSetWindowIcon(window, static_cast<int>(icons.size()), icons.data());  // Set icons
    for (GLFWimage icon : icons) stbi_image_free(icon.pixels);                // Free all pixel data
}

VkSurfaceKHR IERenderEngine::createWindowSurface() {
    if (!SDL_Vulkan_CreateSurface(window, instance.instance, &surface))
        settings->logger.log("Failed to create Vulkan surface for SDL window. Error:" + std::string(SDL_GetError()), IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_CRITICAL);
    deletionQueue.insert(deletionQueue.begin(), [&] { vkb::destroy_surface(instance.instance, surface); });
    return surface;
}

vkb::Device IERenderEngine::setUpDevice(
  std::vector<std::vector<const char *>> *desiredExtensions,
  void                                   *desiredExtensionFeatures
) {
    vkb::PhysicalDeviceSelector selector{instance};
    // Note: The physical device selection stage is used to add extensions while the logical device building stage
    // is used to add extension features.

    // Add desired extensions if any are listed.
    if (desiredExtensions != nullptr && !desiredExtensions->empty())
        selector.add_desired_extensions(*desiredExtensions->data());

    selector.prefer_gpu_device_type(vkb::PreferredDeviceType::discrete);

    // Set surface for physical device.
    vkb::Result<vkb::PhysicalDevice> physicalDeviceBuilder = selector.set_surface(surface).select();

    // Prepare to build logical device
    vkb::DeviceBuilder logicalDeviceBuilder{physicalDeviceBuilder.value()};

    // Add extension features if any are listed.
    if (desiredExtensionFeatures != nullptr) logicalDeviceBuilder.add_pNext(desiredExtensionFeatures);

    // Build logical device.
    vkb::Result<vkb::Device> logicalDevice = logicalDeviceBuilder.build();
    if (!logicalDevice) {
        // Failed? Report the error.
        settings->logger.log(
          "Failed to create Vulkan device! Error: " + logicalDevice.error().message(),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }

    // get the device and add its destruction to the deletion queue.
    device = logicalDevice.value();
    deletionQueue.insert(deletionQueue.begin(), [&] { vkb::destroy_device(device); });
    return device;
}

VmaAllocator IERenderEngine::setUpGPUMemoryAllocator() {
    VmaAllocatorCreateInfo allocatorInfo{
      .physicalDevice = device.physical_device.physical_device,
      .device         = device.device,
      .instance       = instance.instance,
    };
    if (settings->rayTracing) allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    //    allocatorInfo.vulkanApiVersion = api.version.number;
    vmaCreateAllocator(&allocatorInfo, &allocator);
    deletionQueue.insert(deletionQueue.begin(), [&] { vmaDestroyAllocator(allocator); });
    return allocator;
}

vkb::Swapchain IERenderEngine::createSwapchain(bool useOldSwapchain) {
    // Create swapchain builder
    vkb::SwapchainBuilder swapchainBuilder{device};
    swapchainBuilder
      .set_desired_present_mode(settings->vSync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR)
      .set_desired_extent((*settings->currentResolution)[0], (*settings->currentResolution)[1])
      .set_desired_format({VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR}
      )  // This may have to change in the event that HDR is to be supported.
      .set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
    if (useOldSwapchain)  // Use the old swapchain if it exists and its usage was requested.
        swapchainBuilder.set_old_swapchain(swapchain);
    vkb::Result<vkb::Swapchain> thisSwapchain = swapchainBuilder.build();
    if (!thisSwapchain) {
        // Failure! Log it then continue without deleting the old swapchain.
        settings->logger.log(

          "Failed to create swapchain! Error: " + thisSwapchain.error().message(),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    } else {
        // Success! Delete the old swapchain and images and replace them with the new ones.
        destroySwapchain();
        swapchain           = thisSwapchain.value();
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
    for (uint32_t i = 0; i < swapchain.image_count; ++i) {
        vkCreateSemaphore(device.device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]);
        vkCreateSemaphore(device.device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]);
        vkCreateFence(device.device, &fenceCreateInfo, nullptr, &inFlightFences[i]);
    }
}

void IERenderEngine::createCommandPools() {
    IECommandPool::CreateInfo commandPoolCreateInfo{};
    vkb::Result<VkQueue>      graphicsQueueDetails = device.get_queue(vkb::QueueType::graphics);
    if (graphicsQueueDetails.has_value()) graphicsQueue = graphicsQueueDetails.value();
    if (graphicsQueue != nullptr) {
        graphicsCommandPool                = std::make_shared<IECommandPool>();
        commandPoolCreateInfo.commandQueue = vkb::QueueType::graphics;
        graphicsCommandPool->create(this, &commandPoolCreateInfo);
    }
    vkb::Result<VkQueue> presentQueueDetails = device.get_queue(vkb::QueueType::present);
    if (presentQueueDetails.has_value()) presentQueue = presentQueueDetails.value();
    if (presentQueue != nullptr) {
        presentCommandPool                 = std::make_shared<IECommandPool>();
        commandPoolCreateInfo.commandQueue = vkb::QueueType::present;
        presentCommandPool->create(this, &commandPoolCreateInfo);
    }
    vkb::Result<VkQueue> transferQueueDetails = device.get_queue(vkb::QueueType::transfer);
    if (transferQueueDetails.has_value()) transferQueue = transferQueueDetails.value();
    if (transferQueue != nullptr) {
        transferCommandPool                = std::make_shared<IECommandPool>();
        commandPoolCreateInfo.commandQueue = vkb::QueueType::transfer;
        transferCommandPool->create(this, &commandPoolCreateInfo);
    }
    vkb::Result<VkQueue> computeQueueDetails = device.get_queue(vkb::QueueType::compute);
    if (computeQueueDetails.has_value()) computeQueue = computeQueueDetails.value();
    if (computeQueue != nullptr) {
        computeCommandPool                 = std::make_shared<IECommandPool>();
        commandPoolCreateInfo.commandQueue = vkb::QueueType::compute;
        computeCommandPool->create(this, &commandPoolCreateInfo);
    }
}

void IERenderEngine::createRenderPass() {
    // Create the renderPass
    renderPass = std::make_shared<IERenderPass>();
    graphicsCommandPool->prepareCommandBuffers(swapchainImageViews.size());
    IERenderPass::CreateInfo renderPassCreateInfo{.msaaSamples = 1};
    renderPass->create(this, &renderPassCreateInfo);
}

void IERenderEngine::setAPI(const IEAPI &API) {
    IERenderable::setAPI(API);
    IEMesh::setAPI(API);
    IEMaterial::setAPI(API);
    IEImage::setAPI(API);
    IEBuffer::setAPI(API);
    IEShader::setAPI(API);
    IEPipeline::setAPI(API);
    if (API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
        _update  = &IERenderEngine::_openGLUpdate;
        _destroy = &IERenderEngine::_openGLDestroy;
    } else if (API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        _update  = &IERenderEngine::_vulkanUpdate;
        _destroy = &IERenderEngine::_vulkanDestroy;
    } else {
        std::cout << "Attempt to set current Graphics API to an invalid API. Valid APIs: Vulkan, OpenGL"
                  << std::endl;
    }
}

IEAPI *IERenderEngine::autoDetectAPIVersion(const std::string &api) {
    API         = IEAPI{api};
    API.version = API.getHighestSupportedVersion(this);
    setAPI(API);
    return &API;
}

void IERenderEngine::buildFunctionPointers() {
    vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(
      vkGetDeviceProcAddr(device.device, "vkGetBufferDeviceAddressKHR")
    );
    vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(
      vkGetDeviceProcAddr(device.device, "vkCmdBuildAccelerationStructuresKHR")
    );
    vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(
      vkGetDeviceProcAddr(device.device, "vkCreateAccelerationStructureKHR")
    );
    vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(
      vkGetDeviceProcAddr(device.device, "vkDestroyAccelerationStructureKHR")
    );
    vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(
      vkGetDeviceProcAddr(device.device, "vkGetAccelerationStructureBuildSizesKHR")
    );
    vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(
      vkGetDeviceProcAddr(device.device, "vkGetAccelerationStructureDeviceAddressKHR")
    );
    vkAcquireNextImageKhr =
      reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(device.device, "vkAcquireNextImageKHR"));
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
    graphicsCommandPool->destroy();
    presentCommandPool->destroy();
    transferCommandPool->destroy();
    computeCommandPool->destroy();
}

IERenderEngine::IERenderEngine(IESettings *settings) : settings(settings) {
    // Create a Vulkan instance
    createVulkanInstance();

    // Initialize GLFW then create and setup window
    /**@todo Clean up this section of the code as it is still quite messy. Optimally this would be done with a GUI
     * abstraction.*/
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = createWindow();
    setWindowIcons("res/logos");
    glfwSetWindowSizeLimits(window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwGetWindowPos(window, &(*settings->currentPosition)[0], &(*settings->currentPosition)[1]);
    glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, 0);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetWindowPosCallback(window, windowPositionCallback);
    glfwSetWindowUserPointer(window, this);

    // Create surface
    createWindowSurface();

    // Set up the device
    std::vector<std::vector<const char *>> extensions{};
    if (settings->rayTracing) {
        extensions.push_back(extensionAndFeatureInfo.queryEngineFeatureExtensionRequirements(
          IE_ENGINE_FEATURE_RAY_QUERY_RAY_TRACING,
          &API
        ));
        /**@todo Find a better way to handle specifying features. Perhaps use a similar method as was used for
         * extensions.*/
        extensionAndFeatureInfo.accelerationStructureFeatures.accelerationStructure = VK_TRUE;
        extensionAndFeatureInfo.bufferDeviceAddressFeatures.bufferDeviceAddress     = VK_TRUE;
        extensionAndFeatureInfo.rayQueryFeatures.rayQuery                           = VK_TRUE;
        extensionAndFeatureInfo.rayTracingPipelineFeatures.rayTracingPipeline       = VK_TRUE;
    }
    setUpDevice(&extensions, extensionAndFeatureInfo.pNextHighestFeature);

    // Get API Version
    autoDetectAPIVersion(IE_RENDER_ENGINE_API_NAME_VULKAN);

    // Build function pointers and generate queues
    buildFunctionPointers();

    // Set up GPU Memory allocator
    setUpGPUMemoryAllocator();

    // Create swapchain
    createSwapchain(false);
    deletionQueue.insert(deletionQueue.begin(), [&] { destroySwapchain(); });

    // Create sync objects
    /**@todo Create an abstraction for sync objects if necessary.*/
    createSyncObjects();
    deletionQueue.insert(deletionQueue.begin(), [&] { destroySyncObjects(); });

    // Create command pools
    createCommandPools();
    deletionQueue.insert(deletionQueue.begin(), [&] { destroyCommandPools(); });

    IEImage::CreateInfo depthImageCreateInfo{
      .format          = VK_FORMAT_D32_SFLOAT_S8_UINT,
      .layout          = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
      .usage           = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      .aspect          = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
      .allocationUsage = VMA_MEMORY_USAGE_GPU_ONLY,
      .width           = swapchain.extent.width,
      .height          = swapchain.extent.height,
      .channels        = 1,
    };
    depthImage = std::make_shared<IEImage>(this, &depthImageCreateInfo);
    depthImage->uploadToVRAM();

    // Create render pass
    createRenderPass();
    deletionQueue.insert(deletionQueue.begin(), [&] { renderPass->destroy(); });

    graphicsCommandPool->index(0)->execute();
    camera.create(this);
    settings->logger.log(
      device.physical_device.properties.deviceName,
      IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_INFO
    );
    settings->logger.log(API.name + " v" + API.version.name, IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_INFO);
}

void IERenderEngine::addAsset(const std::shared_ptr<IEAsset> &asset) {
    for (std::shared_ptr<IEAspect> &aspect : asset->aspects) {
        // If aspect is downcast-able to a renderable
        if (dynamic_cast<IERenderable *>(aspect.get())) {
            renderables.push_back(std::dynamic_pointer_cast<IERenderable>(aspect));
            std::dynamic_pointer_cast<IERenderable>(aspect)->create(this, asset->filename);
            std::dynamic_pointer_cast<IERenderable>(aspect)->loadFromDiskToRAM();
            std::dynamic_pointer_cast<IERenderable>(aspect)->loadFromRAMToVRAM();
        }
    }
    if (API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) graphicsCommandPool->index(0)->execute();
}

void IERenderEngine::handleResolutionChange() {
    if (API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        createSwapchain();
        IEImage::CreateInfo depthImageCreateInfo{
          .format          = VK_FORMAT_D32_SFLOAT_S8_UINT,
          .layout          = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
          .usage           = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
          .aspect          = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
          .allocationUsage = VMA_MEMORY_USAGE_GPU_ONLY,
          .width           = swapchain.extent.width,
          .height          = swapchain.extent.height,
          .channels        = 1,
        };
        depthImage = std::make_shared<IEImage>(this, &depthImageCreateInfo);
        depthImage->uploadToVRAM();
        renderPass->destroy();
        createRenderPass();
    }
}

bool IERenderEngine::update() {
    return _update(*this);
}

bool IERenderEngine::_openGLUpdate() {
    if (framebufferResized) {
        framebufferResized = false;
        handleResolutionChange();
    }
    if (shouldBeFullscreen) {
        shouldBeFullscreen = false;
        toggleFullscreen();
        return glfwWindowShouldClose(window) != 1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.update();
    glViewport(0, 0, (*settings->currentResolution)[0], (*settings->currentResolution)[1]);
    for (const std::weak_ptr<IERenderable> &renderable : renderables) renderable.lock()->update(0);
    glfwSwapBuffers(window);
    auto currentTime = (float) glfwGetTime();
    frameTime        = currentTime - previousTime;
    previousTime     = currentTime;
    frameNumber++;
    return !glfwWindowShouldClose(window);
}

bool IERenderEngine::_vulkanUpdate() {
    if (window == nullptr) return false;
    if (renderables.empty()) return glfwWindowShouldClose(window) != 1;
    if (framebufferResized) {
        framebufferResized = false;
        handleResolutionChange();
    }
    if (shouldBeFullscreen) {
        shouldBeFullscreen = false;
        toggleFullscreen();
    }
    vkWaitForFences(device.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    uint32_t imageIndex{0};
    VkResult result = vkAcquireNextImageKhr(
      device.device,
      swapchain.swapchain,
      UINT64_MAX,
      imageAvailableSemaphores[currentFrame],
      VK_NULL_HANDLE,
      &imageIndex
    );
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) handleResolutionChange();
    if (imagesInFlight[imageIndex] != VK_NULL_HANDLE)
        vkWaitForFences(device.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    imagesInFlight[imageIndex] = inFlightFences[currentFrame];
    VkViewport viewport{
      .x        = 0.0F,
      .y        = 0.0F,
      .width    = (float) swapchain.extent.width,
      .height   = (float) swapchain.extent.height,
      .minDepth = 0.0F,
      .maxDepth = 1.0F};
    graphicsCommandPool->index(imageIndex)->recordSetViewport(0, 1, &viewport);
    VkRect2D scissor{
      .offset = {0, 0},
      .extent = swapchain.extent,
    };
    graphicsCommandPool->index(imageIndex)->recordSetScissor(0, 1, &scissor);
    IERenderPassBeginInfo renderPassBeginInfo = renderPass->beginRenderPass(imageIndex);
    graphicsCommandPool->index(imageIndex)
      ->recordBeginRenderPass(&renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    camera.update();
    for (const std::weak_ptr<IERenderable> &renderable : renderables) renderable.lock()->update(imageIndex);
    graphicsCommandPool->index(imageIndex)->recordEndRenderPass();
    graphicsCommandPool->index(imageIndex)
      ->execute(
        imageAvailableSemaphores[currentFrame],
        renderFinishedSemaphores[currentFrame],
        inFlightFences[currentFrame]
      );
    VkPresentInfoKHR presentInfo{
      .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .waitSemaphoreCount = 1,
      .pWaitSemaphores    = &renderFinishedSemaphores[currentFrame],
      .swapchainCount     = 1,
      .pSwapchains        = &swapchain.swapchain,
      .pImageIndices      = &imageIndex,
    };
    graphicsCommandPool->index(imageIndex)->commandPool->commandPoolMutex.lock();
    result = vkQueuePresentKHR(presentQueue, &presentInfo);
    graphicsCommandPool->index(imageIndex)->commandPool->commandPoolMutex.unlock();
    if (result != VK_SUCCESS && result != VK_ERROR_OUT_OF_DATE_KHR)
        settings->logger.log(
          "Failed to present image! Error: " + translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    currentFrame = (currentFrame + 1) % (int) swapchain.image_count;
    if (frameTime > 1.0 / 30.0) {
        settings->logger.log(
          "Frame #" + std::to_string(frameNumber) + " took " + std::to_string(frameTime * 1000) + "ms to compute.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    }
    auto currentTime = (float) glfwGetTime();
    frameTime        = currentTime - previousTime;
    previousTime     = currentTime;
    frameNumber++;
    return glfwWindowShouldClose(window) != 1;
}

void IERenderEngine::toggleFullscreen() {
    settings->fullscreen ^= true;
    if (settings->fullscreen) {
        int                monitorCount{};
        int                windowX{};
        int                windowY{};
        int                windowWidth{};
        int                windowHeight{};
        int                monitorX{};
        int                monitorY{};
        int                monitorWidth;
        int                monitorHeight;
        int                bestMonitorWidth{};
        int                bestMonitorHeight{};
        int                bestMonitorRefreshRate{};
        int                overlap;
        int                bestOverlap{0};
        GLFWmonitor      **monitors;
        const GLFWvidmode *mode;
        glfwGetWindowPos(window, &windowX, &windowY);
        glfwGetWindowSize(window, &windowWidth, &windowHeight);
        monitors = glfwGetMonitors(&monitorCount);
        for (int i = 0; i < monitorCount; ++i) {
            mode = glfwGetVideoMode(monitors[i]);
            glfwGetMonitorPos(monitors[i], &monitorX, &monitorY);
            monitorWidth  = mode->width;
            monitorHeight = mode->height;
            overlap =
              std::max(0, std::min(windowX + windowWidth, monitorX + monitorWidth) - std::max(windowX, monitorX)) *
              std::max(
                0,
                std::min(windowY + windowHeight, monitorY + monitorHeight) - std::max(windowY, monitorY)
              );
            if (bestOverlap < overlap) {
                bestOverlap            = overlap;
                monitor                = monitors[i];
                bestMonitorWidth       = monitorWidth;
                bestMonitorHeight      = monitorHeight;
                bestMonitorRefreshRate = mode->refreshRate;
            }
        }
        settings->refreshRate        = bestMonitorRefreshRate;
        settings->currentResolution  = &settings->fullscreenResolution;
        *settings->currentResolution = {bestMonitorWidth, bestMonitorHeight};
        glfwGetWindowPos(window, &(*settings->currentPosition)[0], &(*settings->currentPosition)[1]);
        settings->currentPosition = &settings->fullscreenPosition;
    } else {
        monitor                     = nullptr;
        settings->currentResolution = &settings->windowedResolution;
        settings->currentPosition   = &settings->windowedPosition;
    }
    glfwSetWindowMonitor(
      window,
      monitor,
      (*settings->currentPosition)[0],
      (*settings->currentPosition)[1],
      (*settings->currentResolution)[0],
      (*settings->currentResolution)[1],
      settings->refreshRate
    );
    handleResolutionChange();
    glfwSetWindowTitle(window, settings->applicationName.c_str());
}

void IERenderEngine::_vulkanDestroy() {
    for (const std::shared_ptr<IECommandBuffer> &commandBuffer : graphicsCommandPool->commandBuffers)
        commandBuffer->wait();
    destroySyncObjects();
    destroySwapchain();
    for (std::function<void()> &function : renderableDeletionQueue) function();
    for (std::function<void()> &function : recreationDeletionQueue) function();
    recreationDeletionQueue.clear();
    for (std::function<void()> &function : fullRecreationDeletionQueue) function();
    fullRecreationDeletionQueue.clear();
}

IERenderEngine::~IERenderEngine() {
    destroy();
}

void IERenderEngine::windowPositionCallback(GLFWwindow *pWindow, int x, int y) {
    auto *renderEngine = static_cast<IERenderEngine *>(IE::Core::Core::getWindow(pWindow)->graphicsEngine);
    *renderEngine->settings->currentPosition = {x, y};
}

void IERenderEngine::framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
    auto *renderEngine = static_cast<IERenderEngine *>(IE::Core::Core::getWindow(pWindow)->graphicsEngine);
    *renderEngine->settings->currentResolution = {width, height};
    renderEngine->framebufferResized           = true;
}

std::string IERenderEngine::translateVkResultCodes(VkResult result) {
    switch (result) {
        case VK_SUCCESS: return "VK_SUCCESS";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
        case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case VK_PIPELINE_COMPILE_REQUIRED: return "VK_PIPELINE_COMPILE_REQUIRED";
        case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case VK_ERROR_NOT_PERMITTED_KHR: return "VK_ERROR_NOT_PERMITTED_KHR";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
        case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
        case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
        case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
        case VK_RESULT_MAX_ENUM: return "VK_RESULT_MAX_ENUM";
        default: return "VK_ERROR_UNKNOWN";
    }
}

bool IERenderEngine::ExtensionAndFeatureInfo::queryFeatureSupport(const std::string &feature) {
    return (this->*extensionAndFeatureSupportQueries[feature])();
}

bool IERenderEngine::ExtensionAndFeatureInfo::queryExtensionSupport(const std::string &extension) {
    return std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) !=
      supportedExtensions.end();
}

std::vector<const char *> IERenderEngine::ExtensionAndFeatureInfo::queryEngineFeatureExtensionRequirements(
  const std::string &engineFeature,
  IEAPI             *API
) {
    if (API->name == IE_RENDER_ENGINE_API_NAME_VULKAN)
        return engineFeatureExtensionRequirementQueries[engineFeature][API->version.minor];
    return {};
}

IERenderEngine::ExtensionAndFeatureInfo::ExtensionAndFeatureInfo() {
    uint32_t count;
    vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> extensions(count);
    vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
    supportedExtensions.reserve(count);
    for (VkExtensionProperties extension : extensions) supportedExtensions.emplace_back(extension.extensionName);
}

bool IERenderEngine::ExtensionAndFeatureInfo::rayTracingWithRayQuerySupportQuery() const {
    return (bufferDeviceAddressFeatures.bufferDeviceAddress & accelerationStructureFeatures.accelerationStructure &
            rayQueryFeatures.rayQuery & rayTracingPipelineFeatures.rayTracingPipeline) != 0U;
}

bool IERenderEngine::ExtensionAndFeatureInfo::variableDescriptorCountSupportQuery() const {
    return descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount != 0U;
}

IERenderEngine::IERenderEngine(IESettings &t_settings) : settings(new IESettings{t_settings}) {
    // Initialize GLFW then create and setup window
    /**@todo Clean up this section of the code as it is still quite messy. Optimally this would be done with a GUI
     * abstraction.*/
    if (glfwInit() != GLFW_TRUE)
        settings->logger.log("Failed to initialize GLFW!", IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR);
    glfwWindowHint(GLFW_SAMPLES, 1);  // 1x MSAA (No MSAA)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
#ifndef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#endif
#ifndef NDEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // To make macOS happy.
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GL_TRUE);    // Use Core Profile by default.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif

    void *window = createWindow();

    setWindowIcons("res/logos");
    glfwSetWindowSizeLimits(window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwGetWindowPos(window, &(*settings->currentPosition)[0], &(*settings->currentPosition)[1]);
    glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, 0);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    glfwSetWindowPosCallback(window, windowPositionCallback);
    glfwSetWindowUserPointer(window, this);

    // Make context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(settings->vSync ? 1 : 0);

    // Initialize glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
        settings->logger.log("Failed to initialize GLEW!", IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR);

    // Get API Version
    autoDetectAPIVersion(IE_RENDER_ENGINE_API_NAME_OPENGL);

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);  // makes sure errors are displayed synchronous
#    ifndef __APPLE__
    glDebugMessageCallback(&IERenderEngine::glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#    endif
#endif

    camera.create(this);
    this->settings->logger.log(
      reinterpret_cast<const char *>(glGetString(GL_RENDERER)),
      IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_INFO
    );
    this->settings->logger.log(
      API.name + " v" + API.version.name + "@" +
        reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)),
      IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_INFO
    );
}

void APIENTRY IERenderEngine::
  glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei, const char *message, const void *) {
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;  // ignore these non-significant error codes
    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;
    switch (source) {
        case GL_DEBUG_SOURCE_API: std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM: std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY: std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION: std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER: std::cout << "Source: Other"; break;
        default: std::cout << "Source: Unknown"; break;
    }
    std::cout << std::endl;
    switch (type) {
        case GL_DEBUG_TYPE_ERROR: std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cout << "Type: Undefined Behaviour"; break;
        case GL_DEBUG_TYPE_PORTABILITY: std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER: std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP: std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP: std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER: std::cout << "Type: Other"; break;
        default: std::cout << "Type: Unknown"; break;
    }
    std::cout << std::endl;
    switch (severity) {
        case GL_DEBUG_SEVERITY_HIGH: std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW: std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
        default: std::cout << "Source: Unknown"; break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}

void IERenderEngine::destroy() {
    _destroy(*this);
}

void IERenderEngine::_openGLDestroy() {
    glFinish();
    glfwTerminate();
}

std::function<bool(IERenderEngine &)> IERenderEngine::_update =
  std::function<bool(IERenderEngine &)>{[](IERenderEngine &) {
      return true;
  }};
std::function<void(IERenderEngine &)> IERenderEngine::_destroy =
  std::function<void(IERenderEngine &)>{[](IERenderEngine &) {
      return;
  }};

IERenderEngine::AspectType *IERenderEngine::getAspect(const std::string &t_id) {
    return static_cast<AspectType *>(IE::Core::Engine::getAspect(t_id));
}

IERenderEngine::AspectType *IERenderEngine::createAspect(std::weak_ptr<IEAsset> t_asset, const std::string &t_id) {
    AspectType *aspect = getAspect(t_id);
    if (!aspect) aspect = new AspectType();
    t_asset.lock()->addAspect(aspect);
    return aspect;
}

void IERenderEngine::queueToggleFullscreen() {
    shouldBeFullscreen = !shouldBeFullscreen;
}
