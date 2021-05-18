#pragma once

class AccelerationStructureManager : public BufferManager {
public:
    struct AccelerationStructureManagerCreateInfo {
        VkAccelerationStructureTypeKHR type{};
        //Only required if type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
        VkDeviceAddress vertexBufferAddress{};
        VkDeviceAddress indexBufferAddress{};
        VkDeviceAddress transformationBufferAddress{};
        uint32_t triangleCount{};
        //Only required if type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
        AccelerationStructureManager *bottomLevelAccelerationStructure{};
        VkTransformMatrixKHR *transformationMatrix{};
    };

    VkAccelerationStructureKHR accelerationStructure{};
    AccelerationStructureManagerCreateInfo *createdWith{};

    void *create (VulkanGraphicsEngineLink *renderEngineLink, AccelerationStructureManagerCreateInfo *createInfo) {
        linkedRenderEngine = renderEngineLink;
        createdWith = createInfo;
        VkAccelerationStructureGeometryKHR accelerationStructureGeometry{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR};
        uint32_t geometryCount = 1;
        BufferManager instancesBuffer{};
        if (createdWith->type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR) {
            accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
            accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
            accelerationStructureGeometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
            accelerationStructureGeometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
            accelerationStructureGeometry.geometry.triangles.vertexData = {createdWith->vertexBufferAddress};
            accelerationStructureGeometry.geometry.triangles.maxVertex = 3;
            accelerationStructureGeometry.geometry.triangles.vertexStride = sizeof(Vertex);
            accelerationStructureGeometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;
            accelerationStructureGeometry.geometry.triangles.indexData = {createdWith->indexBufferAddress};
            accelerationStructureGeometry.geometry.triangles.transformData = {createdWith->transformationBufferAddress};
        } else if (createdWith->type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) {
            VkAccelerationStructureInstanceKHR accelerationStructureInstance{};
            accelerationStructureInstance.transform = *createdWith->transformationMatrix;
            accelerationStructureInstance.mask = 0xFF;
            accelerationStructureInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
            accelerationStructureInstance.accelerationStructureReference = createdWith->bottomLevelAccelerationStructure->deviceAddress;
            accelerationStructureGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
            instancesBuffer.setEngineLink(linkedRenderEngine);
            memcpy(instancesBuffer.create(sizeof(VkAccelerationStructureInstanceKHR), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VMA_MEMORY_USAGE_CPU_TO_GPU), &accelerationStructureInstance, sizeof(VkAccelerationStructureInstanceKHR));
            accelerationStructureGeometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
            accelerationStructureGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
            accelerationStructureGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
            accelerationStructureGeometry.geometry.instances.data = {instancesBuffer.deviceAddress};
        }
        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
        accelerationStructureBuildGeometryInfo.type = createdWith->type;
        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
        accelerationStructureBuildGeometryInfo.geometryCount = geometryCount;
        accelerationStructureBuildGeometryInfo.pGeometries = &accelerationStructureGeometry;
        VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR};
        linkedRenderEngine->vkGetAccelerationStructureBuildSizesKHR(linkedRenderEngine->device->device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &accelerationStructureBuildGeometryInfo, &geometryCount, &accelerationStructureBuildSizesInfo);
        bufferSize = accelerationStructureBuildSizesInfo.accelerationStructureSize;
        VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCreateInfo.size = bufferSize;
        bufferCreateInfo.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        if (vmaCreateBuffer(*linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) { throw std::runtime_error("failed to create buffer"); }
        deletionQueue.emplace_front([&]{ if (buffer != VK_NULL_HANDLE) { vmaDestroyBuffer(*linkedRenderEngine->allocator, buffer, allocation); buffer = VK_NULL_HANDLE; } });
        VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR};
        accelerationStructureCreateInfo.buffer = buffer;
        accelerationStructureCreateInfo.size = bufferSize;
        accelerationStructureCreateInfo.type = createdWith->type;
        linkedRenderEngine->vkCreateAccelerationStructureKHR(linkedRenderEngine->device->device, &accelerationStructureCreateInfo, nullptr, &accelerationStructure);
        deletionQueue.emplace_front([&]{ linkedRenderEngine->vkDestroyAccelerationStructureKHR(linkedRenderEngine->device->device, accelerationStructure, nullptr); });
        VkAccelerationStructureDeviceAddressInfoKHR accelerationStructureDeviceAddressInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR};
        accelerationStructureDeviceAddressInfo.accelerationStructure = accelerationStructure;
        deviceAddress = linkedRenderEngine->vkGetAccelerationStructureDeviceAddressKHR(linkedRenderEngine->device->device, &accelerationStructureDeviceAddressInfo);
        vmaMapMemory(*linkedRenderEngine->allocator, allocation, &data);
        deletionQueue.emplace_front([&]{ if (buffer != VK_NULL_HANDLE) { vmaUnmapMemory(*linkedRenderEngine->allocator, allocation); } });
        BufferManager scratchBuffer{};
        scratchBuffer.setEngineLink(linkedRenderEngine);
        scratchBuffer.create(accelerationStructureBuildSizesInfo.accelerationStructureSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
        accelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        accelerationStructureBuildGeometryInfo.dstAccelerationStructure = accelerationStructure;
        accelerationStructureBuildGeometryInfo.scratchData.deviceAddress = scratchBuffer.deviceAddress;
        VkAccelerationStructureBuildRangeInfoKHR bottomLevelAccelerationStructureBuildRangeInfo{createdWith->triangleCount};
        std::vector<VkAccelerationStructureBuildRangeInfoKHR *> pAccelerationStructureBuildRangeInfo{&bottomLevelAccelerationStructureBuildRangeInfo};
        if (linkedRenderEngine->physicalDeviceInfo->physicalDeviceAccelerationStructureFeatures.accelerationStructureHostCommands) { linkedRenderEngine->vkBuildAccelerationStructuresKHR(linkedRenderEngine->device->device, VK_NULL_HANDLE, 1, &accelerationStructureBuildGeometryInfo, pAccelerationStructureBuildRangeInfo.data()); }
        else {
            VkCommandBuffer commandBuffer = linkedRenderEngine->beginSingleTimeCommands();
            linkedRenderEngine->vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &accelerationStructureBuildGeometryInfo, pAccelerationStructureBuildRangeInfo.data());
            linkedRenderEngine->endSingleTimeCommands(commandBuffer);
        }
        scratchBuffer.destroy();
        if (createdWith->type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR) { instancesBuffer.destroy(); }
        return data;
    }

