/* Include this file's header. */
#include "IECommandPool.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"

/* Include dependencies from Core. */
#include "Core/LogModule/IELogger.hpp"

void IECommandPool::create(IERenderEngine *engineLink, IECommandPool::CreateInfo *createInfo) {
    linkedRenderEngine = engineLink;
    createdWith        = *createInfo;
    queue              = linkedRenderEngine->device.get_queue(createdWith.commandQueue).value();
    VkCommandPoolCreateInfo commandPoolCreateInfo{
      .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags            = static_cast<VkCommandPoolCreateFlags>(createInfo->flags),
      .queueFamilyIndex = linkedRenderEngine->device.get_queue_index(createInfo->commandQueue).value()};
    if (vkCreateCommandPool(linkedRenderEngine->device.device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) {
        linkedRenderEngine->settings->logger.log(
          "Failed to create command pool!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG
        );
    }
    deletionQueue.emplace_back([&] {
        vkDestroyCommandPool(linkedRenderEngine->device.device, commandPool, nullptr);
    });
}

void IECommandPool::prepareCommandBuffers(uint32_t commandBufferCount) {
    uint32_t startIndex = commandBuffers.size();
    commandBuffers.reserve(commandBufferCount);
    for (; startIndex < commandBufferCount; ++startIndex)
        commandBuffers.push_back(std::make_shared<IECommandBuffer>(linkedRenderEngine, shared_from_this()));
}

std::shared_ptr<IECommandBuffer> IECommandPool::index(uint32_t index) {
    if (index <= commandBuffers.size()) {
        if (index == commandBuffers.size())
            commandBuffers.push_back(std::make_shared<IECommandBuffer>(linkedRenderEngine, shared_from_this()));
        return commandBuffers[index];
    }
    linkedRenderEngine->settings->logger.log(
      "Attempt to access a command buffer that does not exist!",
      IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
    );
    return commandBuffers[commandBuffers.size() - 1];
}

void IECommandPool::destroy() {
    for (const std::shared_ptr<IECommandBuffer> &commandBuffer : commandBuffers) commandBuffer->destroy();
    commandBuffers.clear();
    for (const std::function<void()> &function : deletionQueue) function();
    deletionQueue.clear();
}

IECommandPool::~IECommandPool() {
    destroy();
}
