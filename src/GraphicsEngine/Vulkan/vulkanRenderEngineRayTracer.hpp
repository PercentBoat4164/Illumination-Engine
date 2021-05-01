#pragma once

#include <vector>

#include "bufferManager.hpp"
#include "gpuData.hpp"
#include "asset.hpp"
#include "vulkanRenderEngine.hpp"
#include "accelerationStructureManager.hpp"
#include "vertex.hpp"
#include "shaderBindingTableManager.hpp"
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
        if (settings.pathTracing) {
            asset->transformationBuffer.setEngineLink(&renderEngineLink);
            memcpy(asset->transformationBuffer.create(sizeof(asset->transformationMatrix), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_CPU_TO_GPU), &asset->transformationMatrix, sizeof(asset->transformationMatrix));
            asset->deletionQueue.emplace_front([&](Asset thisAsset) { thisAsset.transformationBuffer.destroy(); });
        }
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

        //TODO: add this section to the accelerationStructureManager.hpp file
        VkAccelerationStructureGeometryKHR accelerationStructureGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
        accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
        accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
        accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
        accelerationStructureGeometry.geometry.triangles.vertexData = {asset->vertexBuffer.bufferAddress};
        accelerationStructureGeometry.geometry.triangles.maxVertex = 3;
        accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
        accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
        accelerationStructureGeometry.geometry.triangles.indexData = {asset->indexBuffer.bufferAddress};
        accelerationStructureGeometry.geometry.triangles.transformData = {asset->transformationBuffer.bufferAddress};
        uint32_t geometryCount = 1;
        VkAccelerationStructureBuildGeometryInfoKHR bottomLevelAccelerationStructureBuildGeometryInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
        bottomLevelAccelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        bottomLevelAccelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        bottomLevelAccelerationStructureBuildGeometryInfo.geometryCount = 1;
        bottomLevelAccelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
        VkAccelerationStructureBuildSizesInfoKHR bottomLevelAccelerationStructureBuildSizesInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
        renderEngineLink.vkGetAccelerationStructureBuildSizesKHR(device.device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &bottomLevelAccelerationStructureBuildGeometryInfo, &geometryCount, &bottomLevelAccelerationStructureBuildSizesInfo);

        //This will stay here
        asset->bottomLevelAccelerationStructure.create(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, bottomLevelAccelerationStructureBuildSizesInfo);

        //TODO: this part also goes into accelerationStructureManager.hpp
        BufferManager scratchBuffer{};
        scratchBuffer.setEngineLink(&renderEngineLink);
        scratchBuffer.create(bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        bottomLevelAccelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        bottomLevelAccelerationStructureBuildGeometryInfo.dstAccelerationStructure = asset->bottomLevelAccelerationStructure.accelerationStructure;
        bottomLevelAccelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.bufferAddress;
        VkAccelerationStructureBuildRangeInfoKHR bottomLevelAccelerationStructureBuildRangeInfo{};
        bottomLevelAccelerationStructureBuildRangeInfo.primitiveCount = asset->triangleCount;
        bottomLevelAccelerationStructureBuildRangeInfo.primitiveOffset = 0;
        bottomLevelAccelerationStructureBuildRangeInfo.firstVertex = 0;
        bottomLevelAccelerationStructureBuildRangeInfo.transformOffset = 0;
        if (renderEngineLink.physicalDeviceInfo->physicalDeviceAccelerationStructureFeatures.accelerationStructureHostCommands) { renderEngineLink.vkBuildAccelerationStructuresKHR(renderEngineLink.device->device, VK_NULL_HANDLE, 1, &bottomLevelAccelerationStructureBuildGeometryInfo, reinterpret_cast<const VkAccelerationStructureBuildRangeInfoKHR *const *>(&bottomLevelAccelerationStructureBuildRangeInfo)); }
        else {
            VkCommandBuffer commandBuffer = renderEngineLink.beginSingleTimeCommands();
            renderEngineLink.vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &bottomLevelAccelerationStructureBuildGeometryInfo, reinterpret_cast<const VkAccelerationStructureBuildRangeInfoKHR *const *>(&bottomLevelAccelerationStructureBuildRangeInfo));
            renderEngineLink.endSingleTimeCommands(commandBuffer);
        }
        scratchBuffer.destroy();

        //build graphics pipeline and descriptor set for this asset
        DescriptorSetManager::DescriptorSetManagerCreateInfo descriptorSetManagerCreateInfo{};
        descriptorSetManagerCreateInfo.poolSizes = {{VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}, {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1}};
        descriptorSetManagerCreateInfo.shaderStages = {static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR), VK_SHADER_STAGE_RAYGEN_BIT_KHR, static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR};
        descriptorSetManagerCreateInfo.shaderData = asset->shaderData;
        descriptorSetManagerCreateInfo.accelerationStructures = {&asset->bottomLevelAccelerationStructure, nullptr, nullptr, nullptr, nullptr};
        descriptorSetManagerCreateInfo.images = {nullptr, /*TODO: add image for writing results to*/nullptr, nullptr, nullptr, nullptr};
        descriptorSetManagerCreateInfo.buffers = {nullptr, nullptr, &asset->uniformBuffer, &asset->vertexBuffer, &asset->indexBuffer};
        asset->rayTracingPipelineManager.create(&renderEngineLink, &descriptorSetManagerCreateInfo);
        asset->deletionQueue.emplace_front([&](Asset thisAsset) { thisAsset.rayTracingPipelineManager.destroy(); });
        if (append) { assets.push_back(asset); }
    }

    explicit VulkanRenderEngineRayTracer(GLFWwindow *attachWindow = nullptr) : VulkanRenderEngine(attachWindow) {
//        std::vector<uint32_t> geometryCounts{};
//        std::vector<VkAccelerationStructureGeometryKHR> accelerationStructureGeometries{};
//        //One bottom level acceleration structure for each asset.
//        for (Asset *asset : assets) {
//            VkAccelerationStructureGeometryKHR accelerationStructureGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
//            accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
//            accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
//            accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
//            accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
//            accelerationStructureGeometry.geometry.triangles.vertexData = {asset->vertexBuffer.bufferAddress};
//            accelerationStructureGeometry.geometry.triangles.maxVertex = 3;
//            accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
//            accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
//            accelerationStructureGeometry.geometry.triangles.indexData = {asset->indexBuffer.bufferAddress};
//            accelerationStructureGeometry.geometry.triangles.transformData = {asset->transformationBuffer.bufferAddress};
//            accelerationStructureGeometries.push_back(accelerationStructureGeometry);
//            geometryCounts.push_back(1);
//        }
//        VkAccelerationStructureBuildGeometryInfoKHR bottomLevelAccelerationStructureBuildGeometryInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
//        bottomLevelAccelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
//        bottomLevelAccelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
//        bottomLevelAccelerationStructureBuildGeometryInfo.geometryCount = static_cast<uint32_t>(accelerationStructureGeometries.size());
//        bottomLevelAccelerationStructureBuildGeometryInfo.pGeometries = accelerationStructureGeometries.data();
//        VkAccelerationStructureBuildSizesInfoKHR bottomLevelAccelerationStructureBuildSizesInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
//        renderEngineLink.vkGetAccelerationStructureBuildSizesKHR(device.device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &bottomLevelAccelerationStructureBuildGeometryInfo, geometryCounts.data(), &bottomLevelAccelerationStructureBuildSizesInfo);
//        bottomLevelAccelerationStructure.setEngineLink(&renderEngineLink);
//        bottomLevelAccelerationStructure.create(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, bottomLevelAccelerationStructureBuildSizesInfo);
//        BufferManager scratchBuffer{};
//        scratchBuffer.setEngineLink(&renderEngineLink);
//        scratchBuffer.create(bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
//        bottomLevelAccelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
//        bottomLevelAccelerationStructureBuildGeometryInfo.dstAccelerationStructure = bottomLevelAccelerationStructure.accelerationStructure;
//        bottomLevelAccelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.bufferAddress;
//        std::vector<VkAccelerationStructureBuildRangeInfoKHR> bottomLevelAccelerationStructureBuildRangeInfos{};
//        for (Asset *asset : assets) {
//            VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
//            accelerationStructureBuildRangeInfo.primitiveCount = asset->triangleCount;
//            accelerationStructureBuildRangeInfo.primitiveOffset = 0;
//            accelerationStructureBuildRangeInfo.firstVertex = 0;
//            accelerationStructureBuildRangeInfo.transformOffset = 0;
//            bottomLevelAccelerationStructureBuildRangeInfos.push_back(accelerationStructureBuildRangeInfo);
//        }
//        std::vector<VkAccelerationStructureBuildRangeInfoKHR *> pAccelerationStructureBuildRangeInfos{};
//        for (VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo : bottomLevelAccelerationStructureBuildRangeInfos) { pAccelerationStructureBuildRangeInfos.push_back(&accelerationStructureBuildRangeInfo); }
//        if (renderEngineLink.physicalDeviceInfo->physicalDeviceAccelerationStructureFeatures.accelerationStructureHostCommands) { renderEngineLink.vkBuildAccelerationStructuresKHR(renderEngineLink.device->device, VK_NULL_HANDLE, 1, &bottomLevelAccelerationStructureBuildGeometryInfo, pAccelerationStructureBuildRangeInfos.data()); }
//        else {
//            VkCommandBuffer commandBuffer = renderEngineLink.beginSingleTimeCommands();
//            renderEngineLink.vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &bottomLevelAccelerationStructureBuildGeometryInfo, pAccelerationStructureBuildRangeInfos.data());
//            renderEngineLink.endSingleTimeCommands(commandBuffer);
//        }
//        // Destroy original scratch buffer here
//
//        // Create top level acceleration structures
//
//        VkTransformMatrixKHR transformationMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
//        VkAccelerationStructureInstanceKHR accelerationStructureInstance{};
//        accelerationStructureInstance.transform = transformationMatrix;
//        accelerationStructureInstance.instanceCustomIndex = 0;
//        accelerationStructureInstance.mask = 0xFF;
//        accelerationStructureInstance.instanceShaderBindingTableRecordOffset = 0;
//        accelerationStructureInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
//        accelerationStructureInstance.accelerationStructureReference = bottomLevelAccelerationStructure.bufferAddress;
//        BufferManager instancesBuffer{};
//        instancesBuffer.setEngineLink(&renderEngineLink);
//        memcpy(instancesBuffer.create(sizeof(VkAccelerationStructureInstanceKHR), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_CPU_TO_GPU), &accelerationStructureInstance, sizeof(VkAccelerationStructureInstanceKHR));
//        VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
//        instanceDataDeviceAddress.deviceAddress = instancesBuffer.bufferAddress;
//        VkAccelerationStructureGeometryKHR accelerationStructureGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
//        accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
//        accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
//        accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
//        accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
//        accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;
//        VkAccelerationStructureBuildGeometryInfoKHR topLevelAccelerationStructureBuildGeometryInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
//        topLevelAccelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
//        topLevelAccelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
//        topLevelAccelerationStructureBuildGeometryInfo.geometryCount = 1;
//        topLevelAccelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
//        uint32_t primitiveCount = 1;
//        VkAccelerationStructureBuildSizesInfoKHR topLevelAccelerationStructureBuildSizesInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
//        renderEngineLink.vkGetAccelerationStructureBuildSizesKHR(renderEngineLink.device->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &topLevelAccelerationStructureBuildGeometryInfo, &primitiveCount, &topLevelAccelerationStructureBuildSizesInfo);
//        topLevelAccelerationStructure.setEngineLink(&renderEngineLink);
//        topLevelAccelerationStructure.create(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, topLevelAccelerationStructureBuildSizesInfo);
//        scratchBuffer.create(topLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
//        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
//        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
//        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
//        accelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
//        accelerationStructureBuildGeometryInfo.dstAccelerationStructure = topLevelAccelerationStructure.accelerationStructure;
//        accelerationStructureBuildGeometryInfo.geometryCount = 1;
//        accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
//        accelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.bufferAddress;
//        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
//        accelerationStructureBuildRangeInfo.primitiveCount = 1;
//        accelerationStructureBuildRangeInfo.primitiveOffset = 0;
//        accelerationStructureBuildRangeInfo.firstVertex = 0;
//        accelerationStructureBuildRangeInfo.transformOffset = 0;
//        std::vector<VkAccelerationStructureBuildRangeInfoKHR *> topLevelAccelerationStructureBuildRangeInfos{};
//        if (renderEngineLink.physicalDeviceInfo->physicalDeviceAccelerationStructureFeatures.accelerationStructureHostCommands) { renderEngineLink.vkBuildAccelerationStructuresKHR(renderEngineLink.device->device, VK_NULL_HANDLE, 1, &topLevelAccelerationStructureBuildGeometryInfo, topLevelAccelerationStructureBuildRangeInfos.data()); }
//        else {
//            VkCommandBuffer commandBuffer = renderEngineLink.beginSingleTimeCommands();
//            renderEngineLink.vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &topLevelAccelerationStructureBuildGeometryInfo, topLevelAccelerationStructureBuildRangeInfos.data());
//            renderEngineLink.endSingleTimeCommands(commandBuffer);
//        }
//        scratchBuffer.destroy();
//
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
    bool framebufferResized{false};
    RayTracingPipelineManager pipelineManager{};
};