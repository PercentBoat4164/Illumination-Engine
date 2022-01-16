#pragma once

#include "IEGraphicsLink.hpp"
#include "Core/LogModule/IELogger.hpp"

#include "VkBootstrap.h"

class IECommandPool {
public:
    struct CreateInfo {
    public:
        VkCommandPoolCreateFlagBits flags{static_cast<VkCommandPoolCreateFlagBits>(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)};  // Optional
        vkb::QueueType commandQueue;  // Required
    } createdWith{};

    struct Created {
    public:
        bool commandPool{};
        bool commandBuffers{};
    } created{};

    VkCommandPool commandPool{};
    std::vector<VkCommandBuffer> commandBuffers{};
    IEGraphicsLink* linkedRenderEngine{};
    VkQueue queue;

    void create(IEGraphicsLink* engineLink, CreateInfo* createInfo) {
        linkedRenderEngine = engineLink;
        queue = linkedRenderEngine->device.get_queue(createdWith.commandQueue).value();
        VkCommandPoolCreateInfo commandPoolCreateInfo{.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .flags=createInfo->flags, .queueFamilyIndex=linkedRenderEngine->device.get_queue_index(createInfo->commandQueue).value()};
        if (vkCreateCommandPool(linkedRenderEngine->device.device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Failed to create command pool!");
        }
        created.commandPool = true;
        addCommandBuffers(1);
    }

    void addCommandBuffers(uint32_t commandBufferCount) {
        // Create vector of new command buffers
        std::vector<VkCommandBuffer> newCommandBuffers{commandBufferCount};

        // Allocate the new command buffers.
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool=commandPool, .level=VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount=static_cast<uint32_t>(commandBufferCount)};
        if (vkAllocateCommandBuffers(linkedRenderEngine->device.device, &commandBufferAllocateInfo, newCommandBuffers.data()) != VK_SUCCESS) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Failed to allocate command buffers!");
        }

        // Add new command buffers to the end of the old command buffers.
        commandBuffers.insert(commandBuffers.cend(), newCommandBuffers.cbegin(), newCommandBuffers.cend());

        // Note that command buffers are now created and hence need to be destroyed.
        created.commandBuffers = true;
    };

    void resetCommandBuffer(const std::vector<uint32_t>& resetIndices) {
        for (uint32_t i : resetIndices) { this->resetCommandBuffer(i); }
    }

    void resetCommandBuffer(uint32_t resetIndex=0) {
        VkCommandBufferResetFlags commandBufferResetFlags{};
        vkResetCommandBuffer(commandBuffers[resetIndex], commandBufferResetFlags);
    }

    void recordCommandBuffer(const std::vector<uint32_t>& recordIndices) {
        for (uint32_t i : recordIndices) { this->recordCommandBuffer(i); }
    }

    void recordCommandBuffer(uint32_t recordIndex=0) {
        VkCommandBufferBeginInfo commandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        if (vkBeginCommandBuffer(commandBuffers[recordIndex], &commandBufferBeginInfo) != VK_SUCCESS) {  }
    }

    void freeCommandBuffer(const std::vector<uint32_t>& freeIndices) {
        std::vector<VkCommandBuffer> commandBuffersToFree{freeIndices.size()};
        for (uint32_t i = 0; i < freeIndices.size(); ++i) { commandBuffersToFree[i] = commandBuffers[freeIndices[i]]; }
        vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool, static_cast<uint32_t>(commandBuffersToFree.size()), commandBuffersToFree.data());
    }

    void freeCommandBuffer(uint32_t freeIndex) {
        vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool, 1, &commandBuffers[freeIndex]);
    }

    /**@todo Multi-thread this to prevent waiting for the GPU to finish all its work before starting to assign it more work.*/
    void executeCommandBuffer(uint32_t submitIndex) {
        vkEndCommandBuffer(commandBuffers[submitIndex]);
        VkSubmitInfo submitInfo{.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount=1, .pCommandBuffers=&commandBuffers[submitIndex]};
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(queue);
    }

    const VkCommandBuffer& operator[](uint32_t index) const {
        return commandBuffers[index];
    }

    VkCommandBuffer& operator[](uint32_t index) {
        return commandBuffers[index];
    }

    void destroy() const {
        if (created.commandBuffers) {
            vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool, commandBuffers.size(), commandBuffers.data());
        }
        if (created.commandPool) {
            vkDestroyCommandPool(linkedRenderEngine->device.device, commandPool, nullptr);
        }
    }

    ~IECommandPool() {
        destroy();
    }
};