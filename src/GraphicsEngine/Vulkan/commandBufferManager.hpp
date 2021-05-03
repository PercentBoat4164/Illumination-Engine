#pragma once

#include <deque>
#include <functional>
#include <vector>

#include <VkBootstrap.h>

/** This is the CommandBufferManager class.
 * It holds the commands that start/stop everything pertaining to the buffers.*/
class CommandBufferManager {
public:
    /** This variable is a Vulkan Command Pool.*/
    VkCommandPool commandPool{};
    /** This variable is the Vulkan command buffers.*/
    std::vector<VkCommandBuffer> commandBuffers{};

    /** This method clears the deletion queue.*/
    void destroy() {
        for (const std::function<void()>& function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

    /** This method sets up the command buffer manager.
     * @param commandQueue This is the command pool queue.
     * @param engineDevice This is the device used in the creation of the command buffer manager.*/
    void setup(vkb::Device &engineDevice, vkb::QueueType commandQueue) {
        creationDevice = engineDevice;
        queue = commandQueue;
        VkCommandPoolCreateInfo commandPoolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        commandPoolCreateInfo.queueFamilyIndex = creationDevice.get_queue_index(queue).value();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if (vkCreateCommandPool(creationDevice.device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) { throw std::runtime_error("failed to create command pool!"); }
        deletionQueue.emplace_front([&]{ vkDestroyCommandPool(creationDevice.device, commandPool, nullptr); });
    }

    /** This creates the command buffers.
     * @param commandBufferCount This is the number of command buffers to create.*/
    void createCommandBuffers(int commandBufferCount) {
        commandBuffers.resize(commandBufferCount);
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
        if (vkAllocateCommandBuffers(creationDevice.device, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) { throw std::runtime_error("failed to allocate command buffers!"); }
    }

    /** This resets the command buffers.
     * @param resetIndices These are the indices that are reset.*/
    void resetCommandBuffers(const std::vector<int>& resetIndices) {
        for (int i : resetIndices) { this->resetCommandBuffer(i); }
    }

    /** This resets a command buffer.
     * @param resetIndex This is the index that is reset.*/
    void resetCommandBuffer(int resetIndex = 0) {
        VkCommandBufferResetFlags commandBufferResetFlags{};
        vkResetCommandBuffer(commandBuffers[resetIndex], commandBufferResetFlags);
    }

    /** This records the command buffers.
     * @param recordIndices These are the indices that are recorded.*/
    void recordCommandBuffers(const std::vector<int>& recordIndices) {
        for (int i : recordIndices) { this->recordCommandBuffer(i); }
    }

    /** This records a command buffer.
     * @param recordIndex This is the index that is recorded.*/
    void recordCommandBuffer(int recordIndex = 0) {
        VkCommandBufferBeginInfo commandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        if (vkBeginCommandBuffer(commandBuffers[recordIndex], &commandBufferBeginInfo) != VK_SUCCESS) { throw std::runtime_error("failed to begin recording command buffer!"); }
    }

private:
    /** This variable is the queue of items for deletion.*/
    std::deque<std::function<void()>> deletionQueue{};
    /** This is a queue called queue{}.*/
    vkb::QueueType queue{};
    /** This is a device called creationDevice{}.*/
    vkb::Device creationDevice{};
};