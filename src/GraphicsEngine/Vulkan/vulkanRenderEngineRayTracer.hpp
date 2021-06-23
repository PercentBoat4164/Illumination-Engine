#pragma once

#include "vulkanShaderBindingTable.hpp"
#include "vulkanImage.hpp"
#include "vulkanAccelerationStructure.hpp"
#include "vulkanDescriptorSet.hpp"
#include "vulkanRayTracingPipeline.hpp"

class VulkanRenderEngineRayTracer : public VulkanRenderEngine {
private:
    struct ShaderBindingTables {
        ShaderBindingTable rayGen{};
        ShaderBindingTable miss{};
        ShaderBindingTable hit{};
        ShaderBindingTable callable{};
    };

public:
    ShaderBindingTables shaderBindingTables{};
    RayTracingPipeline rayTracingPipelineManager{};
    float frameTime{};
    int frameNumber{};

    bool update() override {
        if (window == nullptr) { return false; }
        if (renderables.empty()) { return glfwWindowShouldClose(window) != 1; }
        vkWaitForFences(device.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        uint32_t imageIndex{0};
        VkResult result = renderEngineLink.vkAcquireNextImageKhr(device.device, swapchain.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex); // Signals semaphore
        if (result == VK_ERROR_OUT_OF_DATE_KHR || framebufferResized) {
            framebufferResized = false;
            rayTracingImage.destroy();
            Image::CreateInfo rayTracingImageCreateInfo{swapchain.image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY};
            rayTracingImage.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            rayTracingImage.create(&renderEngineLink, &rayTracingImageCreateInfo);
            createSwapchain();
            return glfwWindowShouldClose(window) != 1;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { throw std::runtime_error("failed to acquire swapchain image!"); }
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) { vkWaitForFences(device.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX); }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];
        camera.update();
        Renderable *renderable = renderables[0];
        renderable->update(camera);
        descriptorSetManager.update({&camera.uniformBufferObject}, {2});
        commandBufferManager.resetCommandBuffer((int)(imageIndex + (swapchain.image_count - 1)) % (int)swapchain.image_count);
        VkImage swapchainImage = renderEngineLink.swapchainImages[imageIndex];
        commandBufferManager.recordCommandBuffer((int)imageIndex);
        VkImageMemoryBarrier imageMemoryBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        vkCmdBindPipeline(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rayTracingPipelineManager.pipeline);
        vkCmdBindDescriptorSets(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rayTracingPipelineManager.pipelineLayout, 0, 1, &descriptorSetManager.descriptorSet, 0, nullptr);
        renderEngineLink.vkCmdTraceRaysKHR(commandBufferManager.commandBuffers[imageIndex], &shaderBindingTables.rayGen.stridedDeviceAddressRegion, &shaderBindingTables.miss.stridedDeviceAddressRegion, &shaderBindingTables.hit.stridedDeviceAddressRegion, &shaderBindingTables.callable.stridedDeviceAddressRegion, settings.resolution[0], settings.resolution[1], 1);
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.image = swapchainImage;
        imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        imageMemoryBarrier.image = rayTracingImage.image;
        imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        vkCmdPipelineBarrier(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        rayTracingImage.imageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        VkImageCopy copyRegion{};
        copyRegion.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copyRegion.srcOffset = {0, 0, 0};
        copyRegion.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
        copyRegion.dstOffset = {0, 0, 0};
        copyRegion.extent = {static_cast<uint32_t>(settings.resolution[0]), static_cast<uint32_t>(settings.resolution[1]), 1};
        vkCmdCopyImage(commandBufferManager.commandBuffers[imageIndex], rayTracingImage.image, rayTracingImage.imageLayout, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imageMemoryBarrier.image = swapchainImage;
        imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = 0;
        vkCmdPipelineBarrier(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        imageMemoryBarrier.oldLayout = rayTracingImage.imageLayout;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.image = rayTracingImage.image;
        imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.dstAccessMask = 0;
        vkCmdPipelineBarrier(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        rayTracingImage.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        if (vkEndCommandBuffer(commandBufferManager.commandBuffers[imageIndex]) != VK_SUCCESS) { throw std::runtime_error("failed to record command buffer!"); }
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &imageAvailableSemaphores[currentFrame]; // Wait for signaled semaphore
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBufferManager.commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];
        vkResetFences(device.device, 1, &inFlightFences[currentFrame]);
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) { throw std::runtime_error("failed to submit draw command buffer!"); }
        VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapchain.swapchain;
        presentInfo.pImageIndices = &imageIndex;
        result = vkQueuePresentKHR(presentQueue, &presentInfo);
        vkQueueWaitIdle(presentQueue);
        auto currentTime = (float)glfwGetTime();
        frameTime = currentTime - previousTime;
        previousTime = currentTime;
        frameNumber++;
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            rayTracingImage.destroy();
            Image::CreateInfo rayTracingImageCreateInfo{swapchain.image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY};
            rayTracingImage.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
            rayTracingImage.create(&renderEngineLink, &rayTracingImageCreateInfo);
            createSwapchain();
        } else if (result != VK_SUCCESS) { throw std::runtime_error("failed to present swapchain image!"); }
        currentFrame = (currentFrame + 1) % (int)swapchain.image_count;
        return glfwWindowShouldClose(window) != 1;
    }

    void uploadRenderable(Renderable *renderable, bool append) override {
        //destroy previously created renderable if any
        renderable->destroy();
        //upload mesh, vertex, uniform, and transformation data
        Buffer::CreateInfo vertexBufferCreateInfo{sizeof(renderable->vertices[0]) * renderable->vertices.size(), VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        memcpy(renderable->vertexBuffer.create(&renderEngineLink, &vertexBufferCreateInfo), renderable->vertices.data(), sizeof(renderable->vertices[0]) * renderable->vertices.size());
        renderable->deletionQueue.emplace_front([&](Renderable thisRenderable){ thisRenderable.vertexBuffer.destroy(); });
        Buffer::CreateInfo indexBufferCreateInfo{sizeof(renderable->indices[0]) * renderable->indices.size(), VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        memcpy(renderable->indexBuffer.create(&renderEngineLink, &indexBufferCreateInfo), renderable->indices.data(), sizeof(renderable->indices[0]) * renderable->indices.size());
        renderable->deletionQueue.emplace_front([&](Renderable thisRenderable){ thisRenderable.indexBuffer.destroy(); });
        Buffer::CreateInfo transformationBufferCreateInfo{sizeof(renderable->transformationMatrix), VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        memcpy(renderable->transformationBuffer.create(&renderEngineLink, &transformationBufferCreateInfo), &renderable->transformationMatrix, sizeof(VkTransformMatrixKHR));
        renderable->deletionQueue.emplace_front([&](Renderable thisRenderable) { thisRenderable.transformationBuffer.destroy(); });
        Buffer::CreateInfo uniformBufferCreateInfo{sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        memcpy(renderable->uniformBuffer.create(&renderEngineLink, &uniformBufferCreateInfo), &renderable->uniformBufferObject, sizeof(UniformBufferObject));
        renderable->deletionQueue.emplace_front([&](Renderable thisRenderable){ thisRenderable.uniformBuffer.destroy(); });
        //upload textures
        renderable->textureImages.resize(renderable->textures.size());
        for (unsigned int i = 0; i < renderable->textures.size(); ++i) {
            Image::CreateInfo textureImageCreateInfo{VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, renderable->width, renderable->height};
            textureImageCreateInfo.filename = renderable->textures[i];
            renderable->textureImages[i].create(&renderEngineLink, &textureImageCreateInfo);
            renderable->deletionQueue.emplace_front([&](const Renderable& thisRenderable){ for (Texture textureImage : thisRenderable.textureImages) { textureImage.destroy(); } });
        }
        //build bottom level acceleration structure
        AccelerationStructure::CreateInfo accelerationStructureManagerCreateInfo{};
        accelerationStructureManagerCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationStructureManagerCreateInfo.vertexBufferAddress = renderable->vertexBuffer.deviceAddress;
        accelerationStructureManagerCreateInfo.indexBufferAddress = renderable->indexBuffer.deviceAddress;
        accelerationStructureManagerCreateInfo.transformationBufferAddress = renderable->transformationBuffer.deviceAddress;
        accelerationStructureManagerCreateInfo.primitiveCount = renderable->triangleCount;
        accelerationStructureManagerCreateInfo.transformationMatrix = &renderable->transformationMatrix;
        renderable->bottomLevelAccelerationStructure.create(&renderEngineLink, &accelerationStructureManagerCreateInfo);
        renderable->deletionQueue.emplace_front([&](Renderable thisRenderable) { thisRenderable.bottomLevelAccelerationStructure.destroy(); });
        //build top level acceleration structure
        auto identityMatrix = glm::identity<glm::mat4>();
        VkTransformMatrixKHR vkIdentityMatrix = {};
        accelerationStructureManagerCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        accelerationStructureManagerCreateInfo.bottomLevelAccelerationStructureDeviceAddress = renderable->bottomLevelAccelerationStructure.deviceAddress;
        accelerationStructureManagerCreateInfo.accelerationStructureToModify = topLevelAccelerationStructure.accelerationStructure;
        accelerationStructureManagerCreateInfo.transformationMatrix = &renderable->transformationMatrix; // Make an identity matrix for this line. This will work for now because the transformation Matrix is always an identity matrix.
        accelerationStructureManagerCreateInfo.primitiveCount = renderables.size() + 1;
        //TODO: Allow choice of update vs rebuild to user. Currently is rebuild. Create new function in AccelerationStructure class for this.
        topLevelAccelerationStructure.create(&renderEngineLink, &accelerationStructureManagerCreateInfo);
        //build descriptor set
        DescriptorSetManager::CreateInfo descriptorSetManagerCreateInfo{};
        descriptorSetManagerCreateInfo.poolSizes = {{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};
        descriptorSetManagerCreateInfo.shaderStages = {static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR), VK_SHADER_STAGE_RAYGEN_BIT_KHR, static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR};
        descriptorSetManagerCreateInfo.data = {&topLevelAccelerationStructure, &rayTracingImage, std::nullopt, &renderable->vertexBuffer, &renderable->indexBuffer, &renderable->textureImages[0]};
        descriptorSetManager.create(&renderEngineLink, &descriptorSetManagerCreateInfo);
        engineDeletionQueue.emplace_front([&]{ descriptorSetManager.destroy(); });
        //build raytracing shaders
        std::vector<Shader> shaders{4};
        Shader::CreateInfo raygenShaderCreateInfo{"res/Shaders/VulkanRayTracingShaders/raygen.rgen", VK_SHADER_STAGE_RAYGEN_BIT_KHR};
        shaders[0].create(&renderEngineLink, &raygenShaderCreateInfo);
        Shader::CreateInfo missShaderCreateInfo{"res/Shaders/VulkanRayTracingShaders/miss.rmiss", VK_SHADER_STAGE_MISS_BIT_KHR};
        shaders[1].create(&renderEngineLink, &missShaderCreateInfo);
        Shader::CreateInfo chitShaderCreateInfo{"res/Shaders/VulkanRayTracingShaders/closestHit.rchit", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR};
        shaders[2].create(&renderEngineLink, &chitShaderCreateInfo);
        Shader::CreateInfo callShaderCreateInfo{"res/Shaders/VulkanRayTracingShaders/callable.rcall", VK_SHADER_STAGE_CALLABLE_BIT_KHR};
        shaders[3].create(&renderEngineLink, &callShaderCreateInfo);
        //build raytracing pipeline
        RayTracingPipeline::CreateInfo rayTracingPipelineManagerCreateInfo{};
        rayTracingPipelineManagerCreateInfo.shaders = shaders;
        rayTracingPipelineManagerCreateInfo.descriptorSetManager = &descriptorSetManager;
        rayTracingPipelineManager.create(&renderEngineLink, &rayTracingPipelineManagerCreateInfo);
        engineDeletionQueue.emplace_front([&]{ rayTracingPipelineManager.destroy(); });
        // Create SBTs
        const uint32_t handleSize = renderEngineLink.supportedPhysicalDeviceInfo.physicalDeviceRayTracingPipelineProperties.shaderGroupHandleSize;
        const uint32_t handleSizeAligned = (renderEngineLink.supportedPhysicalDeviceInfo.physicalDeviceRayTracingPipelineProperties.shaderGroupHandleSize + renderEngineLink.supportedPhysicalDeviceInfo.physicalDeviceRayTracingPipelineProperties.shaderGroupHandleAlignment - 1) & ~(renderEngineLink.supportedPhysicalDeviceInfo.physicalDeviceRayTracingPipelineProperties.shaderGroupHandleAlignment - 1);
        const auto groupCount = static_cast<uint32_t>(shaders.size());
        const uint32_t SBTSize = groupCount * handleSizeAligned;
        std::vector<uint8_t> shaderHandleStorage(SBTSize);
        if (renderEngineLink.vkGetRayTracingShaderGroupHandlesKHR(renderEngineLink.device->device, rayTracingPipelineManager.pipeline, 0, groupCount, SBTSize, shaderHandleStorage.data()) != VK_SUCCESS) { throw std::runtime_error("failed to get raytracing shader group handles."); }
        ShaderBindingTable::CreateInfo defaultShaderBindingTableCreateInfo{handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        defaultShaderBindingTableCreateInfo.handleCount = 1;
        memcpy(shaderBindingTables.rayGen.create(&renderEngineLink, &defaultShaderBindingTableCreateInfo), shaderHandleStorage.data(), handleSize);
        engineDeletionQueue.emplace_front([&]{ shaderBindingTables.rayGen.destroy(); });
        memcpy(shaderBindingTables.miss.create(&renderEngineLink, &defaultShaderBindingTableCreateInfo), shaderHandleStorage.data() + handleSizeAligned, handleSize);
        engineDeletionQueue.emplace_front([&]{ shaderBindingTables.miss.destroy(); });
        memcpy(shaderBindingTables.hit.create(&renderEngineLink, &defaultShaderBindingTableCreateInfo), shaderHandleStorage.data() + handleSizeAligned * 2, handleSize);
        engineDeletionQueue.emplace_front([&]{ shaderBindingTables.hit.destroy(); });
        ShaderBindingTable::CreateInfo callableShaderBindingTableCreateInfo{handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        callableShaderBindingTableCreateInfo.handleCount = static_cast<uint32_t>(renderables.size()) + 1;
        memcpy(shaderBindingTables.callable.create(&renderEngineLink, &callableShaderBindingTableCreateInfo), shaderHandleStorage.data() + handleSizeAligned * 3, handleSize * (renderables.size() + 1));
        engineDeletionQueue.emplace_front([&]{ shaderBindingTables.callable.destroy(); });
        if (append) { renderables.push_back(renderable); }
    }

    explicit VulkanRenderEngineRayTracer(GLFWwindow *attachWindow = nullptr, bool rayTracing = true) : VulkanRenderEngine(attachWindow, rayTracing) {
        engineDeletionQueue.emplace_front([&]{ topLevelAccelerationStructure.destroy(); });
    }

private:
    bool framebufferResized{false};
    float previousTime{};
    int currentFrame{};
    int vertexSize{sizeof(Vertex)};
};