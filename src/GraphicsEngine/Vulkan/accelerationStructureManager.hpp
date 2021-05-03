#pragma once

#include "bufferManager.hpp"

/** This class sets up the acceleration structure for Vulkan*/
class AccelerationStructureManager : public BufferManager {
public:
    /** This creates a local instance of the Vulkan acceleration structure.*/
    VkAccelerationStructureKHR accelerationStructure{};
    /** This creates the a local instance of a Vulkan Strided Device Address Region*/
    VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegion{};

    /** This method creates the Vulkan Acceleration Structure Manager.
     * @param accelerationStructureBuildSizesInfo
     * @param allocationUsage This is the memory allocated.
     * @param type This is the acceleration structure type.
     * @param usage This is the buffer usage flags that hold the buffer info.
     * @return data*/
    void *create (VkBufferUsageFlags usage, VmaMemoryUsage allocationUsage, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo) {
        bufferSize = accelerationStructureBuildSizesInfo.accelerationStructureSize;
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
        bufferCreateInfo.usage = usage;
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = allocationUsage;
        if (vmaCreateBuffer(*linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) { throw std::runtime_error("failed to create buffer"); }
        deletionQueue.emplace_front([&]{ if (buffer != VK_NULL_HANDLE) { vmaDestroyBuffer(*linkedRenderEngine->allocator, buffer, allocation); buffer = VK_NULL_HANDLE; } });
        if (usage == (VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)) {
            VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
            accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
            accelerationStructureCreateInfo.buffer = buffer;
            accelerationStructureCreateInfo.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
            accelerationStructureCreateInfo.type = type;
            vkCreateAccelerationStructureKHR(linkedRenderEngine->device->device, &accelerationStructureCreateInfo, nullptr, &accelerationStructure);
            deletionQueue.emplace_front([&]{ linkedRenderEngine->vkDestroyAccelerationStructureKHR(linkedRenderEngine->device->device, accelerationStructure, nullptr); });
        }
        deletionQueue.emplace_front([&]{ linkedRenderEngine->vkDestroyAccelerationStructureKHR(linkedRenderEngine->device->device, accelerationStructure, nullptr); });
        if (usage == (VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)) {
            VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
            bufferDeviceAddressInfo.buffer = buffer;
            bufferAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device->device, &bufferDeviceAddressInfo);
        }
        vmaMapMemory(*linkedRenderEngine->allocator, allocation, &data);
        deletionQueue.emplace_front([&]{ if (buffer != VK_NULL_HANDLE) { vmaUnmapMemory(*linkedRenderEngine->allocator, allocation); } });
        return data;
    }
};