//    void update() {
//        VkAccelerationStructureBuildGeometryInfoKHR accelerationStructureBuildGeometryInfo{VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR};
//        accelerationStructureBuildGeometryInfo.type = createdWith->type;
//        accelerationStructureBuildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
//        accelerationStructureBuildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
//        accelerationStructureBuildGeometryInfo.srcAccelerationStructure = accelerationStructure;
//        accelerationStructureBuildGeometryInfo.dstAccelerationStructure = accelerationStructure;
//        VkAccelerationStructureBuildRangeInfoKHR bottomLevelAccelerationStructureBuildRangeInfo{createdWith->triangleCount};
//        std::vector<VkAccelerationStructureBuildRangeInfoKHR *> pAccelerationStructureBuildRangeInfo{&bottomLevelAccelerationStructureBuildRangeInfo};
//        if (linkedRenderEngine->physicalDeviceInfo->physicalDeviceAccelerationStructureFeatures.accelerationStructureHostCommands) { linkedRenderEngine->vkBuildAccelerationStructuresKHR(linkedRenderEngine->device->device, VK_NULL_HANDLE, 1, &accelerationStructureBuildGeometryInfo, pAccelerationStructureBuildRangeInfo.data()); }
//        else {
//            VkCommandBuffer commandBuffer = linkedRenderEngine->beginSingleTimeCommands();
//            linkedRenderEngine->vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &accelerationStructureBuildGeometryInfo, pAccelerationStructureBuildRangeInfo.data());
//            linkedRenderEngine->endSingleTimeCommands(commandBuffer);
//        }
//    }
};