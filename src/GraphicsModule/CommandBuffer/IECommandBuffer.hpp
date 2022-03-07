#pragma once

class IECommandPool;

class IERenderEngine;

#include <vulkan/vulkan.h>
#include <vector>
#include <variant>
#include "IEBuffer.hpp"
#include "Image/IEImage.hpp"
#include "IEPipeline.hpp"


typedef enum IECommandBufferState {
    IE_COMMAND_BUFFER_STATE_NONE = 0x0,
    IE_COMMAND_BUFFER_STATE_INITIAL = 0x1,
    IE_COMMAND_BUFFER_STATE_RECORDING = 0x2,
    IE_COMMAND_BUFFER_STATE_EXECUTABLE = 0x3,
    IE_COMMAND_BUFFER_STATE_PENDING = 0x4,
    IE_COMMAND_BUFFER_STATE_INVALID = 0x5
} IECommandBufferState;

class IECommandBuffer {
public:
    VkCommandBuffer commandBuffer{};
    IECommandPool *commandPool;
    std::vector<std::variant<IEBuffer, IEImage, IEPipeline>> dependencies;
    IERenderEngine *linkedRenderEngine;
    IECommandBufferState state;

    IECommandBuffer(IERenderEngine *linkedRenderEngine, IECommandPool *commandPool) {
        this->linkedRenderEngine = linkedRenderEngine;
        this->commandPool = commandPool;
        state = IE_COMMAND_BUFFER_STATE_NONE;
    }

    /**
     * @brief Allocate this command buffer as a primary command buffer.
     */
    void allocate();

    /**
     * @brief Prepare this command buffer for recording.
     */
    void record();

    IEBuffer *addDependency(const IEBuffer &dependency);

    IEImage *addDependency(const IEImage &dependency);

    IEPipeline *addDependency(const IEPipeline &dependency);

    void free() {
        for (std::variant<IEBuffer, IEImage, IEPipeline> dependency: dependencies) {
            if (IEBuffer *buffer = std::get_if<IEBuffer>(&dependency)) {
                buffer->destroy();
            }
            if (IEImage *image = std::get_if<IEImage>(&dependency)) {
                image->destroy();
            }
            if (IEPipeline *pipeline = std::get_if<IEPipeline>(&dependency)) {
                pipeline->destroy();
            }
        }
    }

    void reset() {

    }

    // Convenience functions that will prevent the user from writing a bit of boilerplate code for dependencies with vkCmd* commands
    void recordBindPipeline(VkPipelineBindPoint bindPoint, const IEPipeline &pipeline) {
        vkCmdBindPipeline(commandBuffer, bindPoint, addDependency(pipeline)->pipeline);
    }

    void recordCopyBuffer(const IEBuffer &srcBuffer, const IEBuffer &dstBuffer, uint32_t regionCount, const VkBufferCopy *pRegions) {
        vkCmdCopyBuffer(commandBuffer, addDependency(srcBuffer)->buffer, addDependency(dstBuffer)->buffer, regionCount, pRegions);
    }
};