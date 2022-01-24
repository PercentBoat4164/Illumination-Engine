/**@todo Add multithreading support throughout the engine
 * - LOW PRIORITY: GPU is busy 95-98% of the time.
 * This will be a higher priority if GPU hits 80-90%.
 * Focus is going to be put on making all engines work before perfecting any of them.
 */

/**@todo Add proper invalid input checks to, and clean up, the abstractions.
 * - HIGH PRIORITY: This should happen before moving on to a new part of the game engine.
 * This has already been started in vulkanDescriptorSet.hpp.
 */

#pragma once

#include "Core/LogModule/IELogger.hpp"

#include "IERenderPass.hpp"
#include "IECommandPool.hpp"
#include "IEGraphicsLink.hpp"
#include "IEBuffer.hpp"
#include "IECamera.hpp"
#include "IERenderable.hpp"
#include "IEUniformBufferObject.hpp"
#include "IESettings.hpp"
#include "IETexture.hpp"
#include "IEFramebuffer.hpp"

#ifndef GLEW_IMPLEMENTATION
#define GLEW_IMPLEMENTATION
#include <GL/glew.h>
#endif
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif
#ifndef VMA_INCLUDED
#define VMA_INCLUDED
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#endif

#include <vulkan/vulkan.h>
#include <VkBootstrap.h>
#define GLFW_INCLUDE_VULKAN  // Needed for glfwCreateWindowSurface
#include <GLFW/glfw3.h>

#include <functional>
#include <optional>
#include <filesystem>

