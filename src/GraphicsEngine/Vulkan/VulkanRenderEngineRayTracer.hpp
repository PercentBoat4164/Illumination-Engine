#pragma once

#include <vector>

#include "BufferManager.hpp"
#include "GPUData.hpp"
#include "Asset.hpp"
#include "VulkanRenderEngine.hpp"
#include "AccelerationStructureManager.hpp"
#include "Vertex.hpp"
#include "ShaderBindingTableManager.hpp"
#include "DescriptorSetManager.hpp"

class VulkanRenderEngineRayTracer : public VulkanRenderEngine {
private:
    struct ShaderBindingTables {
        ShaderBindingTableManager rayGen{};
        ShaderBindingTableManager miss{};
        ShaderBindingTableManager hit{};
        ShaderBindingTableManager callable{};
    };

public:
    AccelerationStructureManager bottomLevelAccelerationStructure{};
    AccelerationStructureManager topLevelAccelerationStructure{};
    ShaderBindingTables shaderBindingTables{};
    UniformBufferObject uniformBufferObject{};
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
    std::vector<uint32_t> geometryCounts{};
    std::vector<VkAccelerationStructureGeometryKHR> accelerationStructureGeometries{};
    VkPipeline pipeline{};
    float frameTime{};

    bool update() override {
       return true;
    }

