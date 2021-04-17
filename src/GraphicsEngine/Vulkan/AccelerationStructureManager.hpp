#pragma once

#include "BufferManager.hpp"

class AccelerationStructureManager : public BufferManager {
public:
    VkAccelerationStructureKHR accelerationStructure{};
    VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegion{};
    uint32_t bufferAddress{};

    void *create (VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage allocationUsage, VkAccelerationStructureTypeKHR type, VkAccelerationStructureBuildSizesInfoKHR accelerationStructureBuildSizesInfo) {
        bufferSize = size;
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = usage;
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = allocationUsage;
        if (vmaCreateBuffer(*linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) { throw std::runtime_error("failed to create buffer"); }
        deletionQueue.emplace_front([&]{ if (buffer != VK_NULL_HANDLE) { vmaDestroyBuffer(*linkedRenderEngine->allocator, buffer, allocation); buffer = VK_NULL_HANDLE; } });
        vmaMapMemory(*linkedRenderEngine->allocator, allocation, &data);
        deletionQueue.emplace_front([&]{ vmaUnmapMemory(*linkedRenderEngine->allocator, allocation); });
        if (usage == (VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)) {
            VkAccelerationStructureCreateInfoKHR accelerationStructureCreate_info{};
            accelerationStructureCreate_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
            accelerationStructureCreate_info.buffer = buffer;
            accelerationStructureCreate_info.size = accelerationStructureBuildSizesInfo.accelerationStructureSize;
            accelerationStructureCreate_info.type = type;
            vkCreateAccelerationStructureKHR(linkedRenderEngine->device->device, &accelerationStructureCreate_info, nullptr, &accelerationStructure);
        }
        deletionQueue.emplace_front([&]{ vkDestroyAccelerationStructureKHR(linkedRenderEngine->device->device, accelerationStructure, nullptr); });
        if (usage == (VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)) {
            VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
            bufferDeviceAddressInfo.buffer = buffer;
            bufferAddress = vkGetBufferDeviceAddress(linkedRenderEngine->device->device, &bufferDeviceAddressInfo);
        }
        return data;
    }
};