/* Include this file's header. */
#include "CommandPool.hpp"

/* Include dependencies within this module. */
#include "RenderEngine.hpp"

/* Include dependencies from Core. */
#include "CommandBuffer.hpp"
#include "Core/LogModule/Logger.hpp"

void IE::Graphics::CommandPool::create(
  IE::Graphics::RenderEngine *t_engineLink,
  VkCommandPoolCreateFlags    t_flags,
  vkb::QueueType              t_commandQueue
) {
    m_linkedRenderEngine = t_engineLink;
    m_queue              = m_linkedRenderEngine->m_device.get_queue(t_commandQueue).value();
    VkCommandPoolCreateInfo commandPoolCreateInfo{
      .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags            = t_flags,
      .queueFamilyIndex = m_linkedRenderEngine->m_device.get_queue_index(t_commandQueue).value()};
    VkResult result{
      vkCreateCommandPool(m_linkedRenderEngine->m_device.device, &commandPoolCreateInfo, nullptr, &m_commandPool)};
    if (result != VK_SUCCESS)
        m_linkedRenderEngine->getLogger().log(
          "Failed to create command pool!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG
        );
    else m_linkedRenderEngine->getLogger().log("Created CommandPool");
}

void IE::Graphics::CommandPool::destroy() {
    m_commandBuffers.clear();
    if (m_commandPool) vkDestroyCommandPool(m_linkedRenderEngine->m_device.device, m_commandPool, nullptr);
}

IE::Graphics::CommandPool::~CommandPool() {
    destroy();
}