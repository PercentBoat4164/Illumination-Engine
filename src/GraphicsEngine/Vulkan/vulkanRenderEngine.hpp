/**@todo: Add multithreading support throughout the engine
 * - LOW PRIORITY: GPU is busy 95-98% of the time.
 * This will be a higher priority if GPU hits 80-90%.
 * Focus is going to be put on making all engines work before perfecting any of them.
 */

/**@todo: Combine all of the engines into one.
 * - LOW PRIORITY: This is optional, and not high on the list until ray tracing performance becomes horrendous.
 * Perhaps create some type of hybrid engine that can both ray trace and rasterize.
 * This would require a rewrite of some parts of the ray tracer, but might be worth it as it would help significantly in real-time scenarios.
 */

/**@todo: Add proper invalid input checks to, and clean up, the abstractions.
 * - HIGH PRIORITY: This should happen before moving on to a new part of the game engine.
 * This has already been started in vulkanDescriptorSet.hpp.
 */

/*
 * Try changing
 */

#pragma once

#include "vulkanRenderPass.hpp"
#include "vulkanGraphicsEngineLink.hpp"
#include "vulkanBuffer.hpp"
#include "vulkanCamera.hpp"
#include "vulkanRenderable.hpp"
#include "vulkanUniformBufferObject.hpp"
#include "vulkanSettings.hpp"
#include "vulkanCommandBuffer.hpp"
#include "vulkanTexture.hpp"

#include <VkBootstrap.h>

#ifndef GLEW_IMPLEMENTATION
#define GLEW_IMPLEMENTATION
#include "../../../deps/glew/include/GL/glew.h"
#endif

#include <GLFW/glfw3.h>

#include <deque>
#include <functional>

class VulkanRenderEngine {
private:
    vkb::Instance instance{};
    VmaAllocator allocator{};
    VkSurfaceKHR surface{};
    std::deque<std::function<void()>> oneTimeOptionalDeletionQueue{};
    std::vector<VkImageView> swapchainImageViews{};
    VkTransformMatrixKHR identityTransformMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

protected:
    size_t currentFrame{};
    bool framebufferResized{false};
    float previousTime{};

    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
        auto vulkanRenderEngine = (VulkanRenderEngine *)glfwGetWindowUserPointer(pWindow);
        vulkanRenderEngine->framebufferResized = true;
        vulkanRenderEngine->settings.resolution[0] = width;
        vulkanRenderEngine->settings.resolution[1] = height;
    }

    std::deque<std::function<void()>> engineDeletionQueue{};
    vkb::Device device{};
    std::vector<VkFence> inFlightFences{};
    vkb::Swapchain swapchain{};
    std::vector<VkFence> imagesInFlight{};
    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};
    VulkanGraphicsEngineLink renderEngineLink{};
    Buffer scratchBuffer{};
    std::deque<std::function<void()>> recreationDeletionQueue{};
    AccelerationStructure topLevelAccelerationStructure{};

