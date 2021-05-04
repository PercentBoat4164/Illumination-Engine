#pragma once

#include "shaderBindingTableManager.hpp"
#include "imageManager.hpp"
#include "accelerationStructureManager.hpp"
#include "descriptorSetManager.hpp"
#include "rayTracingPipelineManager.hpp"

class VulkanRenderEngineRayTracer : public VulkanRenderEngine {
private:
    struct ShaderBindingTables {
        ShaderBindingTableManager rayGen{};
        ShaderBindingTableManager miss{};
        ShaderBindingTableManager hit{};
        ShaderBindingTableManager callable{};
    };

public:
    ShaderBindingTables shaderBindingTables{};
    UniformBufferObject uniformBufferObject{};
    ImageManager rayTracingImage{};
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
    float frameTime{};
    int frameNumber{};

    bool update() override {
        VkCommandBufferBeginInfo commandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        VkImageSubresourceRange imageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

        ++frameNumber;
        return false;
    }

    void uploadAsset(Asset *asset, bool append) override {
        //destroy previously created asset if any
        asset->destroy();
        //upload mesh, vertex, and transformation data if path tracing
        asset->vertexBuffer.setEngineLink(&renderEngineLink);
        memcpy(asset->vertexBuffer.create(sizeof(asset->vertices[0]) * asset->vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), asset->vertices.data(), sizeof(asset->vertices[0]) * asset->vertices.size());
        asset->deletionQueue.emplace_front([&](Asset thisAsset){ thisAsset.vertexBuffer.destroy(); });
        asset->indexBuffer.setEngineLink(&renderEngineLink);
        memcpy(asset->indexBuffer.create(sizeof(asset->indices[0]) * asset->indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), asset->indices.data(), sizeof(asset->indices[0]) * asset->indices.size());
        asset->deletionQueue.emplace_front([&](Asset thisAsset){ thisAsset.indexBuffer.destroy(); });
        asset->transformationBuffer.setEngineLink(&renderEngineLink);
        memcpy(asset->transformationBuffer.create(sizeof(asset->transformationMatrix), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_CPU_TO_GPU), &asset->transformationMatrix, sizeof(asset->transformationMatrix));
        asset->deletionQueue.emplace_front([&](Asset thisAsset) { thisAsset.transformationBuffer.destroy(); });
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
        //build acceleration structures for this asset
        AccelerationStructureManager::BottomLevelAccelerationStructureManagerCreateInfo accelerationStructureManagerCreateInfo{};
        accelerationStructureManagerCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        accelerationStructureManagerCreateInfo.allocationUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        accelerationStructureManagerCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationStructureManagerCreateInfo.vertexBufferAddress = asset->vertexBuffer.bufferAddress;
        accelerationStructureManagerCreateInfo.indexBufferAddress = asset->indexBuffer.bufferAddress;
        accelerationStructureManagerCreateInfo.transformationBufferAddress = asset->transformationBuffer.bufferAddress;
        accelerationStructureManagerCreateInfo.triangleCount = asset->triangleCount;
        asset->bottomLevelAccelerationStructure.create(&renderEngineLink, &accelerationStructureManagerCreateInfo);
        accelerationStructureManagerCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        accelerationStructureManagerCreateInfo.bottomLevelAccelerationStructure = &asset->bottomLevelAccelerationStructure;
        accelerationStructureManagerCreateInfo.transformationMatrix = &asset->transformationMatrix;
        asset->topLevelAccelerationStructure.create(&renderEngineLink, &accelerationStructureManagerCreateInfo);
        //build graphics pipeline and descriptor set for this asset
        DescriptorSetManager::DescriptorSetManagerCreateInfo descriptorSetManagerCreateInfo{};
        descriptorSetManagerCreateInfo.poolSizes = {{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}};
        descriptorSetManagerCreateInfo.shaderStages = {static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR), VK_SHADER_STAGE_RAYGEN_BIT_KHR, static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR};
        descriptorSetManagerCreateInfo.shaderData = asset->shaderData;
        descriptorSetManagerCreateInfo.data = {&asset->bottomLevelAccelerationStructure, &rayTracingImage, &asset->uniformBuffer, &asset->vertexBuffer, &asset->indexBuffer};
        asset->rayTracingPipelineManager.create(&renderEngineLink, &descriptorSetManagerCreateInfo);
        asset->deletionQueue.emplace_front([&](Asset thisAsset) { thisAsset.rayTracingPipelineManager.destroy(); });
        if (append) { assets.push_back(asset); }
    }

    explicit VulkanRenderEngineRayTracer(GLFWwindow *attachWindow = nullptr) : VulkanRenderEngine(attachWindow) {
        rayTracingImage.setEngineLink(&renderEngineLink);
        rayTracingImage.create(swapchain.image_format, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 1, (int)swapchain.extent.width, (int)swapchain.extent.height, COLOR);
        rayTracingImage.transition(rayTracingImage.imageLayout, VK_IMAGE_LAYOUT_GENERAL);
        deletionQueue.emplace_front([&]{ rayTracingImage.destroy(); });
        //TODO: Place the next part in the upload asset function.
//        // Create SBTs
//
//        const uint32_t handleSize = renderEngineLink.physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleSize;
//        const uint32_t handleSizeAligned = (renderEngineLink.physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleSize + renderEngineLink.physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleAlignment - 1) & ~(renderEngineLink.physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleAlignment - 1);
//        const auto groupCount = static_cast<uint32_t>(shaderGroups.size());
//        const uint32_t SBTSize = groupCount * handleSizeAligned;
//        std::vector<uint8_t> shaderHandleStorage(SBTSize);
//        if (renderEngineLink.vkGetRayTracingShaderGroupHandlesKHR(renderEngineLink.device->device, pipeline, 0, groupCount, SBTSize, shaderHandleStorage.data()) != VK_SUCCESS) { throw std::runtime_error("failed to get raytracing shader group handles."); }
//        memcpy(shaderBindingTables.rayGen.create(handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 1), shaderHandleStorage.data(), handleSize);
//        memcpy(shaderBindingTables.miss.create(handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 1), shaderHandleStorage.data() + handleSizeAligned, handleSize);
//        memcpy(shaderBindingTables.hit.create(handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 1), shaderHandleStorage.data() + handleSizeAligned * 2, handleSize);
//        memcpy(shaderBindingTables.callable.create(handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, static_cast<uint32_t>(assets.size())), shaderHandleStorage.data() + handleSizeAligned * 3, handleSize * assets.size());
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
    bool framebufferResized{false};
    RayTracingPipelineManager pipelineManager{};
};