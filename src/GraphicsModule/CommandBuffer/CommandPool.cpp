/* Include this file's header. */
#include "CommandPool.hpp"

/* Include dependencies within this module. */
#include "RenderEngine.hpp"

/* Include dependencies from Core. */
#include "CommandBuffer.hpp"
#include "Core/LogModule/IELogger.hpp"

void IE::Graphics::CommandPool::create(
  IE::Graphics::RenderEngine            *engineLink,
  IE::Graphics::CommandPool::CreateInfo *createInfo
) {
    m_linkedRenderEngine = engineLink;
    createdWith          = *createInfo;
    m_queue              = m_linkedRenderEngine->m_device.get_queue(createdWith.commandQueue).value();
    VkCommandPoolCreateInfo commandPoolCreateInfo{
      .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags            = static_cast<VkCommandPoolCreateFlags>(createInfo->flags),
      .queueFamilyIndex = m_linkedRenderEngine->m_device.get_queue_index(createInfo->commandQueue).value()};
    if (vkCreateCommandPool(m_linkedRenderEngine->m_device.device, &commandPoolCreateInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
        m_linkedRenderEngine->getLogger().log(
          "Failed to create command pool!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG
        );
    }
}

void IE::Graphics::CommandPool::prepareCommandBuffers(uint32_t commandBufferCount) {
    m_commandBuffers.reserve(commandBufferCount);
    for (uint32_t startIndex = m_commandBuffers.size(); startIndex < commandBufferCount; ++startIndex)
        m_commandBuffers.push_back(std::make_shared<IE::Graphics::CommandBuffer>(m_linkedRenderEngine, this));
}

std::shared_ptr<IE::Graphics::CommandBuffer> IE::Graphics::CommandPool::index(uint32_t index) {
    if (index <= m_commandBuffers.size()) {
        if (index == m_commandBuffers.size())
            m_commandBuffers.push_back(std::make_shared<IE::Graphics::CommandBuffer>(m_linkedRenderEngine, this));
        return m_commandBuffers[index];
    }
    m_linkedRenderEngine->getLogger().log(
      "Attempt to access a command buffer that does not exist!",
      IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
    );
    return m_commandBuffers[m_commandBuffers.size() - 1];
}

IE::Graphics::CommandPool::~CommandPool() {
    for (const std::shared_ptr<IE::Graphics::CommandBuffer> &commandBuffer : m_commandBuffers)
        commandBuffer->destroy();
    m_commandBuffers.clear();
}