class IERenderEngine {
private:
    /**
     * @brief Private helper function that creates a Vulkan instance.
     * @return The newly created Vulkan instance.
     */
    vkb::Instance createVulkanInstance() {
        vkb::InstanceBuilder builder;

        // Set engine properties
        builder.set_engine_name(ILLUMINATION_ENGINE_NAME);
        builder.set_engine_version(ILLUMINATION_ENGINE_VERSION_MAJOR, ILLUMINATION_ENGINE_VERSION_MINOR, ILLUMINATION_ENGINE_VERSION_PATCH);

        // Set application properties
        builder.set_app_name(renderEngineLink.settings.applicationName.c_str());
        builder.set_app_version(renderEngineLink.settings.applicationVersion.number);

        // If debugging and components are available, add validation layers and a debug messenger
        #ifndef NDEBUG
        if (renderEngineLink.systemInfo->validation_layers_available) {
            builder.request_validation_layers();
        }
        if (renderEngineLink.systemInfo->debug_utils_available) {
            builder.use_default_debug_messenger();  /**@todo Make a custom messenger that uses the logging system.*/
        }
        #endif

        // Build the instance and check for errors.
        vkb::detail::Result<vkb::Instance> instanceBuilder = builder.build();
        if (!instanceBuilder) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create Vulkan instance. Error: " + instanceBuilder.error().message());
        }
        renderEngineLink.deletionQueue.insert(renderEngineLink.deletionQueue.cbegin(), [&] { vkb::destroy_instance(renderEngineLink.instance); });
        renderEngineLink.instance = instanceBuilder.value();
        return renderEngineLink.instance;
    }

    /**
     * @brief Creates a window using hardcoded hints and settings from renderEngineLink.settings.
     * @return The window that was just created.
     */
    GLFWwindow* createWindow() const {
        // Specify all window hints for the window
        /**todo Optional - Make a convenient way to change these programmatically based on some settings.*/
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* pWindow = glfwCreateWindow(renderEngineLink.settings.defaultWindowResolution[0], renderEngineLink.settings.defaultWindowResolution[1], renderEngineLink.settings.applicationName.c_str(), renderEngineLink.settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
        return pWindow;
    }

    /**
     * @brief Changes the window icon to all of the images found in [path] with a name that contains last part of [path].
     * @param path string of path to icons folder plus the identifier.
     */
    void setWindowIcons(const std::string& path) const {
        int width, height, channels;
        std::vector<GLFWimage> icons{};
        std::string filename = path.substr(path.find_last_of('/'));  // filename component of path

        // iterate over all files and directories within path recursively
        for (const std::filesystem::directory_entry& file : std::filesystem::recursive_directory_iterator(path)) {
            if (path.find(filename) >= 0) {  // if filename to look for is in path
                stbi_uc* pixels = stbi_load(file.path().c_str(), &width, &height, &channels, STBI_rgb_alpha);  // Load image from disk
                if (!pixels) {
                    IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Failed to load icon " + file.path().generic_string() + ". Is this file an image?");
                }
                icons.push_back(GLFWimage{.width=width, .height=height, .pixels=pixels});  // Generate image
            }
        }
        glfwSetWindowIcon(window, static_cast<int>(icons.size()), icons.data());  // Set icons
        for (GLFWimage icon : icons) {
            stbi_image_free(icon.pixels);  // Free all pixel data
        }
    }

    /**
     * @brief Creates a VkSurface for the window.
     * @return The newly created surface.
     */
    VkSurfaceKHR createWindowSurface() {
        if (glfwCreateWindowSurface(renderEngineLink.instance.instance, window, nullptr, &renderEngineLink.surface) != VK_SUCCESS) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create window surface!");
        }
        renderEngineLink.deletionQueue.insert(renderEngineLink.deletionQueue.cbegin(), [&] { vkb::destroy_surface(renderEngineLink.instance.instance, renderEngineLink.surface); });
        return renderEngineLink.surface;
    }

    /**
     * @brief Creates a physical and logical device with the extensions and features provided activated.
     * @param desiredExtensions vector of vectors of Vulkan extension names.
     * @param desiredExtensionFeatures pointer to pNext chain of desired features.
     * @return The newly created vkb::Device.
     */
    vkb::Device setUpDevice(std::vector<std::vector<const char *>>* desiredExtensions=nullptr, void* desiredExtensionFeatures=nullptr) {
        vkb::PhysicalDeviceSelector selector{renderEngineLink.instance};
        // Note: The physical device selection stage is used to add extensions while the logical device building stage is used to add extension features.

        // Add desired extensions if any are listed.
        if (desiredExtensions != nullptr && !desiredExtensions->empty()) {
            selector.add_desired_extensions(*desiredExtensions->data());
        }

        // Set surface for physical device.
        vkb::detail::Result<vkb::PhysicalDevice> physicalDeviceBuilder = selector.set_surface(renderEngineLink.surface).select();

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
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create Vulkan device! Error: " + logicalDevice.error().message());
        }

        // get the device and add its destruction to the deletion queue.
        renderEngineLink.device = logicalDevice.value();
        renderEngineLink.deletionQueue.insert(renderEngineLink.deletionQueue.cbegin(), [&] { vkb::destroy_device(renderEngineLink.device); });
        return renderEngineLink.device;
    }

    /**
     * @brief Creates a VmaAllocator with no flags activated.
     * @return The newly created VmaAllocator.
     */
    VmaAllocator setUpGPUMemoryAllocator () {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = renderEngineLink.device.physical_device.physical_device;
        allocatorInfo.device = renderEngineLink.device.device;
        allocatorInfo.instance = renderEngineLink.instance.instance;
        if (renderEngineLink.settings.rayTracing) {
            allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        }
//        allocatorInfo.vulkanApiVersion = renderEngineLink.api.version.number;
        vmaCreateAllocator(&allocatorInfo, &renderEngineLink.allocator);
        renderEngineLink.deletionQueue.insert(renderEngineLink.deletionQueue.cbegin(), [&] { vmaDestroyAllocator(renderEngineLink.allocator); });
        return renderEngineLink.allocator;
    }

    /**
     * @brief Creates a new swapchain. Uses the old swapchain if [useOldSwapchain] is true.
     * @param useOldSwapchain
     * @return The newly created swapchain.
     */
    vkb::Swapchain createSwapchain(bool useOldSwapchain=true) {
        // Create swapchain builder
        vkb::SwapchainBuilder swapchainBuilder{renderEngineLink.device};
        swapchainBuilder.set_desired_present_mode(renderEngineLink.settings.vSync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR)
                        .set_desired_extent(renderEngineLink.settings.resolution[0], renderEngineLink.settings.resolution[1])
                        .set_desired_format({VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR})  // This may have to change in the event that HDR is to be supported.
                        .set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        if (useOldSwapchain) {  // Use the old swapchain if it exists and that was requested.
            swapchainBuilder.set_old_swapchain(renderEngineLink.swapchain);
        }
        vkb::detail::Result<vkb::Swapchain> swapchain = swapchainBuilder.build();
        if (!swapchain) {
            // Failure! Log it then continue without deleting the old swapchain.
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create swapchain! Error: " + swapchain.error().message());
        }
        else {
            // Success! Delete the old swapchain and images and replace them with the new ones.
            destroySwapchain();
            renderEngineLink.swapchain = swapchain.value();
            renderEngineLink.swapchainImageViews = renderEngineLink.swapchain.get_image_views().value();
        }
        return renderEngineLink.swapchain;
    }

    /**
     * @brief Creates the basic sync objects. Recreates them if the already exist.
     */
    void createSyncObjects() {
        // Avoid potential major issues when threading by waiting for the device to stop using any sync objects
        vkDeviceWaitIdle(renderEngineLink.device.device);

        // Prepare for creation
        VkSemaphoreCreateInfo semaphoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};

        // Ensure that the vectors have the appropriate size as to avoid errors in the next step.
        imageAvailableSemaphores.resize(renderEngineLink.swapchain.image_count);
        renderFinishedSemaphores.resize(renderEngineLink.swapchain.image_count);
        inFlightFences.resize(renderEngineLink.swapchain.image_count);
        imagesInFlight.resize(renderEngineLink.swapchain.image_count);

        // Destroy all the semaphores and fences.
        for (int i = 0; i < renderEngineLink.swapchain.image_count; ++i) {
            vkCreateSemaphore(renderEngineLink.device.device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]);
            vkCreateSemaphore(renderEngineLink.device.device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]);
            vkCreateFence(renderEngineLink.device.device, &fenceCreateInfo, nullptr, &inFlightFences[i]);
        }
    }

    /**
     * @brief Initializes all the command pools in renderEngineLink. Each one is set to use its respective queue.
     */
    void createCommandPools() {
        renderEngineLink.computeCommandPool = &computeCommandPool;
        renderEngineLink.computeCommandPool->create(&renderEngineLink, new IECommandPool::CreateInfo{.commandQueue=vkb::QueueType::compute});
        renderEngineLink.graphicsCommandPool = &graphicsCommandPool;
        renderEngineLink.graphicsCommandPool->create(&renderEngineLink, new IECommandPool::CreateInfo{.commandQueue=vkb::QueueType::graphics});
        renderEngineLink.transferCommandPool = &transferCommandPool;
        renderEngineLink.transferCommandPool->create(&renderEngineLink, new IECommandPool::CreateInfo{.commandQueue=vkb::QueueType::transfer});
        renderEngineLink.presentCommandPool = &presentCommandPool;
        renderEngineLink.presentCommandPool->create(&renderEngineLink, new IECommandPool::CreateInfo{.commandQueue=vkb::QueueType::present});
    }

    /**
     * @brief Destroys all the sync objects required by the program.
     */
    void destroySyncObjects() {
        for (uint32_t i = 0; i < renderEngineLink.swapchain.image_count; ++i) {
            vkDestroySemaphore(renderEngineLink.device.device, imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(renderEngineLink.device.device, renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(renderEngineLink.device.device, inFlightFences[i], nullptr);
        }
    }

    /**
     * @brief Destroys the swapchain.
     */
    void destroySwapchain() {
        renderEngineLink.swapchain.destroy_image_views(renderEngineLink.swapchainImageViews);
        vkb::destroy_swapchain(renderEngineLink.swapchain);
    }

public:
    explicit IERenderEngine(IESettings* settings=nullptr) {
        renderEngineLink.settings = *settings;
        renderEngineLink.renderPass = &renderPass;
        renderEngineLink.textures = &textures;

        // Create a Vulkan instance
        createVulkanInstance();

        // Initialize GLFW then create and setup window
        /**@todo Clean up this section of the code as it is still quite messy. Optimally this would be done with a GUI abstraction.*/
        glfwInit();
        window = createWindow();
        setWindowIcons("res/icons");
        glfwSetWindowSizeLimits(window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
        glfwGetWindowPos(window, &renderEngineLink.settings.windowPosition[0], &renderEngineLink.settings.windowPosition[1]);
        glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, 0);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetWindowUserPointer(window, this);

        // Create surface
        createWindowSurface();

        // Set up the device
        renderEngineLink.api = IEGraphicsLink::IEAPI{IE_RENDER_ENGINE_API_NAME_VULKAN};
        std::vector<std::vector<const char *>> extensions{};
        if (renderEngineLink.settings.rayTracing) {
            extensions.push_back(renderEngineLink.extensionAndFeatureInfo.queryEngineFeatureExtensionRequirements(IE_ENGINE_FEATURE_RAY_QUERY_RAY_TRACING, &renderEngineLink.api));

            /**@todo Find a better way to handle specifying features. Perhaps use a similar method as was used for extensions.*/
            renderEngineLink.extensionAndFeatureInfo.accelerationStructureFeatures.accelerationStructure = VK_TRUE;
            renderEngineLink.extensionAndFeatureInfo.bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
            renderEngineLink.extensionAndFeatureInfo.rayQueryFeatures.rayQuery = VK_TRUE;
            renderEngineLink.extensionAndFeatureInfo.rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
        }
        setUpDevice(&extensions, renderEngineLink.extensionAndFeatureInfo.pNextHighestFeature);

        // Build function pointers and generate queues
        renderEngineLink.build();

        // Set up GPU Memory allocator
        setUpGPUMemoryAllocator();

        // Create swapchain
        createSwapchain(false);

        // Create sync objects
        /**@todo Create an abstraction for sync objects if necessary.*/
        createSyncObjects();

        // Create command pools
        createCommandPools();
        renderEngineLink.graphicsCommandPool->addCommandBuffers(renderEngineLink.swapchain.image_count);

        // Create the renderPass
        renderPass.create(&renderEngineLink);
        renderEngineLink.deletionQueue.insert(renderEngineLink.deletionQueue.cbegin(), [&]{
            renderPass.destroy();
        });

        // Create framebuffers
        IEFramebuffer::CreateInfo framebufferCreateInfo{};
        framebuffers.resize(renderEngineLink.swapchain.image_count);
        for (uint32_t i = 0; i < renderEngineLink.swapchain.image_count; ++i) {
            framebufferCreateInfo.renderPass = &renderPass;
            framebufferCreateInfo.swapchainImageView = (renderEngineLink.swapchainImageViews)[i];
            framebuffers[i].create(&renderEngineLink, &framebufferCreateInfo);
        }
        renderEngineLink.deletionQueue.insert(renderEngineLink.deletionQueue.cbegin(), [&]{
            for (IEFramebuffer& framebuffer : framebuffers) {
                framebuffer.destroy();
            }
        });

        camera.create(&renderEngineLink);
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, renderEngineLink.device.physical_device.properties.deviceName);
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, renderEngineLink.api.name + " v" +renderEngineLink.api.version.name);
        renderEngineLink.deletionQueue.insert(renderEngineLink.deletionQueue.cbegin(), [&] { topLevelAccelerationStructure.destroy(); });
    }

    void loadRenderable(IERenderable* renderable) {
        graphicsCommandPool.recordCommandBuffer(0);

        // Create Shaders
        renderable->createShaders(renderable->directory);

        // Create model buffer
        renderable->createModelBuffer();

        // Create vertex buffer
        renderable->createVertexBuffer();

        // Create index buffer
        renderable->createIndexBuffer();
        // Create mesh transformation buffer and acceleration structure if ray tracing

        // Create texture
        renderable->createTextures();

        // Create descriptor set
        renderable->createDescriptorSet();

        // Create pipeline
        renderable->createPipeline();
        graphicsCommandPool.executeCommandBuffer(0);
    }

