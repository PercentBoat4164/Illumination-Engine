#pragma once

#include "BufferManager.hpp"

class ShaderBindingTableManager : public BufferManager {
public:
    VkDeviceAddress bufferAddress{};
    VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegion{};

    void *create(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage allocationUsage, uint32_t handleCount) {
        bufferSize = size;
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = usage;
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = allocationUsage;
        if (vmaCreateBuffer(*linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) { throw std::runtime_error("failed to create buffer"); }
        deletionQueue.emplace_front([&]{ if (buffer != VK_NULL_HANDLE) { vmaDestroyBuffer(*linkedRenderEngine->allocator, buffer, allocation); buffer = VK_NULL_HANDLE; } });
        if (usage == (VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)) {
            VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
            bufferDeviceAddressInfo.buffer = buffer;
            bufferAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device->device, &bufferDeviceAddressInfo);
        } if (usage == (VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR)) {
            const uint32_t handleSizeAligned = (linkedRenderEngine->physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleSize + linkedRenderEngine->physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleAlignment - 1) & ~(linkedRenderEngine->physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleAlignment - 1);
            stridedDeviceAddressRegion.deviceAddress = bufferAddress;
            stridedDeviceAddressRegion.stride = handleSizeAligned;
            stridedDeviceAddressRegion.size = handleCount * handleSizeAligned;
        }
        vmaMapMemory(*linkedRenderEngine->allocator, allocation, &data);
        deletionQueue.emplace_front([&]{ vmaUnmapMemory(*linkedRenderEngine->allocator, allocation); });
        return data;
    }
};