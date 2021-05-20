#pragma once

class ShaderBindingTable : public Buffer {
public:
    VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegion{};

    void *create(VulkanGraphicsEngineLink *engineLink, CreateInfo *createInfo) override {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        bufferSize = createdWith.size;
        VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCreateInfo.size = bufferSize;
        bufferCreateInfo.usage = createdWith.usage;
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = createdWith.allocationUsage;
        if (vmaCreateBuffer(*linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) { throw std::runtime_error("failed to create buffer"); }
        deletionQueue.emplace_front([&]{ if (buffer != VK_NULL_HANDLE) { vmaDestroyBuffer(*linkedRenderEngine->allocator, buffer, allocation); buffer = VK_NULL_HANDLE; } });
        VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
        bufferDeviceAddressInfo.buffer = buffer;
        deviceAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device->device, &bufferDeviceAddressInfo);
        const uint32_t handleSizeAligned = (linkedRenderEngine->physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleSize + linkedRenderEngine->physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleAlignment - 1) & ~(linkedRenderEngine->physicalDeviceInfo->physicalDeviceRayTracingPipelineProperties.shaderGroupHandleAlignment - 1);
        stridedDeviceAddressRegion.deviceAddress = deviceAddress;
        stridedDeviceAddressRegion.stride = handleSizeAligned;
        stridedDeviceAddressRegion.size = createdWith.handleCount * handleSizeAligned;
        vmaMapMemory(*linkedRenderEngine->allocator, allocation, &data);
        deletionQueue.emplace_front([&]{ if (buffer != VK_NULL_HANDLE) { vmaUnmapMemory(*linkedRenderEngine->allocator, allocation); } });
        return data;
    }
};