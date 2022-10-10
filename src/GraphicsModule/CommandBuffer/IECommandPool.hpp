#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IECommandBuffer.hpp"

// External dependencies
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

// System dependencies
#include <cstdint>
#include <vector>

class IECommandPool : public std::enable_shared_from_this<IECommandPool> {
public:
    struct CreateInfo {
    public:
        VkCommandPoolCreateFlagBits flags{static_cast<VkCommandPoolCreateFlagBits>(
          VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
        )};
        vkb::QueueType              commandQueue;
    } createdWith{};

    VkCommandPool                                 commandPool{};
    std::vector<std::shared_ptr<IECommandBuffer>> commandBuffers{};
    IERenderEngine                               *linkedRenderEngine{};
    VkQueue                                       queue;
    std::mutex                                    commandPoolMutex{};
    std::vector<std::function<void()>>            deletionQueue{};

    void create(IERenderEngine *engineLink, CreateInfo *createInfo);

    void destroy();

    ~IECommandPool();

    void prepareCommandBuffers(uint32_t commandBufferCount);

    std::shared_ptr<IECommandBuffer> index(uint32_t index);
};