    VulkanRenderEngineRayTracer() {
        //One bottom level acceleration structure for each asset.
        for (Asset *asset : assets) {
            VkDeviceOrHostAddressConstKHR vertexBufferDeviceAddress{};
            vertexBufferDeviceAddress.deviceAddress = asset->vertexBuffer.bufferAddress;
            VkDeviceOrHostAddressConstKHR indexBufferDeviceAddress{};
            indexBufferDeviceAddress.deviceAddress = asset->indexBuffer.bufferAddress;
            VkDeviceOrHostAddressConstKHR transformationBufferDeviceAddress{};
            transformationBufferDeviceAddress.deviceAddress = asset->transformationBuffer.bufferAddress;
            VkAccelerationStructureGeometryKHR accelerationStructureGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
            accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
            accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
            accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
            accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
            accelerationStructureGeometry.geometry.triangles.vertexData = vertexBufferDeviceAddress;
            accelerationStructureGeometry.geometry.triangles.maxVertex = 3;
            accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
            accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
            accelerationStructureGeometry.geometry.triangles.indexData = indexBufferDeviceAddress;
            accelerationStructureGeometry.geometry.triangles.transformData = transformationBufferDeviceAddress;
            accelerationStructureGeometries.push_back(accelerationStructureGeometry);
            geometryCounts.push_back(1);
        }
        VkAccelerationStructureBuildGeometryInfoKHR bottomLevelAccelerationStructureBuildGeometryInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
        bottomLevelAccelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        bottomLevelAccelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        bottomLevelAccelerationStructureBuildGeometryInfo.geometryCount = accelerationStructureGeometries.size();
        bottomLevelAccelerationStructureBuildGeometryInfo.pGeometries = accelerationStructureGeometries.data();
        VkAccelerationStructureBuildSizesInfoKHR bottomLevelAccelerationStructureBuildSizesInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
        renderEngineLink.vkGetAccelerationStructureBuildSizesKHR(device.device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &bottomLevelAccelerationStructureBuildGeometryInfo, geometryCounts.data(), &bottomLevelAccelerationStructureBuildSizesInfo);
        bottomLevelAccelerationStructure.setEngineLink(&renderEngineLink);
        bottomLevelAccelerationStructure.create(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR, bottomLevelAccelerationStructureBuildSizesInfo);
        BufferManager scratchBuffer{};
        scratchBuffer.setEngineLink(&renderEngineLink);
        scratchBuffer.create(bottomLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        bottomLevelAccelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        bottomLevelAccelerationStructureBuildGeometryInfo.dstAccelerationStructure = bottomLevelAccelerationStructure.accelerationStructure;
        bottomLevelAccelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.bufferAddress;
        std::vector<VkAccelerationStructureBuildRangeInfoKHR> bottomLevelAccelerationStructureBuildRangeInfos{};
        for (Asset *asset : assets) {
            VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
            accelerationStructureBuildRangeInfo.primitiveCount = asset->triangleCount;
            accelerationStructureBuildRangeInfo.primitiveOffset = 0;
            accelerationStructureBuildRangeInfo.firstVertex = 0;
            accelerationStructureBuildRangeInfo.transformOffset = 0;
            bottomLevelAccelerationStructureBuildRangeInfos.push_back(accelerationStructureBuildRangeInfo);
        }
        std::vector<VkAccelerationStructureBuildRangeInfoKHR *> pAccelerationStructureBuildRangeInfos{};
        for (VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo : bottomLevelAccelerationStructureBuildRangeInfos) { pAccelerationStructureBuildRangeInfos.push_back(&accelerationStructureBuildRangeInfo); }
        if (renderEngineLink.physicalDeviceInfo->physicalDeviceAccelerationStructureFeatures.accelerationStructureHostCommands) { renderEngineLink.vkBuildAccelerationStructuresKHR(renderEngineLink.device->device, VK_NULL_HANDLE, 1, &bottomLevelAccelerationStructureBuildGeometryInfo, pAccelerationStructureBuildRangeInfos.data()); }
        else {
            VkCommandBuffer commandBuffer = renderEngineLink.beginSingleTimeCommands();
            renderEngineLink.vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &bottomLevelAccelerationStructureBuildGeometryInfo, pAccelerationStructureBuildRangeInfos.data());
            renderEngineLink.endSingleTimeCommands(commandBuffer);
        }
        // Destroy original scratch buffer here

        // Create top level acceleration structures

        VkTransformMatrixKHR transformationMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        VkAccelerationStructureInstanceKHR accelerationStructureInstance{};
        accelerationStructureInstance.transform = transformationMatrix;
        accelerationStructureInstance.instanceCustomIndex = 0;
        accelerationStructureInstance.mask = 0xFF;
        accelerationStructureInstance.instanceShaderBindingTableRecordOffset = 0;
        accelerationStructureInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
        accelerationStructureInstance.accelerationStructureReference = bottomLevelAccelerationStructure.bufferAddress;
        BufferManager instancesBuffer{};
        instancesBuffer.setEngineLink(&renderEngineLink);
        memcpy(instancesBuffer.create(sizeof(VkAccelerationStructureInstanceKHR), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_CPU_TO_GPU), &accelerationStructureInstance, sizeof(VkAccelerationStructureInstanceKHR));
        VkDeviceOrHostAddressConstKHR instanceDataDeviceAddress{};
        instanceDataDeviceAddress.deviceAddress = instancesBuffer.bufferAddress;
        VkAccelerationStructureGeometryKHR accelerationStructureGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
        accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
        accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
        accelerationStructureGeometry.geometry.instances.data = instanceDataDeviceAddress;
        VkAccelerationStructureBuildGeometryInfoKHR topLevelAccelerationStructureBuildGeometryInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
        topLevelAccelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        topLevelAccelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        topLevelAccelerationStructureBuildGeometryInfo.geometryCount = 1;
        topLevelAccelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
        uint32_t primitiveCount = 1;
        VkAccelerationStructureBuildSizesInfoKHR topLevelAccelerationStructureBuildSizesInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
        renderEngineLink.vkGetAccelerationStructureBuildSizesKHR(renderEngineLink.device->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &topLevelAccelerationStructureBuildGeometryInfo, &primitiveCount, &topLevelAccelerationStructureBuildSizesInfo);
        topLevelAccelerationStructure.setEngineLink(&renderEngineLink);
        topLevelAccelerationStructure.create(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR, topLevelAccelerationStructureBuildSizesInfo);
        scratchBuffer.create(topLevelAccelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
        accelerationStructureBuildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        accelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        accelerationStructureBuildGeometryInfo.dstAccelerationStructure = topLevelAccelerationStructure.accelerationStructure;
        accelerationStructureBuildGeometryInfo.geometryCount = 1;
        accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
        accelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.bufferAddress;
        VkAccelerationStructureBuildRangeInfoKHR accelerationStructureBuildRangeInfo{};
        accelerationStructureBuildRangeInfo.primitiveCount = 1;
        accelerationStructureBuildRangeInfo.primitiveOffset = 0;
        accelerationStructureBuildRangeInfo.firstVertex = 0;
        accelerationStructureBuildRangeInfo.transformOffset = 0;
        std::vector<VkAccelerationStructureBuildRangeInfoKHR *> topLevelAccelerationStructureBuildRangeInfos{};
        if (renderEngineLink.physicalDeviceInfo->physicalDeviceAccelerationStructureFeatures.accelerationStructureHostCommands) { vkBuildAccelerationStructuresKHR(renderEngineLink.device->device, VK_NULL_HANDLE, 1, &topLevelAccelerationStructureBuildGeometryInfo, topLevelAccelerationStructureBuildRangeInfos.data()); }
        else {
            VkCommandBuffer commandBuffer = renderEngineLink.beginSingleTimeCommands();
            vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &topLevelAccelerationStructureBuildGeometryInfo, topLevelAccelerationStructureBuildRangeInfos.data());
            renderEngineLink.endSingleTimeCommands(commandBuffer);
        }
        scratchBuffer.destroy();

        // Create SBTs

        const uint32_t handleSize = renderEngineLink.physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleSize;
        const uint32_t handleSizeAligned = (renderEngineLink.physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleSize + renderEngineLink.physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleAlignment - 1) & ~(renderEngineLink.physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleAlignment - 1);
        const uint32_t groupCount = shaderGroups.size();
        const uint32_t SBTSize = groupCount * handleSizeAligned;
        std::vector<uint8_t> shaderHandleStorage(SBTSize);
        if (renderEngineLink.vkGetRayTracingShaderGroupHandlesKHR(renderEngineLink.device->device, pipeline, 0, groupCount, SBTSize, shaderHandleStorage.data()) != VK_SUCCESS) { throw std::runtime_error("failed to get raytracing shader group handles."); }
        memcpy(shaderBindingTables.rayGen.create(handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 1), shaderHandleStorage.data(), handleSize);
        memcpy(shaderBindingTables.miss.create(handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 1), shaderHandleStorage.data() + handleSizeAligned, handleSize);
        memcpy(shaderBindingTables.hit.create(handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, 1), shaderHandleStorage.data() + handleSizeAligned * 2, handleSize);
        memcpy(shaderBindingTables.callable.create(handleSize, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, assets.size()), shaderHandleStorage.data() + handleSizeAligned * 3, handleSize * assets.size());
    }
};