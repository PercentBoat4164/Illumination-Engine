#include "CommandBuffer.hpp"

#include "Buffer/Buffer.hpp"
#include "CommandPool.hpp"
#include "Core/LogModule/Logger.hpp"
#include "Image/ImageVulkan.hpp"
#include "RenderEngine.hpp"
#include "RenderPass/Framebuffer.hpp"
#include "RenderPass/RenderPass.hpp"

// #include "Shader/DescriptorSet.hpp"
// #include "Shader/IEPipeline.hpp"

void IE::Graphics::CommandBuffer::allocate(AllocationFlags t_flags) {
    // Prepare
    m_allocationFlags = t_flags;
    VkCommandBufferAllocateInfo allocateInfo{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext              = nullptr,
      .commandPool        = m_commandPool->m_commandPool,
      .level              = (m_allocationFlags & IE_COMMAND_BUFFER_ALLOCATE_SECONDARY) != 0U ?
                     VK_COMMAND_BUFFER_LEVEL_SECONDARY :
                     VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};

    // Lock command pool
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);

    // Handle any needed status changes
    if (m_status == IE_COMMAND_BUFFER_STATE_INITIAL) {
        m_linkedRenderEngine->getLogger().log(
          "Attempt to allocate command buffers that are already allocated.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
        return;
    }

    // Allocate the command buffer
    VkResult result =
      vkAllocateCommandBuffers(m_linkedRenderEngine->m_device.device, &allocateInfo, &m_commandBuffer);
    if (result != VK_SUCCESS) {  // handle any potential errors
        m_linkedRenderEngine->getLogger().log(
          "Failure to properly allocate command buffers! Error: " +
            IE::Graphics::RenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    } else m_status = IE_COMMAND_BUFFER_STATE_INITIAL;
}

void IE::Graphics::CommandBuffer::record(
  RecordFlags  t_flags,
  RenderPass  *t_renderPass,
  uint32_t     t_subpass
) {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);

    VkCommandBufferInheritanceInfo inheritanceInfo{};

    if (m_allocationFlags & IE_COMMAND_BUFFER_ALLOCATE_SECONDARY) {
        inheritanceInfo = {
          .sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
          .pNext                = nullptr,
          .renderPass           = t_renderPass->m_renderPass,
          .subpass              = t_subpass,
          .framebuffer          = t_renderPass->m_framebuffer.m_framebuffer,
          .occlusionQueryEnable = VK_FALSE,
          .queryFlags           = 0x0,
          .pipelineStatistics   = 0x0};
    }

    m_recordFlags = t_flags;
    VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = static_cast<VkCommandBufferUsageFlags>(
        (m_recordFlags & IE_COMMAND_BUFFER_RECORD_ONE_TIME_SUBMIT) != 0U ?
          VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT :
          0
      ),
      .pInheritanceInfo = &inheritanceInfo,
    };

    // Handle any needed status changes
    if (m_status == IE_COMMAND_BUFFER_STATE_RECORDING) {
        m_linkedRenderEngine->getLogger().log(
          "Attempted to record a command buffer that was already in the recording state.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
        return;
    }
    if (m_status == IE_COMMAND_BUFFER_STATE_NONE)
        m_linkedRenderEngine->getLogger().log(
          "Attempted to record an unallocated command buffer!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    if (m_status >= IE_COMMAND_BUFFER_STATE_EXECUTABLE)
        m_linkedRenderEngine->getLogger().log(
          "Attempted to record to a command buffer that has already finished its execution phase. Call 'reset' on "
          "this command buffer before calling 'record'.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );

    // Begin recording
    VkResult result = vkBeginCommandBuffer(m_commandBuffer, &beginInfo);

    if (result != VK_SUCCESS)
        m_linkedRenderEngine->getLogger().log(
          "Failure to properly begin command buffer recording! Error: " +
            IE::Graphics::RenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    else m_status = IE_COMMAND_BUFFER_STATE_RECORDING;
}

void IE::Graphics::CommandBuffer::free(std::lock_guard<std::mutex> &lock) {
    if (m_status > IE_COMMAND_BUFFER_STATE_NONE) {
        vkFreeCommandBuffers(
          m_linkedRenderEngine->m_device.device,
          m_commandPool->m_commandPool,
          1,
          &m_commandBuffer
        );
        m_status = IE_COMMAND_BUFFER_STATE_NONE;
    }
}

void IE::Graphics::CommandBuffer::free() {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    free(lock);
}

IE::Graphics::CommandBuffer::CommandBuffer(IE::Graphics::CommandPool *t_parentCommandPool) :
        m_commandPool(t_parentCommandPool),
        m_linkedRenderEngine(t_parentCommandPool->m_linkedRenderEngine),
        m_status(IE_COMMAND_BUFFER_STATE_NONE) {
}

void IE::Graphics::CommandBuffer::reset(std::lock_guard<std::mutex> &lock) {
    // Handle any needed status changes
    if (m_status == IE_COMMAND_BUFFER_STATE_INVALID) {
        m_linkedRenderEngine->getLogger().log(
          "Attempt to reset a command buffer that is invalid!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }

    // Reset
    vkResetCommandBuffer(m_commandBuffer, 0);

    // Update new status
    m_status = IE_COMMAND_BUFFER_STATE_INITIAL;
}

void IE::Graphics::CommandBuffer::reset() {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    reset(lock);
}

void IE::Graphics::CommandBuffer::finish(bool synchronize) {
    if (synchronize)  // If synchronizing, lock this command pool
        m_commandPool->m_commandPoolMutex->lock();

    // Handle any needed status changes
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        m_linkedRenderEngine->getLogger().log(
          "Attempt to finish a command buffer that was not recording.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }

    // End
    vkEndCommandBuffer(m_commandBuffer);

    // Update status
    m_status = IE_COMMAND_BUFFER_STATE_EXECUTABLE;

    if (synchronize)  // If synchronizing, unlock this command pool
        m_commandPool->m_commandPoolMutex->unlock();
}

void IE::Graphics::CommandBuffer::execute(VkSemaphore input, VkSemaphore output, VkFence fence) {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status == IE_COMMAND_BUFFER_STATE_RECORDING) {
        finish(false);
    } else if (m_status != IE_COMMAND_BUFFER_STATE_EXECUTABLE) {
        m_linkedRenderEngine->getLogger().log(
          "Attempt to record a command buffer that is not recording or executable!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }

    VkPipelineStageFlags dstStageMask{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo{
      .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount   = static_cast<uint32_t>(input != VK_NULL_HANDLE),
      .pWaitSemaphores      = &input,
      .pWaitDstStageMask    = &dstStageMask,
      .commandBufferCount   = 1,
      .pCommandBuffers      = &m_commandBuffer,
      .signalSemaphoreCount = static_cast<uint32_t>(output != VK_NULL_HANDLE),
      .pSignalSemaphores    = &output,
    };

    // Set up fence if not already setup.
    bool fenceWasNullptr = fence == (VkFence) nullptr;
    if (fence == (VkFence) nullptr) {
        VkFenceCreateInfo fenceCreateInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vkCreateFence(m_linkedRenderEngine->m_device.device, &fenceCreateInfo, nullptr, &fence);
    }
    vkResetFences(m_linkedRenderEngine->m_device.device, 1, &fence);

    m_status = IE_COMMAND_BUFFER_STATE_PENDING;

    // Submit
    VkResult result = vkQueueSubmit(m_commandPool->m_queue, 1, &submitInfo, fence);


    // Wait for GPU to finish then unlock
    vkWaitForFences(m_linkedRenderEngine->m_device.device, 1, &fence, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS) {
        m_linkedRenderEngine->getLogger().log(
          "Failed to prepareAndSubmit command buffer! Error: " + RenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }
    m_status = (m_recordFlags & IE_COMMAND_BUFFER_RECORD_ONE_TIME_SUBMIT) != 0U ?
      IE_COMMAND_BUFFER_STATE_INVALID :
      IE_COMMAND_BUFFER_STATE_EXECUTABLE;

    if (fenceWasNullptr)  // destroy the fence if it was created in this function
        vkDestroyFence(m_linkedRenderEngine->m_device.device, fence, nullptr);

    // All tasks are done. Command buffer can be unlocked.
}

void IE::Graphics::CommandBuffer::recordCopyBufferToImage(
  const std::shared_ptr<Buffer>                            &buffer,
  const std::shared_ptr<IE::Graphics::detail::ImageVulkan> &image,
  std::vector<VkBufferImageCopy>                            regions

) {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdCopyBufferToImage(
      m_commandBuffer,
      VK_NULL_HANDLE,
      image->m_id,
      image->m_layout,
      regions.size(),
      regions.data()
    );
}

void IE::Graphics::CommandBuffer::recordBindVertexBuffers(
  uint32_t                             firstBinding,
  uint32_t                             bindingCount,
  std::vector<std::shared_ptr<Buffer>> buffers,
  VkDeviceSize                        *pOffsets
) {
    std::vector<VkBuffer> pVkBuffers{};
    pVkBuffers.resize(buffers.size());
    for (size_t i = 0; i < buffers.size(); ++i) pVkBuffers[i] = VK_NULL_HANDLE;
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdBindVertexBuffers(m_commandBuffer, firstBinding, bindingCount, pVkBuffers.data(), pOffsets);
}

void IE::Graphics::CommandBuffer::recordBindVertexBuffers(
  uint32_t                                    firstBinding,
  uint32_t                                    bindingCount,
  const std::vector<std::shared_ptr<Buffer>> &buffers,
  VkDeviceSize                               *pOffsets,
  VkDeviceSize                               *pSizes,
  VkDeviceSize                               *pStrides
) {
    std::vector<VkBuffer> pVkBuffers{};
    pVkBuffers.resize(buffers.size());
    for (size_t i = 0; i < buffers.size(); ++i) pVkBuffers[i] = VK_NULL_HANDLE;
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdBindVertexBuffers2(
      m_commandBuffer,
      firstBinding,
      bindingCount,
      pVkBuffers.data(),
      pOffsets,
      pSizes,
      pStrides
    );
}

void IE::Graphics::CommandBuffer::recordBindIndexBuffer(
  const std::shared_ptr<Buffer> &buffer,
  uint32_t                       offset,
  VkIndexType                    indexType

) {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdBindIndexBuffer(m_commandBuffer, VK_NULL_HANDLE, offset, indexType);
}

void IE::Graphics::CommandBuffer::recordBindPipeline(
  VkPipelineBindPoint              pipelineBindPoint,
  const std::shared_ptr<Pipeline> &pipeline
) {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdBindPipeline(m_commandBuffer, pipelineBindPoint, pipeline->m_pipeline);
}

void IE::Graphics::CommandBuffer::recordBindDescriptorSets(
  VkPipelineBindPoint                                pipelineBindPoint,
  const std::shared_ptr<Pipeline>                   &pipeline,
  uint32_t                                           firstSet,
  const std::vector<std::shared_ptr<DescriptorSet>> &descriptorSets,
  std::vector<uint32_t>                              dynamicOffsets
) {
    std::vector<VkDescriptorSet> pDescriptorSets{};
    pDescriptorSets.resize(descriptorSets.size());
    for (size_t i = 0; i < descriptorSets.size(); ++i) pDescriptorSets[i] = descriptorSets[i]->m_set;
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdBindDescriptorSets(
      m_commandBuffer,
      pipelineBindPoint,
      pipeline->m_layout,
      firstSet,
      descriptorSets.size(),
      pDescriptorSets.data(),
      dynamicOffsets.size(),
      dynamicOffsets.data()
    );
}

void IE::Graphics::CommandBuffer::recordDrawIndexed(
  uint32_t indexCount,
  uint32_t instanceCount,
  uint32_t firstIndex,
  int32_t  vertexOffset,
  uint32_t firstInstance
) {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void IE::Graphics::CommandBuffer::recordBeginRenderPass(
  VkRenderPassBeginInfo *pRenderPassBegin,
  VkSubpassContents      contents
) {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdBeginRenderPass(m_commandBuffer, pRenderPassBegin, contents);
}

void IE::Graphics::CommandBuffer::recordSetViewport(
  uint32_t          firstViewPort,
  uint32_t          viewPortCount,
  const VkViewport *pViewPorts

) {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdSetViewport(m_commandBuffer, firstViewPort, viewPortCount, pViewPorts);
}

void IE::Graphics::CommandBuffer::recordSetScissor(
  uint32_t        firstScissor,
  uint32_t        scissorCount,
  const VkRect2D *pScissors
) {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdSetScissor(m_commandBuffer, firstScissor, scissorCount, pScissors);
}

void IE::Graphics::CommandBuffer::recordEndRenderPass() {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdEndRenderPass(m_commandBuffer);
}

void IE::Graphics::CommandBuffer::destroy() {
    free();
}

void IE::Graphics::CommandBuffer::recordPipelineBarrier(
  VkPipelineStageFlags                srcStageMask,
  VkPipelineStageFlags                dstStageMask,
  VkDependencyFlags                   dependencyFlags,
  const std::vector<VkMemoryBarrier> &memoryBarriers
) {
}

void IE::Graphics::CommandBuffer::recordNextSubpass() {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdNextSubpass(m_commandBuffer, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}

void IE::Graphics::CommandBuffer::recordExecuteSecondaryCommandBuffers(
  std::vector<VkCommandBuffer> t_commandBuffers
) {
    std::lock_guard<std::mutex> lock(*m_commandPool->m_commandPoolMutex);
    if (m_status != IE_COMMAND_BUFFER_STATE_RECORDING) record();
    vkCmdExecuteCommands(m_commandBuffer, t_commandBuffers.size(), t_commandBuffers.data());
}

void IE::Graphics::CommandBuffer::recordExecuteSecondaryCommandBuffers(
  std::vector<std::shared_ptr<CommandBuffer>> t_commandBuffers
) {
    std::vector<VkCommandBuffer> commandBuffers(t_commandBuffers.size());
    for (size_t i{}; i < t_commandBuffers.size(); ++i) commandBuffers[i] = t_commandBuffers[i]->m_commandBuffer;
    recordExecuteSecondaryCommandBuffers(commandBuffers);
}

IE::Graphics::CommandBuffer::~CommandBuffer() {
    destroy();
}
