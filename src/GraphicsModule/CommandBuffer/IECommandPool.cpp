/* Include this file's header. */
#include "IECommandPool.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"

/* Include dependencies from Core. */
#include "Core/LogModule/IELogger.hpp"


void IECommandPool::create(IERenderEngine *engineLink, IECommandPool::CreateInfo *createInfo) {
    linkedRenderEngine = engineLink;
    createdWith = *createInfo;
    queue = linkedRenderEngine->device.get_queue(createdWith.commandQueue).value();
    VkCommandPoolCreateInfo commandPoolCreateInfo{
        .sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags=createInfo->flags,
        .queueFamilyIndex=linkedRenderEngine->device.get_queue_index(createInfo->commandQueue).value()
    };
    if (vkCreateCommandPool(linkedRenderEngine->device.device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Failed to create command pool!");
    }

}

const IECommandBuffer &IECommandPool::operator[](uint32_t index) const {
    if (index <= commandBuffers.size()) {
        return commandBuffers[index];
    }
    else {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to access a command buffer that does not exist!");
        return commandBuffers[commandBuffers.size() - 1];
    }
}

IECommandBuffer &IECommandPool::operator[](uint32_t index) {
    if (index <= commandBuffers.size()) {
        if (index == commandBuffers.size()) {
            commandBuffers.emplace_back(IECommandBuffer(linkedRenderEngine, this));
        }
        return commandBuffers[index];
    }
    else {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to access a command buffer that does not exist!");
        return commandBuffers[commandBuffers.size() - 1];
    }
}

void IECommandPool::destroy() {
    for (IECommandBuffer commandBuffer : commandBuffers) {
        commandBuffer.free();
    }
    if (commandPool) {
        vkDestroyCommandPool(linkedRenderEngine->device.device, commandPool, nullptr);
    }
}

IECommandPool::~IECommandPool() {
    destroy();
}

void IECommandPool::clearUnused() {
    commandBuffers.erase(std::remove_if(commandBuffers.begin(), commandBuffers.end(), commandBufferIsUnused), commandBuffers.end());
}

bool IECommandPool::commandBufferIsUnused(const IECommandBuffer &commandBuffer) {
    return commandBuffer.state == IE_COMMAND_BUFFER_STATE_INVALID || commandBuffer.state == IE_COMMAND_BUFFER_STATE_INITIAL;
}
