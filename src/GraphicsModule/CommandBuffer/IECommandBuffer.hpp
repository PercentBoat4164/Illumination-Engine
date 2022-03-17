#pragma once

class IECommandPool;

class IERenderEngine;

#include <vulkan/vulkan.h>
#include <vector>
#include <variant>
#include "IEBuffer.hpp"
#include "Image/IEImage.hpp"
#include "IEDependent.hpp"


typedef enum IECommandBufferState {
    IE_COMMAND_BUFFER_STATE_NONE = 0x0,
    IE_COMMAND_BUFFER_STATE_INITIAL = 0x1,
    IE_COMMAND_BUFFER_STATE_RECORDING = 0x2,
    IE_COMMAND_BUFFER_STATE_EXECUTABLE = 0x3,
    IE_COMMAND_BUFFER_STATE_PENDING = 0x4,
    IE_COMMAND_BUFFER_STATE_INVALID = 0x5
} IECommandBufferState;

class IECommandBuffer : public IEDependent{
public:
    VkCommandBuffer commandBuffer{};
    IECommandPool *commandPool;
    std::vector<void*> dependencies;
    IERenderEngine *linkedRenderEngine;
    IECommandBufferState state;

    IECommandBuffer(IERenderEngine *linkedRenderEngine, IECommandPool *commandPool);

    /**
     * @brief Allocate this command buffer as a primary command buffer.
     */
    void allocate();

    /**
     * @brief Prepare this command buffer for recording.
     */
    void record();

    void free();

    void reset();

    void execute();

    void finish();

    void recordPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const std::vector<VkMemoryBarrier> &memoryBarriers, const std::vector<IEBufferMemoryBarrier *> &bufferMemoryBarriers, const std::vector<IEImageMemoryBarrier *> &imageMemoryBarriers);

    void recordPipelineBarrier(const IEDependencyInfo *dependencyInfo) const;
};