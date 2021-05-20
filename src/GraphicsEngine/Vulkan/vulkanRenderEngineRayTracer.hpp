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
    AccelerationStructure topLevelAccelerationStructure{};
    DescriptorSetManager descriptorSetManager{};
    Image rayTracingImage{};
    float frameTime{};
    int frameNumber{};

    bool update() override {
        if (window == nullptr) { return false; }
        if (assets.empty()) { return glfwWindowShouldClose(window) != 1; }
        vkWaitForFences(device.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        uint32_t imageIndex{0};
        VkResult result = vkAcquireNextImageKHR(device.device, swapchain.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex); // Signals semaphore
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
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
        commandBufferManager.resetCommandBuffer((int)(imageIndex + (swapchain.image_count - 1)) % (int)swapchain.image_count);
        commandBufferManager.recordCommandBuffer((int)imageIndex);
        VkImageMemoryBarrier imageMemoryBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        Asset *asset = assets[0];
        asset->update(camera);
        vkCmdBindPipeline(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rayTracingPipelineManager.pipeline);
        vkCmdBindDescriptorSets(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rayTracingPipelineManager.pipelineLayout, 0, 1, &descriptorSetManager.descriptorSet, 0, nullptr);
        renderEngineLink.vkCmdTraceRaysKHR(commandBufferManager.commandBuffers[imageIndex], &shaderBindingTables.rayGen.stridedDeviceAddressRegion, &shaderBindingTables.miss.stridedDeviceAddressRegion, &shaderBindingTables.hit.stridedDeviceAddressRegion, &shaderBindingTables.callable.stridedDeviceAddressRegion, settings.resolution[0], settings.resolution[1], 1);
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.image = swapchain.get_images().value()[imageIndex];
        imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkCmdPipelineBarrier(commandBufferManager.commandBuffers[imageIndex], VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        imageMemoryBarrier.oldLayout = rayTracingImage.imageLayout;
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
        vkCmdCopyImage(commandBufferManager.commandBuffers[imageIndex], rayTracingImage.image, rayTracingImage.imageLayout, swapchain.get_images().value()[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
        imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        imageMemoryBarrier.image = swapchain.get_images().value()[imageIndex];
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

    void uploadAsset(Asset *asset, bool append) override {
        //destroy previously created asset if any
        asset->destroy();
        //upload mesh, vertex, and transformation data if path tracing
        Buffer::CreateInfo vertexBufferCreateInfo {sizeof(asset->vertices[0]) * asset->vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        memcpy(asset->vertexBuffer.create(&renderEngineLink, &vertexBufferCreateInfo), asset->vertices.data(), asset->vertexBuffer.bufferSize);
        asset->deletionQueue.emplace_front([&](Asset thisAsset){ thisAsset.vertexBuffer.destroy(); });
        Buffer::CreateInfo indexBufferCreateInfo {sizeof(asset->indices[0]) * asset->indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        memcpy(asset->indexBuffer.create(&renderEngineLink, &indexBufferCreateInfo), asset->indices.data(), asset->indexBuffer.bufferSize);
        asset->deletionQueue.emplace_front([&](Asset thisAsset){ thisAsset.indexBuffer.destroy(); });
        Buffer::CreateInfo transformationBufferCreateInfo {sizeof(asset->transformationMatrix), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_CPU_TO_GPU};
        memcpy(asset->transformationBuffer.create(&renderEngineLink, &transformationBufferCreateInfo), &asset->transformationMatrix, asset->transformationBuffer.bufferSize);
        asset->deletionQueue.emplace_front([&](Asset thisAsset) { thisAsset.transformationBuffer.destroy(); });
        //upload textures
        asset->textureImages.resize(asset->textures.size());
        for (unsigned int i = 0; i < asset->textures.size(); ++i) {
            scratchBuffer.destroy();
            Buffer::CreateInfo scratchBufferCreateInfo{static_cast<VkDeviceSize>(asset->width * asset->height * 4), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
            memcpy(scratchBuffer.create(&renderEngineLink, &scratchBufferCreateInfo), asset->textures[i], asset->width * asset->height * 4);
            Image::CreateInfo textureImageCreateInfo{VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, asset->width, asset->height};
            textureImageCreateInfo.dataSource = &scratchBuffer;
            asset->textureImages[i].create(&renderEngineLink, &textureImageCreateInfo);
        }
        //build uniform buffers
        Buffer::CreateInfo uniformBufferCreateInfo {sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        memcpy(asset->uniformBuffer.create(&renderEngineLink, &uniformBufferCreateInfo), &asset->uniformBufferObject, sizeof(UniformBufferObject));
        asset->deletionQueue.emplace_front([&](Asset thisAsset){ thisAsset.uniformBuffer.destroy(); });
        //build bottom level acceleration structure
        AccelerationStructure::CreateInfo accelerationStructureManagerCreateInfo{};
        accelerationStructureManagerCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationStructureManagerCreateInfo.vertexBufferAddress = asset->vertexBuffer.deviceAddress;
        accelerationStructureManagerCreateInfo.indexBufferAddress = asset->indexBuffer.deviceAddress;
        accelerationStructureManagerCreateInfo.transformationBufferAddress = asset->transformationBuffer.deviceAddress;
        accelerationStructureManagerCreateInfo.triangleCount = asset->triangleCount;
        asset->bottomLevelAccelerationStructure.create(&renderEngineLink, &accelerationStructureManagerCreateInfo);
        asset->deletionQueue.emplace_front([&](Asset thisAsset) { thisAsset.bottomLevelAccelerationStructure.destroy(); });
        //build top level acceleration structure
        accelerationStructureManagerCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        accelerationStructureManagerCreateInfo.transformationMatrix = &asset->transformationMatrix;
        topLevelAccelerationStructure.destroy();
        topLevelAccelerationStructure.create(&renderEngineLink, &accelerationStructureManagerCreateInfo);
        //build descriptor set
        DescriptorSetManager::CreateInfo descriptorSetManagerCreateInfo{};
        descriptorSetManagerCreateInfo.poolSizes = {{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}};
        descriptorSetManagerCreateInfo.shaderStages = {static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR), VK_SHADER_STAGE_RAYGEN_BIT_KHR, static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR};
        descriptorSetManagerCreateInfo.data = {&topLevelAccelerationStructure, &rayTracingImage, &asset->uniformBuffer, &asset->vertexBuffer, &asset->indexBuffer};
        descriptorSetManager.create(&renderEngineLink, &descriptorSetManagerCreateInfo);
        deletionQueue.emplace_front([&]{ descriptorSetManager.destroy(); });
        //build raytracing shaders
        std::vector<VulkanShader> shaders{4};
        VulkanShader::CreateInfo raygenShaderCreateInfo{"VulkanRayTracingShaders/raygen.rgen", VK_SHADER_STAGE_RAYGEN_BIT_KHR};
        shaders[0].create(&renderEngineLink, &raygenShaderCreateInfo);
        VulkanShader::CreateInfo missShaderCreateInfo{"VulkanRayTracingShaders/miss.rmiss", VK_SHADER_STAGE_MISS_BIT_KHR};
        shaders[1].create(&renderEngineLink, &missShaderCreateInfo);
        VulkanShader::CreateInfo chitShaderCreateInfo{"VulkanRayTracingShaders/closestHit.rchit", VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR};
        shaders[2].create(&renderEngineLink, &chitShaderCreateInfo);
        VulkanShader::CreateInfo callShaderCreateInfo{"VulkanRayTracingShaders/callable.rcall", VK_SHADER_STAGE_CALLABLE_BIT_KHR};
        shaders[3].create(&renderEngineLink, &callShaderCreateInfo);
        //build raytracing pipeline
        RayTracingPipeline::CreateInfo rayTracingPipelineManagerCreateInfo{};
        rayTracingPipelineManagerCreateInfo.shaders = shaders;
        rayTracingPipelineManagerCreateInfo.descriptorSetManager = &descriptorSetManager;
        rayTracingPipelineManager.create(&renderEngineLink, &rayTracingPipelineManagerCreateInfo);
        deletionQueue.emplace_front([&]{ rayTracingPipelineManager.destroy(); });
        // Create SBTs
        // TODO: Move much of this into the ShaderBindingTableManager::Create() function.
        const uint32_t handleSize = renderEngineLink.physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleSize;
        const uint32_t handleSizeAligned = (renderEngineLink.physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleSize + renderEngineLink.physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleAlignment - 1) & ~(renderEngineLink.physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleAlignment - 1);
        const auto groupCount = static_cast<uint32_t>(shaders.size());
        const uint32_t SBTSize = groupCount * handleSizeAligned;
        std::vector<uint8_t> shaderHandleStorage(SBTSize);
        if (renderEngineLink.vkGetRayTracingShaderGroupHandlesKHR(renderEngineLink.device->device, rayTracingPipelineManager.pipeline, 0, groupCount, SBTSize, shaderHandleStorage.data()) != VK_SUCCESS) { throw std::runtime_error("failed to get raytracing shader group handles."); }
        ShaderBindingTable::CreateInfo defaultShaderBindingTableCreateInfo{handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        defaultShaderBindingTableCreateInfo.handleCount = 1;
        memcpy(shaderBindingTables.rayGen.create(&renderEngineLink, &defaultShaderBindingTableCreateInfo), shaderHandleStorage.data(), handleSize);
        deletionQueue.emplace_front([&]{ shaderBindingTables.rayGen.destroy(); });
        memcpy(shaderBindingTables.miss.create(&renderEngineLink, &defaultShaderBindingTableCreateInfo), shaderHandleStorage.data() + handleSizeAligned, handleSize);
        deletionQueue.emplace_front([&]{ shaderBindingTables.miss.destroy(); });
        memcpy(shaderBindingTables.hit.create(&renderEngineLink, &defaultShaderBindingTableCreateInfo), shaderHandleStorage.data() + handleSizeAligned * 2, handleSize);
        deletionQueue.emplace_front([&]{ shaderBindingTables.hit.destroy(); });
        ShaderBindingTable::CreateInfo callableShaderBindingTableCreateInfo{handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        callableShaderBindingTableCreateInfo.handleCount = static_cast<uint32_t>(assets.size()) + 1;
        memcpy(shaderBindingTables.callable.create(&renderEngineLink, &callableShaderBindingTableCreateInfo), shaderHandleStorage.data() + handleSizeAligned * 3, handleSize * (assets.size() + 1));
        deletionQueue.emplace_front([&]{ shaderBindingTables.callable.destroy(); });
        if (append) { assets.push_back(asset); }
    }

    explicit VulkanRenderEngineRayTracer(GLFWwindow *attachWindow = nullptr) : VulkanRenderEngine(attachWindow) {
        Image::CreateInfo rayTracingImageCreateInfo{swapchain.image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY};
        rayTracingImage.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
        rayTracingImage.create(&renderEngineLink, &rayTracingImageCreateInfo);
        deletionQueue.emplace_front([&]{ rayTracingImage.destroy(); });
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
    bool framebufferResized{false};
    float previousTime{};
    int currentFrame{};
};