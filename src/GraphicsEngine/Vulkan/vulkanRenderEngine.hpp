/**@todo: Add multithreading support throughout the engine
 * - LOW PRIORITY: GPU is busy 95-98% of the time.
 * This will be a higher priority if GPU hits 80-90%.
 * Focus is going to be put on making all engines work before perfecting any of them.
 */

/**@todo: Add proper invalid input checks to, and clean up, the abstractions.
 * - HIGH PRIORITY: This should happen before moving on to a new part of the game engine.
 * This has already been started in vulkanDescriptorSet.hpp.
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
public:
    explicit VulkanRenderEngine() {
        renderEngineLink.device = &device;
        renderEngineLink.swapchain = &swapchain;
        renderEngineLink.settings = &settings;
        renderEngineLink.commandPool = &commandBuffer.commandPool;
        renderEngineLink.allocator = &allocator;
        renderEngineLink.graphicsQueue = &graphicsQueue;
        renderEngineLink.presentQueue = &presentQueue;
        renderEngineLink.transferQueue = &transferQueue;
        renderEngineLink.computeQueue = &computeQueue;
        vkb::detail::Result<vkb::SystemInfo> systemInfo = vkb::SystemInfo::get_system_info();
        //build instance
        vkb::InstanceBuilder builder;
        builder.set_app_name(settings.applicationName.c_str()).set_app_version(settings.applicationVersion[0], settings.applicationVersion[1], settings.applicationVersion[2]).require_api_version(settings.requiredVulkanVersion[0], settings.requiredVulkanVersion[1], settings.requiredVulkanVersion[2]);
#ifdef _DEBUG
        /* Validation layers hamper performance. Therefore to eek out extra speed from the GPU they will be turned off if the program is run in 'Release' mode. */
        if (systemInfo->validation_layers_available) { builder.request_validation_layers(); }
        if (systemInfo->debug_utils_available) { builder.use_default_debug_messenger(); }
