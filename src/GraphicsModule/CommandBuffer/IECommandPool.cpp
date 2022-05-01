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
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Failed to create command pool!");
    }
    deletionQueue.emplace_back([&] {
        vkDestroyCommandPool(linkedRenderEngine->device.device, commandPool, nullptr);
    });
}

void IECommandPool::prepareCommandBuffers(uint32_t commandBufferCount) {
    commandBuffers.reserve(commandBufferCount);
}

IECommandBuffer &IECommandPool::operator[](uint32_t index) {
    if (index <= commandBuffers.size()) {
        if (index == commandBuffers.size()) {
            commandBuffers.emplace_back(linkedRenderEngine, this);
        }
        return commandBuffers[index];
    }
    linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to access a command buffer that does not exist!");
    return commandBuffers[commandBuffers.size() - 1];
}

void IECommandPool::destroy() {
    commandBuffers.clear();
    for (const std::function<void()>& function : deletionQueue) {
        function();
    }
    deletionQueue.clear();
}

IECommandPool::~IECommandPool() {
    destroy();
}