public:
    virtual void loadRenderable(VulkanRenderable *renderable, bool append) {
        //destroy previously created renderable if any
        renderable->destroy();
        //upload mesh, vertex, and transformation data
        Buffer::CreateInfo vertexBufferCreateInfo{sizeof(renderable->vertices[0]) * renderable->vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        if (settings.rayTracing) { vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; }
        memcpy(renderable->vertexBuffer.create(&renderEngineLink, &vertexBufferCreateInfo), renderable->vertices.data(), sizeof(renderable->vertices[0]) * renderable->vertices.size());
        renderable->deletionQueue.emplace_front([&](VulkanRenderable thisRenderable){ thisRenderable.vertexBuffer.destroy(); });
        Buffer::CreateInfo indexBufferCreateInfo{sizeof(renderable->indices[0]) * renderable->indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        if (settings.rayTracing) { indexBufferCreateInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; }
        memcpy(renderable->indexBuffer.create(&renderEngineLink, &indexBufferCreateInfo), renderable->indices.data(), sizeof(renderable->indices[0]) * renderable->indices.size());
        renderable->deletionQueue.emplace_front([&](VulkanRenderable thisRenderable){ thisRenderable.indexBuffer.destroy(); });
        if (settings.rayTracing) {
            Buffer::CreateInfo transformationBufferCreateInfo{sizeof(renderable->transformationMatrix), VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
            memcpy(renderable->transformationBuffer.create(&renderEngineLink, &transformationBufferCreateInfo), &renderable->transformationMatrix, sizeof(VkTransformMatrixKHR));
            renderable->deletionQueue.emplace_front([&](VulkanRenderable thisRenderable) { thisRenderable.transformationBuffer.destroy(); });
        }
        //upload textures
        renderable->textureImages.resize(renderable->textures.size());
        for (unsigned int i = 0; i < renderable->textures.size(); ++i) {
            renderable->textureImages[i].destroy();
            Texture::CreateInfo textureImageCreateInfo{VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, renderable->textureImages[i].createdWith.width, renderable->textureImages[i].createdWith.height};
            textureImageCreateInfo.filename = renderable->textures[i];
            textureImageCreateInfo.mipMapping = settings.mipMapping;
            renderable->textureImages[i].create(&renderEngineLink, &textureImageCreateInfo);
        }
        //build uniform buffers
        Buffer::CreateInfo uniformBufferCreateInfo {sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        memcpy(renderable->uniformBuffer.create(&renderEngineLink, &uniformBufferCreateInfo), &renderable->uniformBufferObject, sizeof(UniformBufferObject));
        renderable->deletionQueue.emplace_front([&](VulkanRenderable thisRenderable){ thisRenderable.uniformBuffer.destroy(); });
        //build shaders
        renderable->shaders.resize(renderable->shaderCreateInfos.size());
        for (int i = 0; i < renderable->shaderCreateInfos.size(); ++i) { renderable->shaders[i].create(&renderEngineLink, &renderable->shaderCreateInfos[i]); }
        renderable->deletionQueue.emplace_front([&](const VulkanRenderable& thisRenderable) { for (const Shader& shader : thisRenderable.shaders) { shader.destroy(); } });
        /* This setup is temporary, but for the purposes of testing only the most recently uploaded model will be included in the raytracing. */
        if (settings.rayTracing) {
            AccelerationStructure::CreateInfo renderableBottomLevelAccelerationStructureCreateInfo{};
            renderableBottomLevelAccelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
            renderableBottomLevelAccelerationStructureCreateInfo.transformationMatrix = &identityTransformMatrix;
            renderableBottomLevelAccelerationStructureCreateInfo.primitiveCount = renderable->triangleCount;
            renderableBottomLevelAccelerationStructureCreateInfo.vertexBufferAddress = renderable->vertexBuffer.deviceAddress;
            renderableBottomLevelAccelerationStructureCreateInfo.indexBufferAddress = renderable->indexBuffer.deviceAddress;
            renderableBottomLevelAccelerationStructureCreateInfo.transformationBufferAddress = renderable->transformationBuffer.deviceAddress;
            renderable->bottomLevelAccelerationStructure.create(&renderEngineLink, &renderableBottomLevelAccelerationStructureCreateInfo);
            renderable->deletionQueue.emplace_front([&] (VulkanRenderable thisRenderable) { thisRenderable.bottomLevelAccelerationStructure.destroy(); });
            topLevelAccelerationStructure.destroy();
            AccelerationStructure::CreateInfo topLevelAccelerationStructureCreateInfo{};
            topLevelAccelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            topLevelAccelerationStructureCreateInfo.transformationMatrix = &identityTransformMatrix;
            topLevelAccelerationStructureCreateInfo.primitiveCount = 1;
            topLevelAccelerationStructureCreateInfo.bottomLevelAccelerationStructureDeviceAddress = renderable->bottomLevelAccelerationStructure.deviceAddress;
            topLevelAccelerationStructure.create(&renderEngineLink, &topLevelAccelerationStructureCreateInfo);
        }
        //build graphics pipeline and descriptor set for this renderable
        DescriptorSetManager::CreateInfo renderableDescriptorSetManagerCreateInfo{};
        if (settings.rayTracing) {
            renderableDescriptorSetManagerCreateInfo.poolSizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1}};
            renderableDescriptorSetManagerCreateInfo.shaderStages = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
            renderableDescriptorSetManagerCreateInfo.data = {&renderable->uniformBuffer, &renderable->textureImages[0], &topLevelAccelerationStructure};
        } else {
            renderableDescriptorSetManagerCreateInfo.poolSizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};
            renderableDescriptorSetManagerCreateInfo.shaderStages = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
            renderableDescriptorSetManagerCreateInfo.data = {&renderable->uniformBuffer, &renderable->textureImages[0]};
        }
        renderable->descriptorSetManager.create(&renderEngineLink, &renderableDescriptorSetManagerCreateInfo);
        renderable->deletionQueue.emplace_front([&](VulkanRenderable thisRenderable) { thisRenderable.descriptorSetManager.destroy(); });
        RasterizationPipelineManager::CreateInfo renderablePipelineManagerCreateInfo{};
        renderablePipelineManagerCreateInfo.descriptorSetManager = &renderable->descriptorSetManager;
        renderablePipelineManagerCreateInfo.renderPassManager = &renderPassManager;
        renderablePipelineManagerCreateInfo.shaders = renderable->shaders;
        renderable->pipelineManager.create(&renderEngineLink, &renderablePipelineManagerCreateInfo);
        renderable->deletionQueue.emplace_front([&](VulkanRenderable thisRenderable) { thisRenderable.pipelineManager.destroy(); });
        if (append) { renderables.push_back(renderable); }
    }

    void reloadRenderables() {
        for (VulkanRenderable *renderable : renderables) {
            renderable->destroy();
            loadRenderable(renderable, false);
        }
    }

    void handleFullscreenSettingsChange() {
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
        } else { glfwSetWindowMonitor(window, nullptr, settings.windowPosition[0], settings.windowPosition[1], static_cast<int>(settings.defaultWindowResolution[0]), static_cast<int>(settings.defaultWindowResolution[1]), static_cast<int>(settings.refreshRate)); }
        glfwSetWindowTitle(window, settings.applicationName.c_str());
    }

    void destroy() {
        camera.destroy();
        for (VulkanRenderable *renderable : renderables) { renderable->destroy(); }
        for (std::function<void()>& function : recreationDeletionQueue) { function(); }
        recreationDeletionQueue.clear();
        for (std::function<void()>& function : oneTimeOptionalDeletionQueue) { function(); }
        oneTimeOptionalDeletionQueue.clear();
        for (std::function<void()>& function : engineDeletionQueue) { function(); }
        engineDeletionQueue.clear();
    }

    virtual bool update() {
        //GPU synchronization
        if (window == nullptr) { return false; }
        if (renderables.empty()) { return glfwWindowShouldClose(window) != 1; }
        vkWaitForFences(device.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        uint32_t imageIndex{0};
        VkResult result = renderEngineLink.vkAcquireNextImageKhr(device.device, swapchain.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            createSwapchain();
            return glfwWindowShouldClose(window) != 1;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { throw std::runtime_error("failed to acquire swapchain image!"); }
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) { vkWaitForFences(device.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX); }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];
        //update state of frame
        VkDeviceSize offsets[] = {0};
        //record command buffers for color pass
        commandBufferManager.resetCommandBuffer((int)(imageIndex + (swapchain.image_count - 1)) % (int)swapchain.image_count);
        commandBufferManager.recordCommandBuffer((int)imageIndex);
        VkViewport viewport{};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = (float)swapchain.extent.width;
        viewport.height = (float)swapchain.extent.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        vkCmdSetViewport(commandBufferManager.commandBuffers[imageIndex], 0, 1, &viewport);
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapchain.extent;
        vkCmdSetScissor(commandBufferManager.commandBuffers[imageIndex], 0, 1, &scissor);
        VkRenderPassBeginInfo renderPassBeginInfo = renderPassManager.beginRenderPass(imageIndex);
        vkCmdBeginRenderPass(commandBufferManager.commandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, renderables[0]->pipelineManager.pipeline);
        camera.update();
        for (VulkanRenderable *renderable : renderables) {
            if (renderable->render) {
                //update renderable
                renderable->update(camera);
                //record command buffer for this renderable
                vkCmdBindVertexBuffers(commandBufferManager.commandBuffers[imageIndex], 0, 1, &renderable->vertexBuffer.buffer, offsets);
                vkCmdBindIndexBuffer(commandBufferManager.commandBuffers[imageIndex], renderable->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipelineManager.pipelineLayout, 0, 1, &renderable->descriptorSetManager.descriptorSet, 0, nullptr);
                vkCmdDrawIndexed(commandBufferManager.commandBuffers[imageIndex], static_cast<uint32_t>(renderable->indices.size()), 1, 0, 0, 0);
            }
        }
        vkCmdEndRenderPass(commandBufferManager.commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBufferManager.commandBuffers[imageIndex]) != VK_SUCCESS) { throw std::runtime_error("failed to record command buffer!"); }
        //Submit
        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBufferManager.commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        vkResetFences(device.device, 1, &inFlightFences[currentFrame]);
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) { throw std::runtime_error("failed to submit draw command buffer!"); }
        //Present
        VkSwapchainKHR swapchains[] = {swapchain.swapchain};
        VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;
        result = vkQueuePresentKHR(presentQueue, &presentInfo);
        //update frameTime and frameNumber
        auto currentTime = (float)glfwGetTime();
        frameTime = currentTime - previousTime;
        previousTime = currentTime;
        frameNumber++;
        //Check if window has been resized
        vkQueueWaitIdle(presentQueue);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            createSwapchain();
        } else if (result != VK_SUCCESS) { throw std::runtime_error("failed to present swapchain image!"); }
        currentFrame = (currentFrame + 1) % (int)swapchain.image_count;
        return glfwWindowShouldClose(window) != 1;
    }

    explicit VulkanRenderEngine(GLFWwindow *attachWindow = nullptr) {
        renderEngineLink.device = &device;
        renderEngineLink.swapchain = &swapchain;
        renderEngineLink.settings = &settings;
        renderEngineLink.commandPool = &commandBufferManager.commandPool;
        renderEngineLink.allocator = &allocator;
        renderEngineLink.graphicsQueue = &graphicsQueue;
        vkb::detail::Result<vkb::SystemInfo> systemInfo = vkb::SystemInfo::get_system_info();
        //build instance
        vkb::InstanceBuilder builder;
        builder.set_app_name(settings.applicationName.c_str()).set_app_version(settings.applicationVersion[0], settings.applicationVersion[1], settings.applicationVersion[2]).require_api_version(settings.requiredVulkanVersion[0], settings.requiredVulkanVersion[1], settings.requiredVulkanVersion[2]);
        #ifdef _DEBUG
        /* Validation layers hamper performance. Therefore to eek out extra speed from the GPU they will be turned off if the program is run in 'Release' mode. */
        if (systemInfo->validation_layers_available) { builder.request_validation_layers(); }
        #endif
        if (systemInfo->debug_utils_available) { builder.use_default_debug_messenger(); }
        vkb::detail::Result<vkb::Instance> inst_ret = builder.build();
        if (!inst_ret) { throw std::runtime_error("Failed to create Vulkan instance. Error: " + inst_ret.error().message() + "\n"); }
        instance = inst_ret.value();
        //build window
        engineDeletionQueue.emplace_front([&] { vkb::destroy_instance(instance); });
        if (attachWindow == nullptr) { glfwInit(); }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(static_cast<int>(settings.resolution[0]), static_cast<int>(settings.resolution[1]), settings.applicationName.c_str(), settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr, attachWindow);
        // load icon
        int width, height, channels, sizes[] = {256, 128, 64, 32, 16};
        GLFWimage icons[sizeof(sizes)/sizeof(int)];
        for (unsigned long i = 0; i < sizeof(sizes)/sizeof(int); ++i) {
            std::string filename = "res/Logos/IlluminationEngineLogo" + std::to_string(sizes[i]) + ".png";
            stbi_uc *pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { throw std::runtime_error("failed to load icon image from file: " + filename); }
            icons[i].pixels = pixels;
            icons[i].height = height;
            icons[i].width = width;
        }
        glfwSetWindowIcon(window, sizeof(icons)/sizeof(GLFWimage), icons);
        for (GLFWimage icon : icons) { stbi_image_free(icon.pixels); }
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
        std::vector<const char *> extensionNames{
            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
            VK_KHR_SPIRV_1_4_EXTENSION_NAME,
        };
        VkPhysicalDeviceFeatures deviceFeatures{}; //require device features here
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;
        vkb::detail::Result<vkb::PhysicalDevice> physicalDeviceBuildResults = selector.set_surface(surface).add_required_extensions(extensionNames).set_required_features(deviceFeatures).prefer_gpu_device_type(vkb::PreferredDeviceType::discrete).select();
        if (!physicalDeviceBuildResults) { throw std::runtime_error("Failed to select Vulkan Physical Device. Error: " + physicalDeviceBuildResults.error().message() + "\n"); }
        //create logical device
        vkb::DeviceBuilder logicalDeviceBuilder{physicalDeviceBuildResults.value()};
        #ifdef ILLUMINATION_ENGINE_VULKAN_RAY_TRACING
        //get features and properties physical device on temporary device with nothing enabled
        vkb::detail::Result<vkb::Device> temporaryDevice = logicalDeviceBuilder.build();
        if (!temporaryDevice) { throw std::runtime_error("Failed to create Vulkan device. Error: " + temporaryDevice.error().message() + "\n"); }
        device = temporaryDevice.value();
        renderEngineLink.build();
        vkb::destroy_device(device);
        //Enable required features
        std::vector<VkBool32 *> requestedFeatures{
            &renderEngineLink.enabledPhysicalDeviceInfo.physicalDeviceBufferDeviceAddressFeatures.bufferDeviceAddress,
            &renderEngineLink.enabledPhysicalDeviceInfo.physicalDeviceAccelerationStructureFeatures.accelerationStructure,
            &renderEngineLink.enabledPhysicalDeviceInfo.physicalDeviceRayQueryFeatures.rayQuery
        };
        if (!renderEngineLink.enableFeature(requestedFeatures)[0]) { throw std::runtime_error("One of the requested features is not supported!"); }
        vkb::detail::Result<vkb::Device> finalDevice = logicalDeviceBuilder.add_pNext(renderEngineLink.enabledPhysicalDeviceInfo.pNextHighestFeature).build();
        if (!finalDevice) { throw std::runtime_error("Failed to create Vulkan device. Error: " + finalDevice.error().message() + "\n"); }
        device = finalDevice.value();
        engineDeletionQueue.emplace_front([&] { vkb::destroy_device(device); });
        #else
        vkb::detail::Result<vkb::Device> dev_ret = device_builder.build();
        if (!dev_ret) { throw std::runtime_error("Failed to create Vulkan device. Error: " + dev_ret.error().message() + "\n"); }
        device = dev_ret.value();
        renderEngineLink.build();
        engineDeletionQueue.emplace_front([&] { vkb::destroy_device(device); });
        #endif
        //get queues
        graphicsQueue = device.get_queue(vkb::QueueType::graphics).value();
        presentQueue = device.get_queue(vkb::QueueType::present).value();
        //create vma allocator
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = device.physical_device.physical_device;
        allocatorInfo.device = device.device;
        allocatorInfo.instance = instance.instance;
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
        vmaCreateAllocator(&allocatorInfo, &allocator);
        engineDeletionQueue.emplace_front([&] { vmaDestroyAllocator(allocator); });
        //Create commandPool
        commandBufferManager.create(device, vkb::QueueType::graphics);
        engineDeletionQueue.emplace_front([&] { commandBufferManager.destroy(); });
        //delete scratch buffer
        engineDeletionQueue.emplace_front([&] { scratchBuffer.destroy(); });
        camera.create(&renderEngineLink);
        createSwapchain(true);
        engineDeletionQueue.emplace_front([&] { topLevelAccelerationStructure.destroy(); });
        initialized = true;
    }

    void createSwapchain(bool fullRecreate = false) {
        //Make sure no other GPU operations are ongoing
        vkDeviceWaitIdle(device.device);
        //Clear recreationDeletionQueue
        for (std::function<void()>& function : recreationDeletionQueue) { function(); }
        recreationDeletionQueue.clear();
        //Create swapchain
        vkb::SwapchainBuilder swapchainBuilder{ device };
        vkb::detail::Result<vkb::Swapchain> swap_ret = swapchainBuilder.set_desired_present_mode(settings.vSync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR).set_desired_extent(settings.resolution[0], settings.resolution[1]).set_desired_format({VK_FORMAT_B8G8R8A8_SRGB, VK_COLORSPACE_SRGB_NONLINEAR_KHR}).set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT).build();
        if (!swap_ret) { throw std::runtime_error(swap_ret.error().message()); }
        swapchain = swap_ret.value();
        renderEngineLink.swapchainImages = swapchain.get_images().value();
        recreationDeletionQueue.emplace_front([&]{ vkb::destroy_swapchain(swapchain); });
        swapchainImageViews = swapchain.get_image_views().value();
        recreationDeletionQueue.emplace_front([&]{ swapchain.destroy_image_views(swapchainImageViews); });
        renderEngineLink.swapchainImageViews = &swapchainImageViews;
        //clear images marked as in flight
        imagesInFlight.clear();
        imagesInFlight.resize(swapchain.image_count, VK_NULL_HANDLE);
        //do the other stuff only if needed
        if (fullRecreate) {
            for (std::function<void()> &function : oneTimeOptionalDeletionQueue) { function(); }
            oneTimeOptionalDeletionQueue.clear();
            //Create commandBuffers
            commandBufferManager.createCommandBuffers((int) swapchain.image_count);
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
            oneTimeOptionalDeletionQueue.emplace_front([&] { for (VkSemaphore imageAvailableSemaphore : imageAvailableSemaphores) { vkDestroySemaphore(device.device, imageAvailableSemaphore, nullptr); }});
            oneTimeOptionalDeletionQueue.emplace_front([&] { for (VkSemaphore renderFinishedSemaphore : renderFinishedSemaphores) { vkDestroySemaphore(device.device, renderFinishedSemaphore, nullptr); }});
            oneTimeOptionalDeletionQueue.emplace_front([&] { for (VkFence inFlightFence : inFlightFences) { vkDestroyFence(device.device, inFlightFence, nullptr); }});
            //Create render pass
            renderPassManager.setup(&renderEngineLink);
            oneTimeOptionalDeletionQueue.emplace_front([&] { renderPassManager.destroy(); });
            //re-upload renderables
            for (VulkanRenderable *renderable : renderables) { loadRenderable(renderable, false); }
        }
        //Recreate framebuffers without recreating entire RenderPass.
        renderPassManager.createFramebuffers();
        camera.updateSettings();
    }

    bool initialized{false};
    GLFWmonitor *monitor{};
    VulkanSettings settings{};
    VulkanCamera camera{};
    GLFWwindow *window{};
    std::vector<VulkanRenderable *> renderables{};
    CommandBufferManager commandBufferManager{};
    int frameNumber{};
    float frameTime{};
    RenderPassManager renderPassManager{};
};