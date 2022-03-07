/* Include this file's header. */
#include "IECommandPool.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"

/* Include dependencies from Core. */
#include "Core/LogModule/IELogger.hpp"


void IECommandPool::create(IERenderEngine *engineLink, IECommandPool::CreateInfo *createInfo) {
    linkedRenderEngine = engineLink;
    queue = linkedRenderEngine->device.get_queue(createdWith.commandQueue).value();
    VkCommandPoolCreateInfo commandPoolCreateInfo{.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .flags=createInfo->flags, .queueFamilyIndex=linkedRenderEngine->device.get_queue_index(createInfo->commandQueue).value()};
    if (vkCreateCommandPool(linkedRenderEngine->device.device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Failed to create command pool!");
    }
    created.commandPool = true;
    addCommandBuffers(1);
}

void IECommandPool::addCommandBuffers(uint32_t commandBufferCount) {
    // Create vector of new command buffers
    std::vector<VkCommandBuffer> newCommandBuffers{commandBufferCount};

    // Allocate the new command buffers.
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool=commandPool, .level=VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount=static_cast<uint32_t>(commandBufferCount)};
    if (vkAllocateCommandBuffers(linkedRenderEngine->device.device, &commandBufferAllocateInfo, newCommandBuffers.data()) != VK_SUCCESS) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Failed to allocate command buffers!");
    }

    // Add new command buffers to the end of the old command buffers.
    commandBuffers.insert(commandBuffers.end(), newCommandBuffers.begin(), newCommandBuffers.end());

    // Note that command buffers are now created and hence need to be destroyed.
    created.commandBuffers = true;
}

void IECommandPool::resetCommandBuffer(const std::vector <uint32_t> &resetIndices) {
    for (uint32_t i : resetIndices) { this->resetCommandBuffer(i); }
}

void IECommandPool::resetCommandBuffer(uint32_t resetIndex) {
    VkCommandBufferResetFlags commandBufferResetFlags{};
    vkResetCommandBuffer(commandBuffers[resetIndex], commandBufferResetFlags);
}

void IECommandPool::recordCommandBuffer(const std::vector <uint32_t> &recordIndices) {
    for (uint32_t i : recordIndices) { this->recordCommandBuffer(i); }
}

void IECommandPool::recordCommandBuffer(uint32_t recordIndex) {
    VkCommandBufferBeginInfo commandBufferBeginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    if (vkBeginCommandBuffer(commandBuffers[recordIndex], &commandBufferBeginInfo) != VK_SUCCESS) {  }
}

void IECommandPool::freeCommandBuffer(const std::vector <uint32_t> &freeIndices) {
    std::vector<VkCommandBuffer> commandBuffersToFree{freeIndices.size()};
    for (uint32_t i = 0; i < freeIndices.size(); ++i) { commandBuffersToFree[i] = commandBuffers[freeIndices[i]]; }
    vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool, static_cast<uint32_t>(commandBuffersToFree.size()), commandBuffersToFree.data());
}

void IECommandPool::freeCommandBuffer(uint32_t freeIndex) {
    vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool, 1, &commandBuffers[freeIndex]);
}

void IECommandPool::executeCommandBuffer(uint32_t submitIndex) {
    vkEndCommandBuffer(commandBuffers[submitIndex]);
    VkSubmitInfo submitInfo{.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount=1, .pCommandBuffers=&commandBuffers[submitIndex]};
    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);
}

const VkCommandBuffer &IECommandPool::operator[](uint32_t index) const {
    return commandBuffers[index];
}

VkCommandBuffer &IECommandPool::operator[](uint32_t index) {
    return commandBuffers[index];
}

void IECommandPool::destroy() {
    if (!commandBuffers.empty()) {
        vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool, commandBuffers.size(), commandBuffers.data());
        commandBuffers.clear();
    }
    if (commandPool) {
        vkDestroyCommandPool(linkedRenderEngine->device.device, commandPool, nullptr);
    }
}

IECommandPool::~IECommandPool() {
    destroy();
}
