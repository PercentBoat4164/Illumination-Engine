#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class RenderEngine;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "CommandBuffer.hpp"

// External dependencies
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

// System dependencies
#include <cstdint>
#include <vector>

namespace IE::Graphics {
class CommandPool {
public:
    VkCommandPool                               m_commandPool{};
    std::vector<std::shared_ptr<CommandBuffer>> m_commandBuffers{};
    IE::Graphics::RenderEngine                 *m_linkedRenderEngine{};
    VkQueue                                     m_queue{};
    std::shared_ptr<std::mutex>                 m_commandPoolMutex{};

    CommandPool() = default;

    CommandPool(const CommandPool &) = default;

    CommandPool(CommandPool &&) = default;

    CommandPool &operator=(const CommandPool &) = default;

    CommandPool &operator=(CommandPool &&) = default;

    void create(
      IE::Graphics::RenderEngine *t_engineLink,
      VkCommandPoolCreateFlags    t_flags,
      vkb::QueueType              t_commandQueue
    );

    ~CommandPool();

    bool tryLock();

    void unlock();
};
}  // namespace IE::Graphics