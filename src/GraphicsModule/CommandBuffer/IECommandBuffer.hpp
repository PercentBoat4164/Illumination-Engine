#pragma once

class IECommandPool;

class IERenderEngine;

#include <vulkan/vulkan.h>
#include <vector>
#include <variant>
#include "Buffer/IEBuffer.hpp"
#include "Image/IEImage.hpp"
#include "IEDependent.hpp"
#include "IEPipeline.hpp"


typedef enum IECommandBufferState {
    IE_COMMAND_BUFFER_STATE_NONE = 0x0,
    IE_COMMAND_BUFFER_STATE_INITIAL = 0x1,
    IE_COMMAND_BUFFER_STATE_RECORDING = 0x2,
    IE_COMMAND_BUFFER_STATE_EXECUTABLE = 0x3,
    IE_COMMAND_BUFFER_STATE_PENDING = 0x4,
    IE_COMMAND_BUFFER_STATE_INVALID = 0x5
} IECommandBufferState;


class IECommandBuffer : public IEDependent {
public:
    VkCommandBuffer commandBuffer{};
    IECommandPool *commandPool{};
    IERenderEngine *linkedRenderEngine{};
    IECommandBufferState state{};
    bool oneTimeSubmission{false};

    IECommandBuffer(IERenderEngine *linkedRenderEngine, IECommandPool *commandPool);

    void wait() override;

    /**
     * @brief Allocate this command buffer as a primary command buffer.
     */
    void allocate();

    /**
     * @brief Prepare this command buffer for recording.
     */
    void record(bool oneTimeSubmit=false);

    void free();

    void reset();

    void execute(VkSemaphore input=nullptr, VkSemaphore output=nullptr, VkFence fence=nullptr);

    void finish();

    void recordPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const std::vector<VkMemoryBarrier> &memoryBarriers, const std::vector<IEBufferMemoryBarrier> &bufferMemoryBarriers, const std::vector<IEImageMemoryBarrier> &imageMemoryBarriers);

    void recordPipelineBarrier(const IEDependencyInfo *dependencyInfo);

    void recordCopyBufferToImage(IEBuffer *buffer, IEImage *image, std::vector<VkBufferImageCopy> regions);

    void recordCopyBufferToImage(IECopyBufferToImageInfo *copyInfo);

    void recordBindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const std::vector<IEBuffer *>& buffers, VkDeviceSize *pOffsets);

    void recordBindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const std::vector<IEBuffer *> &buffers, VkDeviceSize *pOffsets, VkDeviceSize *pSizes, VkDeviceSize *pStrides);

    void recordBindPipeline(VkPipelineBindPoint pipelineBindPoint, IEPipeline *pipeline);

    void recordBindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, IEPipeline *pipeline, uint32_t firstSet, const std::vector<IEDescriptorSet *> &descriptorSets, std::vector<uint32_t> dynamicOffsets);

    void recordDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);

    void recordBindIndexBuffer(IEBuffer *buffer, uint32_t offset, VkIndexType indexType);

    void recordBeginRenderPass(IERenderPassBeginInfo *pRenderPassBegin, VkSubpassContents contents);

    void recordSetViewport(uint32_t firstViewPort, uint32_t viewPortCount, const VkViewport *pViewPorts);

    void recordSetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *pScissors);

    void recordEndRenderPass();

    void destroy() final;

    ~IECommandBuffer() override;
};