#endif
        vkb::detail::Result<vkb::Instance> instanceBuilder = builder.build();
        if (!instanceBuilder) { throw std::runtime_error("Failed to create Vulkan instance. Error: " + instanceBuilder.error().message() + "\n"); }
        instance = instanceBuilder.value();
        //build window
        engineDeletionQueue.emplace_front([&] { vkb::destroy_instance(instance); });
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(static_cast<int>(settings.resolution[0]), static_cast<int>(settings.resolution[1]), settings.applicationName.c_str(), settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
        // load icons
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
        /**@todo: Implement a device selection scheme for systems with multiple dedicated GPUs.*/
        // build temporary device
        vkb::PhysicalDeviceSelector selector{instance};
        vkb::detail::Result<vkb::PhysicalDevice> temporaryPhysicalDeviceBuilder = selector.set_surface(surface).prefer_gpu_device_type(vkb::PreferredDeviceType::discrete).select();
        if (!temporaryPhysicalDeviceBuilder) { throw std::runtime_error("failed to select Vulkan Physical Device! Does your device support Vulkan? Error: " + temporaryPhysicalDeviceBuilder.error().message() + "\n"); }
        vkb::DeviceBuilder temporaryLogicalDeviceBuilder{temporaryPhysicalDeviceBuilder.value()};
        vkb::detail::Result<vkb::Device> temporaryLogicalDevice = temporaryLogicalDeviceBuilder.build();
        if (!temporaryLogicalDevice) { throw std::runtime_error("failed to create Vulkan device! Error: " + temporaryLogicalDevice.error().message() + "\n"); }
        device = temporaryLogicalDevice.value();
        renderEngineLink.build();
        vkb::destroy_device(device);
        //EXTENSION SELECTION
        //-------------------
        std::vector<const char *> rayTracingExtensions{
                VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                VK_KHR_SPIRV_1_4_EXTENSION_NAME
        };
        std::vector<std::vector<const char *>> extensionGroups{
            rayTracingExtensions
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
                &renderEngineLink.enabledPhysicalDeviceInfo.physicalDeviceRayQueryFeatures.rayQuery
        };
        std::vector<std::vector<VkBool32 *>> extensionFeatureGroups{
                rayTracingFeatures
        };
        //===========================
        for (const std::vector<VkBool32 *>& deviceFeatureGroup : deviceFeatureGroups) {
            if (renderEngineLink.testFeature(std::vector<VkBool32 *>(deviceFeatureGroup.begin() + 1, deviceFeatureGroup.end()))[0]) {
                *deviceFeatureGroup[0] = VK_TRUE;
                *(deviceFeatureGroup[0] - (VkBool32 *)&renderEngineLink.enabledPhysicalDeviceInfo + (VkBool32 *)&renderEngineLink.supportedPhysicalDeviceInfo) = VK_TRUE;
                renderEngineLink.enableFeature(deviceFeatureGroup);
            }
        }
        //build final device
        vkb::detail::Result<vkb::PhysicalDevice> finalPhysicalDeviceBuilder = selector.set_surface(surface).add_desired_extensions(*extensionGroups.data()).set_required_features(renderEngineLink.enabledPhysicalDeviceInfo.physicalDeviceFeatures).prefer_gpu_device_type(vkb::PreferredDeviceType::discrete).select();
        vkb::DeviceBuilder finalLogicalDeviceBuilder{finalPhysicalDeviceBuilder.value()};
        vkb::detail::Result<vkb::Device> finalLogicalDevice = finalLogicalDeviceBuilder.add_pNext(renderEngineLink.enabledPhysicalDeviceInfo.pNextHighestFeature).build();
        if (!finalLogicalDevice) { throw std::runtime_error("failed to create Vulkan device. Error: " + finalLogicalDevice.error().message() + "\n"); }
        device = finalLogicalDevice.value();
        renderEngineLink.build();
        //Enable required features
        for (const std::vector<VkBool32 *>& extensionFeatureGroup : extensionFeatureGroups) {
            if (renderEngineLink.testFeature(std::vector<VkBool32 *>(extensionFeatureGroup.begin() + 1, extensionFeatureGroup.end()))[0]) {
                *extensionFeatureGroup[0] = VK_TRUE;
                *(extensionFeatureGroup[0] - (VkBool32 *)&renderEngineLink.enabledPhysicalDeviceInfo + (VkBool32 *)&renderEngineLink.supportedPhysicalDeviceInfo) = VK_TRUE;
                renderEngineLink.enableFeature(extensionFeatureGroup);
            }
        }
        engineDeletionQueue.emplace_front([&] { vkb::destroy_device(device); });
        //get queues
        graphicsQueue = device.get_queue(vkb::QueueType::graphics).value();
        presentQueue = device.get_queue(vkb::QueueType::present).value();
        transferQueue = device.get_queue(vkb::QueueType::transfer).value();
        computeQueue = device.get_queue(vkb::QueueType::compute).value();
        //create vma allocator
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = device.physical_device.physical_device;
        allocatorInfo.device = device.device;
        allocatorInfo.instance = instance.instance;
        allocatorInfo.flags = renderEngineLink.enabledPhysicalDeviceInfo.rayTracing ? VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT : 0;
        vmaCreateAllocator(&allocatorInfo, &allocator);
        engineDeletionQueue.emplace_front([&] { vmaDestroyAllocator(allocator); });
        //Create commandPool
        commandBuffer.create(device, vkb::QueueType::graphics);
        engineDeletionQueue.emplace_front([&] { commandBuffer.destroy(); });
        //delete scratch buffer
        engineDeletionQueue.emplace_front([&] { scratchBuffer.destroy(); });
        camera.create(&renderEngineLink);
        createSwapchain(true);
        engineDeletionQueue.emplace_front([&] { topLevelAccelerationStructure.destroy(); });
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
            commandBuffer.createCommandBuffers((int) swapchain.image_count);
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
            renderPass.create(&renderEngineLink);
            oneTimeOptionalDeletionQueue.emplace_front([&] { renderPass.destroy(); });
            //re-upload renderables
            for (VulkanRenderable *renderable : renderables) { loadRenderable(renderable, false); }
        }
        //Recreate framebuffers without recreating entire RenderPass.
        renderPass.createFramebuffers();
        camera.updateSettings();
    }

    void loadRenderable(VulkanRenderable *renderable, bool append = true) {
        //destroy previously created renderable if any
        renderable->destroy();
        //upload mesh, vertex, and transformation data
        VulkanBuffer::CreateInfo vertexBufferCreateInfo{sizeof(renderable->vertices[0]) * renderable->vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        if (settings.rayTracing) { vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; }
        memcpy(renderable->vertexBuffer.create(&renderEngineLink, &vertexBufferCreateInfo), renderable->vertices.data(), sizeof(renderable->vertices[0]) * renderable->vertices.size());
        renderable->deletionQueue.emplace_front([&](VulkanRenderable thisRenderable){ thisRenderable.vertexBuffer.destroy(); });
        VulkanBuffer::CreateInfo indexBufferCreateInfo{sizeof(renderable->indices[0]) * renderable->indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        if (settings.rayTracing) { indexBufferCreateInfo.usage |= VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; }
        memcpy(renderable->indexBuffer.create(&renderEngineLink, &indexBufferCreateInfo), renderable->indices.data(), sizeof(renderable->indices[0]) * renderable->indices.size());
        renderable->deletionQueue.emplace_front([&](VulkanRenderable thisRenderable){ thisRenderable.indexBuffer.destroy(); });
        if (settings.rayTracing) {
            VulkanBuffer::CreateInfo transformationBufferCreateInfo{sizeof(renderable->transformationMatrix), VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
            memcpy(renderable->transformationBuffer.create(&renderEngineLink, &transformationBufferCreateInfo), &renderable->transformationMatrix, sizeof(VkTransformMatrixKHR));
            renderable->deletionQueue.emplace_front([&](VulkanRenderable thisRenderable) { thisRenderable.transformationBuffer.destroy(); });
        }
        //upload textures
        renderable->textureImages.resize(renderable->textures.size());
        for (unsigned int i = 0; i < renderable->textures.size(); ++i) {
            renderable->textureImages[i].destroy();
            VulkanTexture::CreateInfo textureImageCreateInfo{VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, renderable->textureImages[i].createdWith.width, renderable->textureImages[i].createdWith.height};
            textureImageCreateInfo.filename = renderable->textures[i];
            textureImageCreateInfo.mipMapping = settings.mipMapping;
            renderable->textureImages[i].create(&renderEngineLink, &textureImageCreateInfo);
        }
        //build uniform buffers
        VulkanBuffer::CreateInfo uniformBufferCreateInfo {sizeof(VulkanUniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        memcpy(renderable->uniformBuffer.create(&renderEngineLink, &uniformBufferCreateInfo), &renderable->uniformBufferObject, sizeof(VulkanUniformBufferObject));
        renderable->deletionQueue.emplace_front([&](VulkanRenderable thisRenderable){ thisRenderable.uniformBuffer.destroy(); });
        //build shaders
        renderable->shaders.resize(renderable->shaderCreateInfos.size());
        for (int i = 0; i < renderable->shaderCreateInfos.size(); ++i) { renderable->shaders[i].create(&renderEngineLink, &renderable->shaderCreateInfos[i]); }
        renderable->deletionQueue.emplace_front([&](const VulkanRenderable& thisRenderable) { for (const VulkanShader& shader : thisRenderable.shaders) { shader.destroy(); } });
        /* This setup is temporary, but for the purposes of testing only the most recently uploaded model will be included in the raytracing. */
        if (settings.rayTracing) {
            VulkanAccelerationStructure::CreateInfo renderableBottomLevelAccelerationStructureCreateInfo{};
            renderableBottomLevelAccelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
            renderableBottomLevelAccelerationStructureCreateInfo.transformationMatrix = &identityTransformMatrix;
            renderableBottomLevelAccelerationStructureCreateInfo.primitiveCount = renderable->triangleCount;
            renderableBottomLevelAccelerationStructureCreateInfo.vertexBufferAddress = renderable->vertexBuffer.deviceAddress;
            renderableBottomLevelAccelerationStructureCreateInfo.indexBufferAddress = renderable->indexBuffer.deviceAddress;
            renderableBottomLevelAccelerationStructureCreateInfo.transformationBufferAddress = renderable->transformationBuffer.deviceAddress;
            renderable->bottomLevelAccelerationStructure.create(&renderEngineLink, &renderableBottomLevelAccelerationStructureCreateInfo);
            renderable->deletionQueue.emplace_front([&] (VulkanRenderable thisRenderable) { thisRenderable.bottomLevelAccelerationStructure.destroy(); });
            topLevelAccelerationStructure.destroy();
            VulkanAccelerationStructure::CreateInfo topLevelAccelerationStructureCreateInfo{};
            topLevelAccelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
            topLevelAccelerationStructureCreateInfo.transformationMatrix = &identityTransformMatrix;
            topLevelAccelerationStructureCreateInfo.primitiveCount = 1;
            topLevelAccelerationStructureCreateInfo.bottomLevelAccelerationStructureDeviceAddress = renderable->bottomLevelAccelerationStructure.deviceAddress;
            topLevelAccelerationStructure.create(&renderEngineLink, &topLevelAccelerationStructureCreateInfo);
        }
        //build graphics pipeline and descriptor set for this renderable
        DescriptorSet::CreateInfo renderableDescriptorSetCreateInfo{};
        if (settings.rayTracing) {
            renderableDescriptorSetCreateInfo.poolSizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1}};
            renderableDescriptorSetCreateInfo.shaderStages = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
            renderableDescriptorSetCreateInfo.data = {&renderable->uniformBuffer, &renderable->textureImages[0], &topLevelAccelerationStructure};
        } else {
            renderableDescriptorSetCreateInfo.poolSizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};
            renderableDescriptorSetCreateInfo.shaderStages = {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
            renderableDescriptorSetCreateInfo.data = {&renderable->uniformBuffer, &renderable->textureImages[0]};
        }
        renderable->descriptorSet.create(&renderEngineLink, &renderableDescriptorSetCreateInfo);
        renderable->deletionQueue.emplace_front([&](VulkanRenderable thisRenderable) { thisRenderable.descriptorSet.destroy(); });
        VulkanPipeline::CreateInfo renderablePipelineCreateInfo{};
        renderablePipelineCreateInfo.descriptorSet = &renderable->descriptorSet;
        renderablePipelineCreateInfo.renderPass = &renderPass;
        renderablePipelineCreateInfo.shaders = renderable->shaders;
        renderable->pipeline.create(&renderEngineLink, &renderablePipelineCreateInfo);
        renderable->deletionQueue.emplace_front([&](VulkanRenderable thisRenderable) { thisRenderable.pipeline.destroy(); });
        if (append) { renderables.push_back(renderable); }
    }

    bool update() {
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
        commandBuffer.resetCommandBuffer((int)(imageIndex + (swapchain.image_count - 1)) % (int)swapchain.image_count);
        commandBuffer.recordCommandBuffer((int)imageIndex);
        VkViewport viewport{};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = (float)swapchain.extent.width;
        viewport.height = (float)swapchain.extent.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        vkCmdSetViewport(commandBuffer.commandBuffers[imageIndex], 0, 1, &viewport);
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapchain.extent;
        vkCmdSetScissor(commandBuffer.commandBuffers[imageIndex], 0, 1, &scissor);
        VkRenderPassBeginInfo renderPassBeginInfo = renderPass.beginRenderPass(imageIndex);
        vkCmdBeginRenderPass(commandBuffer.commandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(commandBuffer.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, renderables[0]->pipeline.pipeline);
        camera.update();
        for (VulkanRenderable *renderable : renderables) {
            if (renderable->render) {
                //update renderable
                renderable->update(camera);
                //record command buffer for this renderable
                vkCmdBindVertexBuffers(commandBuffer.commandBuffers[imageIndex], 0, 1, &renderable->vertexBuffer.buffer, offsets);
                vkCmdBindIndexBuffer(commandBuffer.commandBuffers[imageIndex], renderable->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(commandBuffer.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, renderable->pipeline.pipelineLayout, 0, 1, &renderable->descriptorSet.descriptorSet, 0, nullptr);
                vkCmdDrawIndexed(commandBuffer.commandBuffers[imageIndex], static_cast<uint32_t>(renderable->indices.size()), 1, 0, 0, 0);
            }
        }
        vkCmdEndRenderPass(commandBuffer.commandBuffers[imageIndex]);
        if (vkEndCommandBuffer(commandBuffer.commandBuffers[imageIndex]) != VK_SUCCESS) { throw std::runtime_error("failed to record command buffer!"); }
        //Submit
        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer.commandBuffers[imageIndex];
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

    GLFWmonitor *monitor{};
    GLFWwindow *window{};
    VulkanSettings settings{};
    VulkanCamera camera{};
    VulkanRenderPass renderPass{};
    VulkanGraphicsEngineLink renderEngineLink{};
    float frameTime{};
    int frameNumber{};

private:
    vkb::Swapchain swapchain{};
    vkb::Instance instance{};
    vkb::Device device{};
    VmaAllocator allocator{};
    VkSurfaceKHR surface{};
    VkTransformMatrixKHR identityTransformMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};
    VkQueue transferQueue{};
    VkQueue computeQueue{};
    std::vector<VkImageView> swapchainImageViews{};
    std::vector<VkFence> inFlightFences{};
    std::vector<VkFence> imagesInFlight{};
    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<VulkanRenderable *> renderables{};
    VulkanAccelerationStructure topLevelAccelerationStructure{};
    VulkanCommandBuffer commandBuffer{};
    VulkanBuffer scratchBuffer{};
    std::deque<std::function<void()>> engineDeletionQueue{};
    std::deque<std::function<void()>> oneTimeOptionalDeletionQueue{};
    std::deque<std::function<void()>> recreationDeletionQueue{};
    size_t currentFrame{};
    bool framebufferResized{false};
    float previousTime{};

    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
        auto vulkanRenderEngine = (VulkanRenderEngine *)glfwGetWindowUserPointer(pWindow);
        vulkanRenderEngine->framebufferResized = true;
        vulkanRenderEngine->settings.resolution[0] = width;
        vulkanRenderEngine->settings.resolution[1] = height;
    }
};