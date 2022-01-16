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

    void create(IEGraphicsLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        VkCommandPoolCreateInfo commandPoolCreateInfo{.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .flags=createInfo->flags, .queueFamilyIndex=linkedRenderEngine->device.get_queue_index(createInfo->commandQueue).value()};
        if (vkCreateCommandPool(linkedRenderEngine->device.device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Failed to create command pool!");
        }
        created.commandPool = true;
        addCommandBuffers(1);
    }

    void addCommandBuffers(uint32_t commandBufferCount) {
        commandBuffers.resize(commandBufferCount + commandBuffers.size());
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool=commandPool, .level=VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount=static_cast<uint32_t>(commandBuffers.size())};
        if (vkAllocateCommandBuffers(linkedRenderEngine->device.device, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Failed to allocate command buffers!");
        }
        created.commandBuffers = true;
    };

    void resetCommandBuffer(const std::vector<uint32_t> &resetIndices) {
        for (uint32_t i : resetIndices) { this->resetCommandBuffer(i); }
    }

    void resetCommandBuffer(uint32_t resetIndex=0) {
        VkCommandBufferResetFlags commandBufferResetFlags{};
        vkResetCommandBuffer(commandBuffers[resetIndex], commandBufferResetFlags);
    }

    void recordCommandBuffer(const std::vector<uint32_t> &recordIndices) {
        for (uint32_t i : recordIndices) { this->recordCommandBuffer(i); }
    }

    void recordCommandBuffer(uint32_t recordIndex = 0) {
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

    const VkCommandBuffer& operator[](uint32_t index) const {
        return commandBuffers[index];
    }

    VkCommandBuffer& operator[](uint32_t index) {
        return commandBuffers[index];
    }

    void destroy() const {
        if (created.commandBuffers) { vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool, commandBuffers.size(), commandBuffers.data()); }
        if (created.commandPool) { vkDestroyCommandPool(linkedRenderEngine->device.device, commandPool, nullptr); }
    }

    ~IECommandPool() {
        destroy();
    }
};