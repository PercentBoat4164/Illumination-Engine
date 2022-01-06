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
#include "IEGraphicsLink.hpp"
#include "IEBuffer.hpp"
#include "IECamera.hpp"
#include "IERenderable.hpp"
#include "IEUniformBufferObject.hpp"
#include "IESettings.hpp"
#include "IECommandBuffer.hpp"
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

#include <deque>
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
        builder.set_app_name(renderEngineLink.settings->applicationName.c_str());
        builder.set_app_version(renderEngineLink.settings->applicationVersion.number);

        // If debugging and components are available, add validation layers and a debug messenger
        #ifndef NDEBUG
        if (renderEngineLink.systemInfo->validation_layers_available) {
            builder.request_validation_layers();
        }
        if (renderEngineLink.systemInfo->debug_utils_available) {
            builder.use_default_debug_messenger();  //*@todo Make a custom messenger that uses the logging system.*/
        }
        #endif

        // Build the instance and check for errors.
        vkb::detail::Result<vkb::Instance> instanceBuilder = builder.build();
        if (!instanceBuilder) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create Vulkan instance. Error: " + instanceBuilder.error().message());
        }
        return instanceBuilder.value();
    }

    [[nodiscard]] GLFWwindow* createWindow() const {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* pWindow = glfwCreateWindow(static_cast<int>(renderEngineLink.settings->resolution[0]), static_cast<int>(renderEngineLink.settings->resolution[1]), renderEngineLink.settings->applicationName.c_str(), renderEngineLink.settings->fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
        return pWindow;
    }

    void setWindowIcons(const std::string& path) const {
        int width, height, channels;
        std::vector<GLFWimage> icons{};
        std::string filename = path.substr(path.find_last_of('/'));  // filename component of path

        // iterate over all files and directories within path recursively
        for (const std::filesystem::directory_entry& file : std::filesystem::recursive_directory_iterator(path)) {
            if (path.find(filename) >= 0) {  // if filename to look for is in path
                stbi_uc* pixels = stbi_load(file.path().c_str(), &width, &height, &channels, STBI_rgb_alpha);  // Load image from disk
                if (!pixels) {
                    IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Failed to load icon " + file.path().generic_string() + ". Is it an image?");
                }
                icons.push_back(GLFWimage{.width=width, .height=height, .pixels=pixels});  // Generate image
            }
        }
        glfwSetWindowIcon(window, static_cast<int>(icons.size()), icons.data());  // Set icons
        for (GLFWimage icon : icons) {
            stbi_image_free(icon.pixels);  // Free all pixel data
        }
    }

public:
    explicit IERenderEngine() {
        // Create a Vulkan instance
        renderEngineLink.instance = createVulkanInstance();
        engineDeletionQueue.emplace_front([&] { vkb::destroy_instance(renderEngineLink.instance); });

        // Initialize GLFW then create and setup window
        /**@todo Clean up this section of the code as it is still quite messy. Optimally this would be done with a GUI abstraction.*/
        glfwInit();
        window = createWindow();
        setWindowIcons("res/icons");
        glfwSetWindowSizeLimits(window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
        int xPos{renderEngineLink.settings->windowPosition[0]}, yPos{renderEngineLink.settings->windowPosition[1]};
        glfwGetWindowPos(window, &xPos, &yPos);
        renderEngineLink.settings->windowPosition = {xPos, yPos};
        glfwSetWindowAttrib(window, GLFW_AUTO_ICONIFY, 0);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetWindowUserPointer(window, this);
        if (glfwCreateWindowSurface(renderEngineLink.instance.instance, window, nullptr, &renderEngineLink.surface) != VK_SUCCESS) { throw std::runtime_error("failed to create window surface!"); }
        engineDeletionQueue.emplace_front([&] { vkDestroySurfaceKHR(renderEngineLink.instance.instance, renderEngineLink.surface, nullptr); });
        /**@todo Implement a device selection scheme for systems with multiple GPUs.*/
        vkb::PhysicalDeviceSelector selector{renderEngineLink.instance};
        vkb::detail::Result<vkb::PhysicalDevice> temporaryPhysicalDeviceBuilder = selector.set_surface(renderEngineLink.surface).prefer_gpu_device_type(vkb::PreferredDeviceType::discrete).select();
        if (!temporaryPhysicalDeviceBuilder) { throw std::runtime_error("failed to select Vulkan Physical Device! Does your device support Vulkan? Error: " + temporaryPhysicalDeviceBuilder.error().message() + "\n"); }
        vkb::DeviceBuilder temporaryLogicalDeviceBuilder{temporaryPhysicalDeviceBuilder.value()};
        vkb::detail::Result<vkb::Device> temporaryLogicalDevice = temporaryLogicalDeviceBuilder.build();
        if (!temporaryLogicalDevice) { throw std::runtime_error("failed to create Vulkan device! Error: " + temporaryLogicalDevice.error().message() + "\n"); }
        renderEngineLink.device = temporaryLogicalDevice.value();
        bool renderDocCapturing = std::getenv("ENABLE_VULKAN_RENDERDOC_CAPTURE") != nullptr;
        renderEngineLink.build(renderDocCapturing);
        vkb::destroy_device(renderEngineLink.device);
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
//            rayTracingExtensions
        };
        //DEVICE FEATURE SELECTION
        //------------------------
        std::vector<VkBool32 *> anisotropicFilteringFeatures{
            &renderEngineLink.enabledPhysicalDeviceInfo.anisotropicFiltering,
            &renderEngineLink.enabledPhysicalDeviceInfo.physicalDeviceFeatures.samplerAnisotropy
        };
        std::vector<VkBool32 *> msaaSmoothingFeatures{
            &renderEngineLink.enabledPhysicalDeviceInfo.msaaSmoothing,
            &renderEngineLink.enabledPhysicalDeviceInfo.physicalDeviceFeatures.sampleRateShading
        };
        std::vector<std::vector<VkBool32 *>> deviceFeatureGroups{
            anisotropicFilteringFeatures,
            msaaSmoothingFeatures
        };
        //EXTENSION FEATURE SELECTION
        //---------------------------
        std::vector<VkBool32 *> rayTracingFeatures{
                &renderEngineLink.enabledPhysicalDeviceInfo.rayTracing,
                &renderEngineLink.enabledPhysicalDeviceInfo.physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress,
                &renderEngineLink.enabledPhysicalDeviceInfo.physicalDeviceAccelerationStructureFeatures.accelerationStructure,
                &renderEngineLink.enabledPhysicalDeviceInfo.physicalDeviceRayQueryFeatures.rayQuery,
                &renderEngineLink.enabledPhysicalDeviceInfo.physicalDeviceRayTracingPipelineFeatures.rayTracingPipeline
        };
        std::vector<std::vector<VkBool32 *>> extensionFeatureGroups{
                rayTracingFeatures
        };
        //===========================
        for (const std::vector<VkBool32 *> &deviceFeatureGroup : deviceFeatureGroups) {
            if (renderEngineLink.testFeature(std::vector<VkBool32 *>(deviceFeatureGroup.begin() + 1, deviceFeatureGroup.end()))[0]) {
                *deviceFeatureGroup[0] = VK_TRUE;
                *(deviceFeatureGroup[0] - (VkBool32 *)&renderEngineLink.enabledPhysicalDeviceInfo + (VkBool32 *)&renderEngineLink.supportedPhysicalDeviceInfo) = VK_TRUE;
                renderEngineLink.enableFeature(deviceFeatureGroup);
            }
        }
        for (const std::vector<VkBool32 *> &extensionFeatureGroup : extensionFeatureGroups) {
            if (renderEngineLink.testFeature(std::vector<VkBool32 *>(extensionFeatureGroup.begin() + 1, extensionFeatureGroup.end()))[0]) {
                *extensionFeatureGroup[0] = VK_TRUE;
                *(extensionFeatureGroup[0] - (VkBool32 *)&renderEngineLink.enabledPhysicalDeviceInfo + (VkBool32 *)&renderEngineLink.supportedPhysicalDeviceInfo) = VK_TRUE;
                renderEngineLink.enableFeature(extensionFeatureGroup);
            }
        }
        if (!extensionGroups.empty()) { selector.add_desired_extensions(*extensionGroups.data()); }
        vkb::detail::Result<vkb::PhysicalDevice> finalPhysicalDeviceBuilder = selector.set_surface(renderEngineLink.surface).set_required_features(renderEngineLink.enabledPhysicalDeviceInfo.physicalDeviceFeatures).prefer_gpu_device_type(vkb::PreferredDeviceType::discrete).select();
        vkb::DeviceBuilder finalLogicalDeviceBuilder{finalPhysicalDeviceBuilder.value()};
        finalLogicalDeviceBuilder.add_pNext(renderDocCapturing ? renderEngineLink.enabledPhysicalDeviceInfo.pNextHighestRenderDocCompatibleFeature : renderEngineLink.enabledPhysicalDeviceInfo.pNextHighestFeature);
        vkb::detail::Result<vkb::Device> finalLogicalDevice = finalLogicalDeviceBuilder.build();
        if (!finalLogicalDevice) { throw std::runtime_error("failed to create Vulkan device. Error: " + finalLogicalDevice.error().message() + "\n"); }
        renderEngineLink.device = finalLogicalDevice.value();
        renderEngineLink.build(renderDocCapturing);
        engineDeletionQueue.emplace_front([&] { vkb::destroy_device(renderEngineLink.device); });
        renderEngineLink.graphicsQueue = renderEngineLink.device.get_queue(vkb::QueueType::graphics).value();
        renderEngineLink.presentQueue = renderEngineLink.device.get_queue(vkb::QueueType::present).value();
        renderEngineLink.transferQueue = renderEngineLink.device.get_queue(vkb::QueueType::transfer).value();
        renderEngineLink.computeQueue = renderEngineLink.device.get_queue(vkb::QueueType::compute).value();
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = renderEngineLink.device.physical_device.physical_device;
        allocatorInfo.device = renderEngineLink.device.device;
        allocatorInfo.instance = renderEngineLink.instance.instance;
        vmaCreateAllocator(&allocatorInfo, &renderEngineLink.allocator);
        engineDeletionQueue.emplace_front([&] { vmaDestroyAllocator(renderEngineLink.allocator); });
        VkCommandPoolCreateInfo commandPoolCreateInfo {
            .sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags=VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex=renderEngineLink.device.get_queue_index(vkb::QueueType::graphics).value()
        };
        vkCreateCommandPool(renderEngineLink.device.device, &commandPoolCreateInfo, nullptr, &renderEngineLink.commandPool);
        commandBuffer.create(&renderEngineLink, vkb::QueueType::graphics);
        engineDeletionQueue.emplace_front([&] { commandBuffer.destroy(); });
        camera.create(&renderEngineLink);
        createSwapchain(true);
        engineDeletionQueue.emplace_front([&] { topLevelAccelerationStructure.destroy(); });
    }

    void createSwapchain(bool fullRecreate = false) {
        vkDeviceWaitIdle(renderEngineLink.device.device);
        for (std::function<void()> &function : recreationDeletionQueue) { function(); }
        recreationDeletionQueue.clear();
        vkb::SwapchainBuilder swapchainBuilder{ renderEngineLink.device };
        vkb::detail::Result<vkb::Swapchain> swap_ret = swapchainBuilder.set_desired_present_mode(renderEngineLink.settings->vSync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR).set_desired_extent(renderEngineLink.settings->resolution[0], renderEngineLink.settings->resolution[1]).set_desired_format({VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR}).set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT).build();
        if (!swap_ret) { throw std::runtime_error(swap_ret.error().message()); }
        renderEngineLink.swapchain = swap_ret.value();
        recreationDeletionQueue.emplace_front([&] { vkb::destroy_swapchain(renderEngineLink.swapchain); });
        renderEngineLink.swapchainImageViews = renderEngineLink.swapchain.get_image_views().value();
        recreationDeletionQueue.emplace_front([&] { renderEngineLink.swapchain.destroy_image_views(renderEngineLink.swapchainImageViews); });
        imagesInFlight.clear();
        imagesInFlight.resize(renderEngineLink.swapchain.image_count, VK_NULL_HANDLE);
        if (fullRecreate) {
            for (std::function<void()> &function : fullRecreationDeletionQueue) { function(); }
            fullRecreationDeletionQueue.clear();
            commandBuffer.createCommandBuffers((int)renderEngineLink.swapchain.image_count);
            fullRecreationDeletionQueue.emplace_front([&] { commandBuffer.freeCommandBuffers(); });
            imageAvailableSemaphores.resize(renderEngineLink.swapchain.image_count);
            renderFinishedSemaphores.resize(renderEngineLink.swapchain.image_count);
            inFlightFences.resize(renderEngineLink.swapchain.image_count);
            VkSemaphoreCreateInfo semaphoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
            VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            for (unsigned int i = 0; i < renderEngineLink.swapchain.image_count; i++) {
                if (vkCreateSemaphore(renderEngineLink.device.device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create semaphores!"); }
                if (vkCreateSemaphore(renderEngineLink.device.device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create semaphores!"); }
                if (vkCreateFence(renderEngineLink.device.device, &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create fences!"); }
            }
            fullRecreationDeletionQueue.emplace_front([&] { for (VkSemaphore imageAvailableSemaphore : imageAvailableSemaphores) { vkDestroySemaphore(renderEngineLink.device.device, imageAvailableSemaphore, nullptr); } });
            fullRecreationDeletionQueue.emplace_front([&] { for (VkSemaphore renderFinishedSemaphore : renderFinishedSemaphores) { vkDestroySemaphore(renderEngineLink.device.device, renderFinishedSemaphore, nullptr); } });
            fullRecreationDeletionQueue.emplace_front([&] { for (VkFence inFlightFence : inFlightFences) { vkDestroyFence(renderEngineLink.device.device, inFlightFence, nullptr); } });
            renderPass.create(&renderEngineLink);
            fullRecreationDeletionQueue.emplace_front([&] { renderPass.destroy(); });
            for (IERenderable *renderable : renderables) { loadRenderable(renderable, false); }
            fullRecreationDeletionQueue.emplace_front([&] { for (IERenderable *renderable : renderables) { renderable->destroy(); } });
        }
        IEFramebuffer::CreateInfo framebufferCreateInfo{};
        framebuffers.resize(renderEngineLink.swapchain.image_count);
        for (uint32_t i = 0; i < renderEngineLink.swapchain.image_count; ++i) {
            framebufferCreateInfo.renderPass = renderPass;
            framebufferCreateInfo.swapchainImageView = (renderEngineLink.swapchainImageViews)[i];
            framebuffers[i].create(&renderEngineLink, &framebufferCreateInfo);
        }
        recreationDeletionQueue.emplace_front([&] { for (IEFramebuffer &framebuffer : framebuffers) { framebuffer.destroy(); } });
        camera.updateSettings();
    }

    void loadRenderable(IERenderable *renderable, bool append = true) {
        renderable->destroy();
        renderable->reloadRenderable(renderable->shaderNames);
        renderable->shaders.resize(renderable->shaderCreateInfos.size());
        for (int i = 0; i < renderable->shaderCreateInfos.size(); ++i) { renderable->shaders[i].create(&renderEngineLink, &renderable->shaderCreateInfos[i]); }
        renderable->deletionQueue.emplace_front([&] (IERenderable *thisRenderable) { for (IEShader &shader : thisRenderable->shaders) { shader.destroy(); } });
        IEBuffer::CreateInfo bufferCreateInfo{};
        bufferCreateInfo.size = sizeof(IEUniformBufferObject);
        bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferCreateInfo.allocationUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        renderable->modelBuffer.create(&renderEngineLink, &bufferCreateInfo);
        renderable->deletionQueue.emplace_front([&](IERenderable *thisRenderable){ thisRenderable->modelBuffer.destroy(); });
        for (IERenderable::IEMesh &mesh : renderable->meshes) {
            bufferCreateInfo.size = sizeof(mesh.vertices[0]) * mesh.vertices.size();
            bufferCreateInfo.usage = renderEngineLink.settings->rayTracing ? VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            mesh.vertexBuffer.create(&renderEngineLink, &bufferCreateInfo);
            mesh.vertexBuffer.uploadData(mesh.vertices.data(), sizeof(mesh.vertices[0]) * mesh.vertices.size());
            mesh.deletionQueue.emplace_front([&] (IERenderable::IEMesh *thisMesh) { thisMesh->vertexBuffer.destroy(); });
            bufferCreateInfo.size = sizeof(mesh.indices[0]) * mesh.indices.size();
            bufferCreateInfo.usage ^= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            mesh.indexBuffer.create(&renderEngineLink, &bufferCreateInfo);
            mesh.indexBuffer.uploadData(mesh.indices.data(), sizeof(mesh.indices[0]) * mesh.indices.size());
            mesh.deletionQueue.emplace_front([&] (IERenderable::IEMesh *thisMesh) { thisMesh->indexBuffer.destroy(); });
            if (renderEngineLink.settings->rayTracing) {
                bufferCreateInfo.size = sizeof(mesh.transformationMatrix);
                bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
                mesh.transformationBuffer.create(&renderEngineLink, &bufferCreateInfo);
                mesh.deletionQueue.emplace_front([&] (IERenderable::IEMesh *thisMesh) { thisMesh->transformationBuffer.destroy(); });
                mesh.deletionQueue.emplace_front([&] (IERenderable::IEMesh *thisMesh) { thisMesh->bottomLevelAccelerationStructure.destroy(); });
            }
            for (IETexture &texture : renderable->textures) {
                texture.destroy();
                IETexture::CreateInfo textureCreateInfo{texture.createdWith};
                textureCreateInfo.mipMapping = renderEngineLink.settings->mipMapping;
                texture.create(&renderEngineLink, &textureCreateInfo);
                texture.upload();
            }
            IEDescriptorSet::CreateInfo renderableDescriptorSetCreateInfo{};
            if (renderEngineLink.settings->rayTracing) {
                renderableDescriptorSetCreateInfo.poolSizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1}};
                renderableDescriptorSetCreateInfo.shaderStages = {static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT), VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
                renderableDescriptorSetCreateInfo.data = {&renderable->modelBuffer, &renderable->textures[mesh.diffuseTexture], std::nullopt};
            } else {
                renderableDescriptorSetCreateInfo.poolSizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};
                renderableDescriptorSetCreateInfo.shaderStages = {static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT), VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
                renderableDescriptorSetCreateInfo.data = {&renderable->modelBuffer, &renderable->textures[mesh.diffuseTexture], &renderable->textures[mesh.specularTexture]};
            }
            mesh.descriptorSet.create(&renderEngineLink, &renderableDescriptorSetCreateInfo);
            mesh.deletionQueue.emplace_front([&] (IERenderable::IEMesh *thisMesh) { thisMesh->descriptorSet.destroy(); });
            IEPipeline::CreateInfo renderablePipelineCreateInfo{};
            renderablePipelineCreateInfo.descriptorSet = &mesh.descriptorSet;
            renderablePipelineCreateInfo.renderPass = &renderPass;
            renderablePipelineCreateInfo.shaders = renderable->shaders;
            mesh.pipeline.create(&renderEngineLink, &renderablePipelineCreateInfo);
            mesh.deletionQueue.emplace_front([&] (IERenderable::IEMesh *thisMesh) { thisMesh->pipeline.destroy(); });
        }
        renderable->deletionQueue.emplace_front([&] (IERenderable *thisRenderable) { for (IETexture &texture : thisRenderable->textures) { texture.destroy(); } });
        renderable->deletionQueue.emplace_front([&] (IERenderable *thisRenderable) { for (IERenderable::IEMesh &mesh : thisRenderable->meshes) { mesh.destroy(); } });
        if (append) { renderables.push_back(renderable); }
    }

    bool update() {
        if (window == nullptr) { return false; }
        if (renderables.empty()) { return glfwWindowShouldClose(window) != 1; }
        vkWaitForFences(renderEngineLink.device.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        uint32_t imageIndex{0};
        VkResult result = renderEngineLink.vkAcquireNextImageKhr(renderEngineLink.device.device, renderEngineLink.swapchain.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            createSwapchain();
            return glfwWindowShouldClose(window) != 1;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { throw std::runtime_error("failed to acquire swapchain image!"); }
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) { vkWaitForFences(renderEngineLink.device.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX); }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];
        VkDeviceSize offsets[] = {0};
        commandBuffer.resetCommandBuffer((int)(imageIndex + (renderEngineLink.swapchain.image_count - 1)) % (int)renderEngineLink.swapchain.image_count);
        commandBuffer.recordCommandBuffer((int)imageIndex);
        VkViewport viewport{};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = (float)renderEngineLink.swapchain.extent.width;
        viewport.height = (float)renderEngineLink.swapchain.extent.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        vkCmdSetViewport(commandBuffer.commandBuffers[imageIndex], 0, 1, &viewport);
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = renderEngineLink.swapchain.extent;
        vkCmdSetScissor(commandBuffer.commandBuffers[imageIndex], 0, 1, &scissor);
        VkRenderPassBeginInfo renderPassBeginInfo = renderPass.beginRenderPass(framebuffers[imageIndex]);
        vkCmdBeginRenderPass(commandBuffer.commandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        camera.update();
        auto time = static_cast<float>(glfwGetTime());
        for (IERenderable *renderable : renderables) { if (renderable->render) { renderable->update(camera, time); } }
        if (renderEngineLink.settings->rayTracing) {
            topLevelAccelerationStructure.destroy();
            std::vector<VkDeviceAddress> bottomLevelAccelerationStructureDeviceAddresses{};
            bottomLevelAccelerationStructureDeviceAddresses.reserve(renderables.size());
            for (IERenderable *renderable : renderables) { if (renderable->render & renderEngineLink.settings->rayTracing) { for (const IERenderable::IEMesh &mesh : renderable->meshes) { bottomLevelAccelerationStructureDeviceAddresses.push_back(mesh.bottomLevelAccelerationStructure.deviceAddress); } } }
            IEAccelerationStructure::CreateInfo topLevelAccelerationStructureCreateInfo{};
            topLevelAccelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            topLevelAccelerationStructureCreateInfo.transformationMatrix = &identityTransformMatrix;
            topLevelAccelerationStructureCreateInfo.primitiveCount = 1;
            topLevelAccelerationStructureCreateInfo.bottomLevelAccelerationStructureDeviceAddresses = bottomLevelAccelerationStructureDeviceAddresses;
            topLevelAccelerationStructure.create(&renderEngineLink, &topLevelAccelerationStructureCreateInfo);
        }
        for (IERenderable *renderable : renderables) {
            if (renderable->render) {
                for (IERenderable::IEMesh &mesh : renderable->meshes) {
                    if (renderEngineLink.settings->rayTracing) { mesh.descriptorSet.update({&topLevelAccelerationStructure}, {2}); }
                    vkCmdBindVertexBuffers(commandBuffer.commandBuffers[imageIndex], 0, 1, &mesh.vertexBuffer.buffer, offsets);
                    vkCmdBindIndexBuffer(commandBuffer.commandBuffers[imageIndex], mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                    vkCmdBindPipeline(commandBuffer.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, mesh.pipeline.pipeline);
                    vkCmdBindDescriptorSets(commandBuffer.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, mesh.pipeline.pipelineLayout, 0, 1, &mesh.descriptorSet.descriptorSet, 0, nullptr);
                    vkCmdDrawIndexed(commandBuffer.commandBuffers[imageIndex], static_cast<uint32_t>(mesh.indices.size()), 1, 0, 0, 0);
                }
            }
        }
        vkCmdEndRenderPass(commandBuffer.commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBuffer.commandBuffers[imageIndex]) != VK_SUCCESS) { throw std::runtime_error("failed to record draw command IEBuffer!"); }
        VkSemaphore waitSemaphores[]{imageAvailableSemaphores[currentFrame]};
        VkSemaphore signalSemaphores[]{renderFinishedSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[]{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer.commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        vkResetFences(renderEngineLink.device.device, 1, &inFlightFences[currentFrame]);
        VkResult test = vkQueueSubmit(renderEngineLink.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);
        if ( test != VK_SUCCESS) { throw std::runtime_error("failed to submit draw command IEBuffer!"); }
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
            createSwapchain();
        } else if (result != VK_SUCCESS) { throw std::runtime_error("failed to present swapchain image!"); }
        currentFrame = (currentFrame + 1) % (int)renderEngineLink.swapchain.image_count;
        return glfwWindowShouldClose(window) != 1;
    }

    void reloadRenderables() {
        for (IERenderable *renderable : renderables) { loadRenderable(renderable, false); }
    }

    void handleFullscreenSettingsChange() {
        if (renderEngineLink.settings->fullscreen) {
            glfwGetWindowPos(window, &renderEngineLink.settings->windowPosition[0], &renderEngineLink.settings->windowPosition[1]);
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
        } else { glfwSetWindowMonitor(window, nullptr, renderEngineLink.settings->windowPosition[0], renderEngineLink.settings->windowPosition[1], static_cast<int>(renderEngineLink.settings->defaultWindowResolution[0]), static_cast<int>(renderEngineLink.settings->defaultWindowResolution[1]), static_cast<int>(renderEngineLink.settings->refreshRate)); }
        glfwSetWindowTitle(window, renderEngineLink.settings->applicationName.c_str());
    }

    void destroy() {
        for (IERenderable *renderable : renderables) { renderable->destroy(); }
        for (std::function<void()> &function : recreationDeletionQueue) { function(); }
        recreationDeletionQueue.clear();
        for (std::function<void()> &function : fullRecreationDeletionQueue) { function(); }
        fullRecreationDeletionQueue.clear();
        for (std::function<void()> &function : engineDeletionQueue) { function(); }
        engineDeletionQueue.clear();
    }

    GLFWmonitor *monitor{};
    GLFWwindow *window{};
    IECamera camera{};
    IERenderPass renderPass{};
    IEGraphicsLink renderEngineLink{};
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
    IECommandBuffer commandBuffer{};
    std::deque<std::function<void()>> engineDeletionQueue{};
    std::deque<std::function<void()>> fullRecreationDeletionQueue{};
    std::deque<std::function<void()>> recreationDeletionQueue{};
    size_t currentFrame{};
    bool framebufferResized{false};
    float previousTime{};

    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
        auto vulkanRenderEngine = (IERenderEngine *)glfwGetWindowUserPointer(pWindow);
        vulkanRenderEngine->framebufferResized = true;
        vulkanRenderEngine->renderEngineLink.settings->resolution[0] = width;
        vulkanRenderEngine->renderEngineLink.settings->resolution[1] = height;
    }
};