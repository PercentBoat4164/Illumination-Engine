#pragma once

#include "VkBootstrap.h"

#include "RenderEngineLink.hpp"
#include "LogModule/Log.hpp"

class CommandPool {
public:
    struct CreateInfo {
    public:
        VkCommandPoolCreateFlagBits flags{static_cast<VkCommandPoolCreateFlagBits>(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)};    // Optional
        vkb::QueueType commandQueue{vkb::QueueType::graphics};                                                                                                                  // Optional
    };

    struct Created {
    public:
        bool commandPool{};
        bool commandBuffers{};
    };
    CreateInfo createdWith{};
    Created created{};
    VkCommandPool commandPool{};
    std::vector<VkCommandBuffer> commandBuffers{};
    RenderEngineLink *linkedRenderEngine{};

    void create(RenderEngineLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        VkCommandPoolCreateInfo commandPoolCreateInfo{.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .flags=createInfo->flags, .queueFamilyIndex=linkedRenderEngine->device.get_queue_index(createInfo->commandQueue).value()};
        if (vkCreateCommandPool(linkedRenderEngine->device.device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) { linkedRenderEngine->log->log("failed to create CommandPool!", log4cplus::DEBUG_LOG_LEVEL, "Graphics Module"); }
        created.commandPool = true;
    }

    void addCommandBuffers(unsigned int commandBufferCount) {
        /**@todo: Test this function to see if it can be called twice in a row safely.*/
        commandBuffers.resize(commandBufferCount + commandBuffers.size());
        VkCommandBufferAllocateInfo commandBufferAllocateInfo{.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool=commandPool, .level=VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount=static_cast<uint32_t>(commandBuffers.size())};
        if (vkAllocateCommandBuffers(linkedRenderEngine->device.device, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) { linkedRenderEngine->log->log("failed to allocate CommandBuffers!", log4cplus::DEBUG_LOG_LEVEL, "Graphics Module"); }
        created.commandBuffers = true;
    };

    void resetCommandBuffer(const std::vector<unsigned int> &resetIndices) {
        for (unsigned int i : resetIndices) { this->resetCommandBuffer(i); }
    }

    void resetCommandBuffer(unsigned int resetIndex = 0) {
        VkCommandBufferResetFlags commandBufferResetFlags{};
        vkResetCommandBuffer(commandBuffers[resetIndex], commandBufferResetFlags);
    }

    void recordCommandBuffer(const std::vector<unsigned int> &recordIndices) {
        for (unsigned int i : recordIndices) { this->recordCommandBuffer(i); }
    }

    void recordCommandBuffer(unsigned int recordIndex = 0) {
        VkCommandBufferBeginInfo commandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        if (vkBeginCommandBuffer(commandBuffers[recordIndex], &commandBufferBeginInfo) != VK_SUCCESS) { throw std::runtime_error("failed to begin recording command buffer!"); }
    }

    void freeCommandBuffer(const std::vector<unsigned int>& freeIndices) {
        std::vector<VkCommandBuffer> commandBuffersToFree{freeIndices.size()};
        for (unsigned int i = 0; i < freeIndices.size(); ++i) { commandBuffersToFree[i] = commandBuffers[freeIndices[i]]; }
        vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool, static_cast<uint32_t>(commandBuffersToFree.size()), commandBuffersToFree.data());
    }

    void freeCommandBuffer(unsigned int freeIndex) {
        vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool, 1, &commandBuffers[freeIndex]);
    }

    void destroy() const {
        if (created.commandBuffers) { vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool, commandBuffers.size(), commandBuffers.data()); }
        if (created.commandPool) { vkDestroyCommandPool(linkedRenderEngine->device.device, commandPool, nullptr); }
    }

    ~CommandPool() {
        destroy();
    }
};