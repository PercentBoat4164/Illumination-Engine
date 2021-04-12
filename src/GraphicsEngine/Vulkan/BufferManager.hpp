#pragma once

#include <deque>
#include <functional>

#include <vk_mem_alloc.h>

#include "VulkanGraphicsEngineLink.hpp"

class BufferManager {
public:
    void *data{};
    VkBuffer buffer{};
    VkDeviceSize bufferSize{};

    void destroy() {
        for (std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
        data = nullptr;
    }

    void setEngineLink(VulkanGraphicsEngineLink *engineLink) {
        linkedRenderEngine = engineLink;
    }

    void *create(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage allocationUsage) {
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
        return data;
    }

    void toImage(VkImage image, uint32_t width, uint32_t height) const {
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
        VkCommandBuffer commandBuffer = linkedRenderEngine->beginSingleTimeCommands();
        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        linkedRenderEngine->endSingleTimeCommands(commandBuffer);
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    VmaAllocation allocation{};
};