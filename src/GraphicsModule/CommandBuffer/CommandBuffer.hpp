#pragma once

#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <vulkan/vulkan.h>

namespace IE::Graphics {
namespace detail {
class ImageVulkan;
}  // namespace detail

class CommandPool;
class Buffer;
class Pipeline;
class DescriptorSet;
class RenderEngine;
class RenderPass;
class Framebuffer;

enum Status {
    IE_COMMAND_BUFFER_STATE_NONE       = 0x0,
    IE_COMMAND_BUFFER_STATE_INITIAL    = 0x1,
    IE_COMMAND_BUFFER_STATE_RECORDING  = 0x2,
    IE_COMMAND_BUFFER_STATE_EXECUTABLE = 0x3,
    IE_COMMAND_BUFFER_STATE_PENDING    = 0x4,
    IE_COMMAND_BUFFER_STATE_INVALID    = 0x5
};

enum AllocationFlagBits {
    IE_COMMAND_BUFFER_ALLOCATE_SECONDARY = 0x1,
};

using AllocationFlags = uint64_t;

enum RecordFlagBits {
    IE_COMMAND_BUFFER_RECORD_ONE_TIME_SUBMIT = 0x1
};

class CommandBuffer {
    using RecordFlags = uint64_t;

public:
    VkCommandBuffer              m_commandBuffer{};
    std::shared_ptr<CommandPool> m_commandPool{};
    IE::Graphics::RenderEngine  *m_linkedRenderEngine{};
    Status                       m_status{};
    RecordFlags                  m_recordFlags{};
    AllocationFlags              m_allocationFlags{};

    CommandBuffer(std::shared_ptr<IE::Graphics::CommandPool> t_parentCommandPool);

    /**
     * @brief Allocate this command buffer as a primary command buffer.
     */
    void allocate(AllocationFlags t_flags = 0);

    /**
     * @brief Prepare this command buffer for recording.
     */
    void record(
      RecordFlags  t_flags       = 0,
      RenderPass  *t_renderPass  = nullptr,
      uint32_t     t_subpass     = 0,
      Framebuffer *t_framebuffer = nullptr
    );

    void free(std::lock_guard<std::mutex> &lock);

    void free();

    void reset(std::lock_guard<std::mutex> &lock);

    void reset();

    void execute(
      VkSemaphore input  = reinterpret_cast<VkSemaphore>(static_cast<void *>(nullptr)),
      VkSemaphore output = reinterpret_cast<VkSemaphore>(static_cast<void *>(nullptr)),
      VkFence     fence  = reinterpret_cast<VkFence>(static_cast<void *>(nullptr))
    );

    void finish(bool synchronize = true);

    void recordPipelineBarrier(
      VkPipelineStageFlags                srcStageMask,
      VkPipelineStageFlags                dstStageMask,
      VkDependencyFlags                   dependencyFlags,
      const std::vector<VkMemoryBarrier> &memoryBarriers
    );

    void recordCopyBufferToImage(
      const std::shared_ptr<Buffer>                            &buffer,
      const std::shared_ptr<IE::Graphics::detail::ImageVulkan> &image,
      std::vector<VkBufferImageCopy>                            regions
    );

    void recordBindVertexBuffers(
      uint32_t                             firstBinding,
      uint32_t                             bindingCount,
      std::vector<std::shared_ptr<Buffer>> buffers,
      VkDeviceSize                        *pOffsets
    );

    void recordBindVertexBuffers(
      uint32_t                                    firstBinding,
      uint32_t                                    bindingCount,
      const std::vector<std::shared_ptr<Buffer>> &buffers,
      VkDeviceSize                               *pOffsets,
      VkDeviceSize                               *pSizes,
      VkDeviceSize                               *pStrides
    );

    void recordBindPipeline(VkPipelineBindPoint pipelineBindPoint, const std::shared_ptr<Pipeline> &pipeline);

    void recordBindDescriptorSets(
      VkPipelineBindPoint                                pipelineBindPoint,
      const std::shared_ptr<Pipeline>                   &pipeline,
      uint32_t                                           firstSet,
      const std::vector<std::shared_ptr<DescriptorSet>> &descriptorSets,
      std::vector<uint32_t>                              dynamicOffsets
    );

    void recordNextSubpass();

    void recordDrawIndexed(
      uint32_t indexCount,
      uint32_t instanceCount,
      uint32_t firstIndex,
      int32_t  vertexOffset,
      uint32_t firstInstance
    );

    void recordBindIndexBuffer(const std::shared_ptr<Buffer> &buffer, uint32_t offset, VkIndexType indexType);

    void recordSetViewport(uint32_t firstViewPort, uint32_t viewPortCount, const VkViewport *pViewPorts);

    void recordSetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *pScissors);

    void recordEndRenderPass();

    void destroy();

    CommandBuffer(const CommandBuffer &source) = delete;

    CommandBuffer(CommandBuffer &&source) = delete;

    void recordBeginRenderPass(VkRenderPassBeginInfo *pRenderPassBegin, VkSubpassContents contents);

    void recordExecuteSecondaryCommandBuffers(std::vector<VkCommandBuffer> t_commandBuffers);
    void recordExecuteSecondaryCommandBuffers(std::vector<std::shared_ptr<CommandBuffer>> t_commandBuffers);
};
}  // namespace IE::Graphics
