#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

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
    struct CreateInfo {
    public:
        VkCommandPoolCreateFlagBits flags{static_cast<VkCommandPoolCreateFlagBits>(
          VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        )};
        vkb::QueueType              commandQueue;
    } __attribute__((aligned(8))) createdWith{};

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

    void create(IE::Graphics::RenderEngine *engineLink, CreateInfo *createInfo);

    ~CommandPool();

    void prepareCommandBuffers(uint32_t commandBufferCount);

    std::shared_ptr<CommandBuffer> index(uint32_t index);
};
}  // namespace IE::Graphics