//    void loadRenderable(IERenderable* renderable, bool append) {
//        renderable->destroy();
//        renderable->reloadRenderable(renderable->shaderNames);
//        renderable->shaders.resize(renderable->shaderCreateInfos.size());
//        for (int i = 0; i < renderable->shaderCreateInfos.size(); ++i) { renderable->shaders[i].create(&renderEngineLink, &renderable->shaderCreateInfos[i]); }
//        renderable->deletionQueue.emplace_back([&] (IERenderable *thisRenderable) { for (IEShader& shader : thisRenderable->shaders) { shader.destroy(); } });
//        IEBuffer::CreateInfo bufferCreateInfo{};
//        bufferCreateInfo.size = sizeof(IEUniformBufferObject);
//        bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
//        bufferCreateInfo.allocationUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
//        renderable->modelBuffer.create(&renderEngineLink, &bufferCreateInfo);
//        renderable->deletionQueue.emplace_back([&](IERenderable *thisRenderable){ thisRenderable->modelBuffer.destroy(); });
//        for (IERenderable::IEMesh& mesh : renderable->meshes) {
//            bufferCreateInfo.size = sizeof(mesh.vertices[0]) * mesh.vertices.size();
//            bufferCreateInfo.usage = renderEngineLink.settings.rayTracing ? VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
//            mesh.vertexBuffer.create(&renderEngineLink, &bufferCreateInfo);
//            mesh.vertexBuffer.uploadData(mesh.vertices.data(), sizeof(mesh.vertices[0]) * mesh.vertices.size());
//            mesh.deletionQueue.emplace_back([&] (IERenderable::IEMesh *thisMesh) { thisMesh->vertexBuffer.destroy(); });
//            bufferCreateInfo.size = sizeof(mesh.indices[0]) * mesh.indices.size();
//            bufferCreateInfo.usage ^= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
//            mesh.indexBuffer.create(&renderEngineLink, &bufferCreateInfo);
//            mesh.indexBuffer.uploadData(mesh.indices.data(), sizeof(mesh.indices[0]) * mesh.indices.size());
//            mesh.deletionQueue.emplace_back([&] (IERenderable::IEMesh *thisMesh) { thisMesh->indexBuffer.destroy(); });
//            if (renderEngineLink.settings.rayTracing) {
//                bufferCreateInfo.size = sizeof(mesh.transformationMatrix);
//                bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
//                mesh.transformationBuffer.create(&renderEngineLink, &bufferCreateInfo);
//                mesh.deletionQueue.emplace_back([&] (IERenderable::IEMesh *thisMesh) { thisMesh->transformationBuffer.destroy(); });
//                mesh.deletionQueue.emplace_back([&] (IERenderable::IEMesh *thisMesh) { thisMesh->bottomLevelAccelerationStructure.destroy(); });
//            }
//            for (IETexture &texture : renderable->textures) {
//                texture.destroy();
//                IETexture::CreateInfo textureCreateInfo{texture.createdWith};
//                textureCreateInfo.mipMapping = renderEngineLink.settings.mipMapping;
//                texture.create(&renderEngineLink, &textureCreateInfo);
//                texture.upload();
//            }
//            IEDescriptorSet::CreateInfo renderableDescriptorSetCreateInfo{};
//            if (renderEngineLink.settings.rayTracing) {
//                renderableDescriptorSetCreateInfo.poolSizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1}};
//                renderableDescriptorSetCreateInfo.shaderStages = {static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT), VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
//                renderableDescriptorSetCreateInfo.data = {&renderable->modelBuffer, &renderable->textures[mesh.diffuseTexture], std::nullopt};
//            } else {
//                renderableDescriptorSetCreateInfo.poolSizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};
//                renderableDescriptorSetCreateInfo.shaderStages = {static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT), VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
//                renderableDescriptorSetCreateInfo.data = {&renderable->modelBuffer, &renderable->textures[mesh.diffuseTexture], &renderable->textures[mesh.specularTexture]};
//            }
//            mesh.descriptorSet.create(&renderEngineLink, &renderableDescriptorSetCreateInfo);
//            mesh.deletionQueue.emplace_back([&] (IERenderable::IEMesh *thisMesh) { thisMesh->descriptorSet.destroy(); });
//            IEPipeline::CreateInfo renderablePipelineCreateInfo{};
//            renderablePipelineCreateInfo.descriptorSet = &mesh.descriptorSet;
//            renderablePipelineCreateInfo.renderPass = &renderPass;
//            renderablePipelineCreateInfo.shaders = &renderable->shaders;
//            mesh.pipeline.create(&renderEngineLink, &renderablePipelineCreateInfo);
//            mesh.deletionQueue.emplace_back([&] (IERenderable::IEMesh *thisMesh) { thisMesh->pipeline.destroy(); });
//        }
//        renderable->deletionQueue.emplace_back([&] (IERenderable *thisRenderable) { for (IETexture &texture : thisRenderable->textures) { texture.destroy(); } });
//        renderable->deletionQueue.emplace_back([&] (IERenderable *thisRenderable) { for (IERenderable::IEMesh &mesh : thisRenderable->meshes) { mesh.destroy(); } });
//        if (append) { renderables.push_back(renderable); }
//    }

    bool update() {
        if (window == nullptr) {
            return false;
        }
        if (renderables.empty()) {
            return glfwWindowShouldClose(window) != 1;
        }
        vkWaitForFences(renderEngineLink.device.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        uint32_t imageIndex{0};
        VkResult result = renderEngineLink.vkAcquireNextImageKhr(renderEngineLink.device.device, renderEngineLink.swapchain.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            /**@todo Handle window resize*/
            return glfwWindowShouldClose(window) != 1;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swapchain image!");
        }
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(renderEngineLink.device.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
        }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];
        VkDeviceSize offsets[] = {0};
        renderEngineLink.graphicsCommandPool->resetCommandBuffer((renderEngineLink.swapchain.image_count - 1 + imageIndex) % renderEngineLink.swapchain.image_count);
        renderEngineLink.graphicsCommandPool->recordCommandBuffer(imageIndex);
        VkViewport viewport{};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = (float)renderEngineLink.swapchain.extent.width;
        viewport.height = (float)renderEngineLink.swapchain.extent.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        vkCmdSetViewport((*renderEngineLink.graphicsCommandPool)[imageIndex], 0, 1, &viewport);
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = renderEngineLink.swapchain.extent;
        vkCmdSetScissor((*renderEngineLink.graphicsCommandPool)[imageIndex], 0, 1, &scissor);
        VkRenderPassBeginInfo renderPassBeginInfo = renderPass.beginRenderPass(framebuffers[imageIndex]);
        vkCmdBeginRenderPass((*renderEngineLink.graphicsCommandPool)[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        camera.update();
        auto time = static_cast<float>(glfwGetTime());
        for (IERenderable *renderable : renderables) { if (renderable->render) { renderable->update(camera, time); } }
        if (renderEngineLink.settings.rayTracing) {
            topLevelAccelerationStructure.destroy();
            std::vector<VkDeviceAddress> bottomLevelAccelerationStructureDeviceAddresses{};
            bottomLevelAccelerationStructureDeviceAddresses.reserve(renderables.size());
            for (IERenderable *renderable : renderables) {
                if (renderable->render & renderEngineLink.settings.rayTracing) {
                    bottomLevelAccelerationStructureDeviceAddresses.push_back(renderable->bottomLevelAccelerationStructure.deviceAddress);
                }
            }
            IEAccelerationStructure::CreateInfo topLevelAccelerationStructureCreateInfo{};
            topLevelAccelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            topLevelAccelerationStructureCreateInfo.transformationMatrix = &identityTransformMatrix;
            topLevelAccelerationStructureCreateInfo.primitiveCount = 1;
            topLevelAccelerationStructureCreateInfo.bottomLevelAccelerationStructureDeviceAddresses = bottomLevelAccelerationStructureDeviceAddresses;
            topLevelAccelerationStructure.create(&renderEngineLink, &topLevelAccelerationStructureCreateInfo);
        }
        for (IERenderable *renderable : renderables) {
            if (renderable->render) {
                    if (renderEngineLink.settings.rayTracing) {
                        renderable->descriptorSet.update({&topLevelAccelerationStructure}, {2});
                    }
                    vkCmdBindVertexBuffers((*renderEngineLink.graphicsCommandPool)[imageIndex], 0, 1, &renderable->vertexBuffer.buffer, offsets);
                    vkCmdBindIndexBuffer((*renderEngineLink.graphicsCommandPool)[imageIndex], renderable->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                    vkCmdBindPipeline((*renderEngineLink.graphicsCommandPool)[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipeline.pipeline);
                    vkCmdBindDescriptorSets((*renderEngineLink.graphicsCommandPool)[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipeline.pipelineLayout, 0, 1, &renderable->descriptorSet.descriptorSet, 0, nullptr);
                    vkCmdDrawIndexed((*renderEngineLink.graphicsCommandPool)[imageIndex], static_cast<uint32_t>(renderable->indices.size()), 1, 0, 0, 0);
            }
        }
        vkCmdEndRenderPass((*renderEngineLink.graphicsCommandPool)[imageIndex]);
        if (vkEndCommandBuffer((*renderEngineLink.graphicsCommandPool)[imageIndex]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record draw command IEBuffer!"); }
        VkSemaphore waitSemaphores[]{imageAvailableSemaphores[currentFrame]};
        VkSemaphore signalSemaphores[]{renderFinishedSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &(*renderEngineLink.graphicsCommandPool)[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        vkResetFences(renderEngineLink.device.device, 1, &inFlightFences[currentFrame]);
        VkResult test = vkQueueSubmit(renderEngineLink.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
        if ( test != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command IEBuffer!");
        }
        VkSwapchainKHR swapchains[]{renderEngineLink.swapchain.swapchain};
        VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;
        result = vkQueuePresentKHR(renderEngineLink.presentQueue, &presentInfo);
        auto currentTime = (float)glfwGetTime();
        frameTime = currentTime - previousTime;
        previousTime = currentTime;
        frameNumber++;
        vkQueueWaitIdle(renderEngineLink.presentQueue);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            createSwapchain(false);
        } else if (result != VK_SUCCESS) { throw std::runtime_error("failed to present swapchain image!"); }
        currentFrame = (currentFrame + 1) % (int)renderEngineLink.swapchain.image_count;
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, std::to_string(1/frameTime));
        return glfwWindowShouldClose(window) != 1;
    }

    void reloadRenderables() {
        for (IERenderable* renderable : renderables) {
            loadRenderable(renderable);
        }
    }

    void handleFullscreenSettingsChange() {
        if (renderEngineLink.settings.fullscreen) {
            glfwGetWindowPos(window, &renderEngineLink.settings.windowPosition[0], &renderEngineLink.settings.windowPosition[1]);
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
        } else { glfwSetWindowMonitor(window, nullptr, renderEngineLink.settings.windowPosition[0], renderEngineLink.settings.windowPosition[1], static_cast<int>(renderEngineLink.settings.defaultWindowResolution[0]), static_cast<int>(renderEngineLink.settings.defaultWindowResolution[1]), static_cast<int>(renderEngineLink.settings.refreshRate)); }
        glfwSetWindowTitle(window, renderEngineLink.settings.applicationName.c_str());
    }

    void destroy() {
        destroySyncObjects();
        destroySwapchain();
        for (IERenderable *renderable : renderables) { renderable->destroy(); }
        for (std::function<void()> &function : recreationDeletionQueue) { function(); }
        recreationDeletionQueue.clear();
        for (std::function<void()> &function : fullRecreationDeletionQueue) { function(); }
        fullRecreationDeletionQueue.clear();
    }

    ~IERenderEngine() {
        destroy();
    }

    GLFWmonitor *monitor{};
    GLFWwindow *window{};
    IECamera camera{};
    IERenderPass renderPass{};
    std::vector<IETexture> textures{1};
    IEGraphicsLink renderEngineLink{};
    IECommandPool graphicsCommandPool{};
    IECommandPool presentCommandPool{};
    IECommandPool transferCommandPool{};
    IECommandPool computeCommandPool{};
    float frameTime{};
    int frameNumber{};
    bool captureInput{};

private:
    VkTransformMatrixKHR identityTransformMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    std::vector<VkFence> inFlightFences{};
    std::vector<VkFence> imagesInFlight{};
    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<IEFramebuffer> framebuffers{};
    std::vector<IERenderable *> renderables{};
    IEAccelerationStructure topLevelAccelerationStructure{};
    std::vector<std::function<void()>> fullRecreationDeletionQueue{};
    std::vector<std::function<void()>> recreationDeletionQueue{};
    std::vector<std::function<void()>> deletionQueue{};
    size_t currentFrame{};
    bool framebufferResized{false};
    float previousTime{};

    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
        auto vulkanRenderEngine = (IERenderEngine *)glfwGetWindowUserPointer(pWindow);
        vulkanRenderEngine->framebufferResized = true;
        vulkanRenderEngine->renderEngineLink.settings.resolution[0] = width;
        vulkanRenderEngine->renderEngineLink.settings.resolution[1] = height;
    }
};