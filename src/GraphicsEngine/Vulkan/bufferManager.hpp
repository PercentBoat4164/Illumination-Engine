#pragma once

#include <deque>
#include <functional>

#include <vk_mem_alloc.h>

#include "vulkanGraphicsEngineLink.hpp"

/** This is the buffer manager class.*/
class BufferManager {
public:
    /** This creates the data{} variable.*/
    void *data{};
    /** This creates a Vulkan Buffer variable.*/
    VkBuffer buffer{};
    /** This creates a Vulkan Buffer Size variable.*/
    VkDeviceSize bufferSize{};
    /** This creates a Vulkan Buffer Address variable.*/
    VkDeviceAddress bufferAddress{};

    /** This method clears the data and the deletion queue.*/
    void destroy() {
        for (std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
        data = nullptr;
    }

    /** This method sets the linkedRenderEngine to the VulkanGraphicsEngineLink*/
    void setEngineLink(VulkanGraphicsEngineLink *engineLink) {
        linkedRenderEngine = engineLink;
    }

    /** This creates the buffer manager for Vulkan.
     * @param usage This is the Vulkan buffer usage.
     * @param allocationUsage This is the Vulkan memory allocation usage.
     * @param size This is the size of the buffer.
     * @return data*/
    virtual void *create(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage allocationUsage) {
        bufferSize = size;
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = usage;
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = allocationUsage;
        if (vmaCreateBuffer(*linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) { throw std::runtime_error("failed to create buffer!"); }
        deletionQueue.emplace_front([&]{ if (buffer != VK_NULL_HANDLE) { vmaDestroyBuffer(*linkedRenderEngine->allocator, buffer, allocation); buffer = VK_NULL_HANDLE; } });
        if (usage == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
            bufferDeviceAddressInfo.buffer = buffer;
            bufferAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device->device, &bufferDeviceAddressInfo);
        }
        if (usage == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
            bufferDeviceAddressInfo.buffer = buffer;
            bufferAddress = vkGetBufferDeviceAddress(linkedRenderEngine->device->device, &bufferDeviceAddressInfo);
        }
        vmaMapMemory(*linkedRenderEngine->allocator, allocation, &data);
        deletionQueue.emplace_front([&]{ if (buffer != VK_NULL_HANDLE) { vmaUnmapMemory(*linkedRenderEngine->allocator, allocation); } });
        return data;
    }

    /** I have no idea what this does.
     * @todo Figure out what this does for the documentation.*/
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

protected:
    /** This variable is the deletion queue used earlier in the program.*/
    std::deque<std::function<void()>> deletionQueue{};
    /** This is the Vulkan Graphics Engine Link.*/
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    /** This is the memory allocation variable.*/
    VmaAllocation allocation{};
};