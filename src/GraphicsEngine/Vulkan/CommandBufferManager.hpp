#pragma once

#include <deque>
#include <functional>
#include <vector>

#include <VkBootstrap.h>

class CommandBufferManager {
public:
    VkCommandPool commandPool{};
    std::vector<VkCommandBuffer> commandBuffers{};

    void destroy() {
        for (const std::function<void()>& function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

    void setup(vkb::Device &engineDevice, vkb::QueueType commandQueue) {
        creationDevice = engineDevice;
        queue = commandQueue;
        VkCommandPoolCreateInfo commandPoolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        commandPoolCreateInfo.queueFamilyIndex = creationDevice.get_queue_index(queue).value();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if (vkCreateCommandPool(creationDevice.device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) { throw std::runtime_error("failed to create command pool!"); }
        deletionQueue.emplace_front([&]{ vkDestroyCommandPool(creationDevice.device, commandPool, nullptr); });
    }

    void createCommandBuffers(int commandBufferCount) {
        commandBuffers.resize(commandBufferCount);
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = commandBuffers.size();
        if (vkAllocateCommandBuffers(creationDevice.device, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) { throw std::runtime_error("failed to allocate command buffers!"); }
    }

    void resetCommandBuffers(const std::vector<int>& resetIndices) {
        for (int i : resetIndices) { this->resetCommandBuffer(i); }
    }

    void resetCommandBuffer(int resetIndex = 0) {
        VkCommandBufferResetFlags commandBufferResetFlags{};
        vkResetCommandBuffer(commandBuffers[resetIndex], commandBufferResetFlags);
    }

    void recordCommandBuffers(const std::vector<int>& recordIndices) {
        for (int i : recordIndices) { this->recordCommandBuffer(i); }
    }

    void recordCommandBuffer(int recordIndex = 0) {
        VkCommandBufferBeginInfo commandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        if (vkBeginCommandBuffer(commandBuffers[recordIndex], &commandBufferBeginInfo) != VK_SUCCESS) { throw std::runtime_error("failed to begin recording command buffer!"); }
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
    vkb::QueueType queue{};
    vkb::Device creationDevice{};
};