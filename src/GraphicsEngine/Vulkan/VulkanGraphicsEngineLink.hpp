#pragma once

#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

class FramebufferManager{};

class VulkanGraphicsEngineLink {
public:
    Settings *settings = nullptr;
    vkb::Device *device{};
    vkb::Swapchain *swapchain{};
    VkCommandPool *commandPool{};
    VmaAllocator *allocator{};
    FramebufferManager *framebufferManager{};

    [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = *commandPool;
        allocInfo.commandBufferCount = 1;
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device->device, &allocInfo, &commandBuffer);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const {
        vkEndCommandBuffer(commandBuffer);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        vkQueueSubmit(device->get_queue(vkb::QueueType::graphics).value(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device->get_queue(vkb::QueueType::graphics).value());
        vkFreeCommandBuffers(device->device, *commandPool, 1, &commandBuffer);
    }
};