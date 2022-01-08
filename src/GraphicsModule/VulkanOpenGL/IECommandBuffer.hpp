#pragma once

#include "IEGraphicsLink.hpp"

#include <vector>

#include <vulkan/vulkan.h>
#include "VkBootstrap.h"

class IECommandBuffer {
public:
    #ifndef NDEBUG
    struct Created {
        bool commandPool{};
        bool commandBuffers{};
    } created;
    #endif
    VkCommandPool commandPool{};
    std::vector<VkCommandBuffer> commandBuffers{};

    void destroy() {
        freeCommandBuffers();
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

    void freeCommandBuffers() {
        for (const std::function<void()> &function : commandBufferDeletionQueue) { function(); }
        commandBufferDeletionQueue.clear();
    };

    void create(IEGraphicsLink *engineLink, vkb::QueueType commandQueue) {
        linkedRenderEngine = engineLink;
        queue = commandQueue;
        VkCommandPoolCreateInfo commandPoolCreateInfo{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
        commandPoolCreateInfo.queueFamilyIndex = linkedRenderEngine->device.get_queue_index(queue).value();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if (vkCreateCommandPool(linkedRenderEngine->device.device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) { throw std::runtime_error("failed to create command pool!"); }
        #ifndef NDEBUG
        created.commandPool = true;
        #endif
        deletionQueue.emplace_back([&] {
            #ifndef NDEBUG
            if (created.commandPool) {
                vkDestroyCommandPool(linkedRenderEngine->device.device, commandPool, nullptr);
                created.commandPool = false;
            }
            #else
            vkDestroyCommandPool(linkedRenderEngine->device.device, commandPool, nullptr);
            #endif
        });
    }

    void createCommandBuffers(int commandBufferCount) {
        commandBuffers.resize(commandBufferCount);
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        commandBufferAllocateInfo.commandPool = commandPool;
        commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
        if (vkAllocateCommandBuffers(linkedRenderEngine->device.device, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) { throw std::runtime_error("failed to allocate command buffers!"); }
        #ifndef NDEBUG
        created.commandBuffers = true;
        #endif
        commandBufferDeletionQueue.emplace_back([&] {
            #ifndef NDEBUG
            if (created.commandBuffers) {
                vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
                created.commandBuffers = false;
            }
            #else
            vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
            #endif
        });
    }

    [[maybe_unused]] void resetCommandBuffer(const std::vector<int> &resetIndices) {
        for (int i : resetIndices) { this->resetCommandBuffer(i); }
    }

    void resetCommandBuffer(int resetIndex = 0) {
        VkCommandBufferResetFlags commandBufferResetFlags{};
        vkResetCommandBuffer(commandBuffers[resetIndex], commandBufferResetFlags);
    }

    [[maybe_unused]] void recordCommandBuffer(const std::vector<int> &recordIndices) {
        for (int i : recordIndices) { this->recordCommandBuffer(i); }
    }

    void recordCommandBuffer(int recordIndex = 0) {
        VkCommandBufferBeginInfo commandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        if (vkBeginCommandBuffer(commandBuffers[recordIndex], &commandBufferBeginInfo) != VK_SUCCESS) { throw std::runtime_error("failed to begin recording command IEBuffer!"); }
    }

    ~IECommandBuffer() {
        destroy();
    }

private:
    std::vector<std::function<void()>> deletionQueue{};
    std::vector<std::function<void()>> commandBufferDeletionQueue{};
    vkb::QueueType queue{};
    IEGraphicsLink *linkedRenderEngine{};
};