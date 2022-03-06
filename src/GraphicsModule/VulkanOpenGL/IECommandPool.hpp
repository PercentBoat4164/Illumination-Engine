#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// External dependencies
#include <VkBootstrap.h>

#include <vulkan/vulkan.h>

// System dependencies
#include <vector>
#include <cstdint>

class IECommandPool {
public:
    struct CreateInfo {
    public:
        VkCommandPoolCreateFlagBits flags{static_cast<VkCommandPoolCreateFlagBits>(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)};  // Optional
        vkb::QueueType commandQueue;  // Required
    } createdWith{};

    struct Created {
    public:
        bool commandPool{};
        bool commandBuffers{};
    } created{};

    VkCommandPool commandPool{};
    std::vector<VkCommandBuffer> commandBuffers{};
    IERenderEngine* linkedRenderEngine{};
    VkQueue queue;

    void create(IERenderEngine* engineLink, CreateInfo* createInfo);

    void addCommandBuffers(uint32_t commandBufferCount);;

    void resetCommandBuffer(const std::vector<uint32_t>& resetIndices);

    void resetCommandBuffer(uint32_t resetIndex=0);

    void recordCommandBuffer(const std::vector<uint32_t>& recordIndices);

    void recordCommandBuffer(uint32_t recordIndex=0);

    void freeCommandBuffer(const std::vector<uint32_t>& freeIndices);

    void freeCommandBuffer(uint32_t freeIndex);

    /**@todo Multi-thread this to prevent waiting for the GPU to finish all its work before starting to assign it more work.*/
    void executeCommandBuffer(uint32_t submitIndex);

    const VkCommandBuffer& operator[](uint32_t index) const;

    VkCommandBuffer& operator[](uint32_t index);

    void destroy();

    ~IECommandPool();
};