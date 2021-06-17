#pragma once

#include "vulkanGraphicsEngineLink.hpp"

#include <deque>
#include <functional>

class Buffer {
public:
    struct CreateInfo {
        //Only required for buffer | shader binding table creation
        VkDeviceSize size{};
        VkBufferUsageFlags usage{};
        VmaMemoryUsage allocationUsage{};

        //Only required for shader binding table creation
        uint32_t handleCount{1};

        //Only required for acceleration structure creation
        VkAccelerationStructureTypeKHR type{};
        VkTransformMatrixKHR *transformationMatrix{};
        uint32_t primitiveCount{1};

        //Only required if type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
        VkDeviceAddress vertexBufferAddress{};
        VkDeviceAddress indexBufferAddress{};
        VkDeviceAddress transformationBufferAddress{};

        //Only required if type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
        VkDeviceAddress bottomLevelAccelerationStructureDeviceAddress{};
    };

    void *data{};
    VkBuffer buffer{};
    VkDeviceSize bufferSize{};
    VkDeviceAddress deviceAddress{};
    CreateInfo createdWith{};

    void destroy() {
        for (std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
        data = nullptr;
    }

    virtual void *create(VulkanGraphicsEngineLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        bufferSize = createdWith.size;
        VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCreateInfo.size = bufferSize;
        bufferCreateInfo.usage = createdWith.usage;
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = createdWith.allocationUsage;
        if (vmaCreateBuffer(*linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) { throw std::runtime_error("failed to create buffer!"); }
        deletionQueue.emplace_front([&]{ if (buffer != VK_NULL_HANDLE) { vmaDestroyBuffer(*linkedRenderEngine->allocator, buffer, allocation); buffer = VK_NULL_HANDLE; } });
        vmaMapMemory(*linkedRenderEngine->allocator, allocation, &data);
        deletionQueue.emplace_front([&]{ if (buffer != VK_NULL_HANDLE) { vmaUnmapMemory(*linkedRenderEngine->allocator, allocation); } });
        if (createdWith.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
            bufferDeviceAddressInfo.buffer = buffer;
            deviceAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device->device, &bufferDeviceAddressInfo);
        }
        return data;
    }

    void toImage(VkImage image, uint32_t width, uint32_t height, VkCommandBuffer commandBuffer = nullptr) const {
        bool noCommandBuffer{false};
        if (commandBuffer == nullptr) { noCommandBuffer = true; }
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};
        if (noCommandBuffer) { commandBuffer = linkedRenderEngine->beginSingleTimeCommands(); }
        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        if (noCommandBuffer) { linkedRenderEngine->endSingleTimeCommands(commandBuffer); }
    }

protected:
    std::deque<std::function<void()>> deletionQueue{};
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    VmaAllocation allocation{};
};