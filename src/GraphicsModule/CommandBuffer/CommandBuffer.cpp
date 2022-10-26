#include "CommandBuffer.hpp"

#include "Buffer/Buffer.hpp"
#include "CommandPool.hpp"
#include "Core/LogModule/IELogger.hpp"
#include "Image/ImageVulkan.hpp"
#include "RenderEngine.hpp"
#include "Shader/IEDescriptorSet.hpp"
#include "Shader/IEPipeline.hpp"


void IE::Graphics::CommandBuffer::allocate(AllocationFlags t_flags) {
    // Prepare
    VkCommandBufferAllocateInfo allocateInfo{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext              = nullptr,
      .commandPool        = commandPool->m_commandPool,
      .level              = (t_flags & IE_COMMAND_BUFFER_ALLOCATE_SECONDARY) != 0U ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};

    // Lock command pool
    commandPool->m_commandPoolMutex->lock();

    // Handle any needed status changes
    if (status == IE_COMMAND_BUFFER_STATE_INITIAL) {
        linkedRenderEngine->getLogger().log(
          "Attempt to allocate command buffers that are already allocated.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
        commandPool->m_commandPoolMutex->unlock();
        return;
    }

    // Allocate the command buffer
    VkResult result = vkAllocateCommandBuffers(linkedRenderEngine->m_device.device, &allocateInfo, &commandBuffer);
    if (result != VK_SUCCESS) {  // handle any potential errors
        linkedRenderEngine->getLogger().log(
          "Failure to properly allocate command buffers! Error: " +
            IE::Graphics::RenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    } else status = IE_COMMAND_BUFFER_STATE_INITIAL;

    // Unlock command pool
    commandPool->m_commandPoolMutex->unlock();
}

void IE::Graphics::CommandBuffer::record(RecordFlags t_flags) {
    commandPool->m_commandPoolMutex->lock();

    recordFlags = t_flags;
    VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = static_cast<VkCommandBufferUsageFlags>(
        (recordFlags & IE_COMMAND_BUFFER_RECORD_ONE_TIME_SUBMIT) != 0U ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0
      ),
    };

    // Handle any needed status changes
    if (status == IE_COMMAND_BUFFER_STATE_RECORDING) {
        linkedRenderEngine->getLogger().log("Attempted to record a command buffer that was already in the recording state.", IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN);
        commandPool->m_commandPoolMutex->unlock();
        return;
    }
    if (status == IE_COMMAND_BUFFER_STATE_NONE) linkedRenderEngine->getLogger().log("Attempted to record an unallocated command buffer!", IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR);
    if (status >= IE_COMMAND_BUFFER_STATE_EXECUTABLE) linkedRenderEngine->getLogger().log("Attempted to record to a command buffer that has already finished its execution phase. Call 'reset' on this command buffer before calling 'record'.", IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR);

    // Begin recording
    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // Update status
    status = IE_COMMAND_BUFFER_STATE_RECORDING;

    commandPool->m_commandPoolMutex->unlock();
    if (result != VK_SUCCESS)
        linkedRenderEngine->getLogger().log(
          "Failure to properly begin command buffer recording! Error: " +
            IE::Graphics::RenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
}

void IE::Graphics::CommandBuffer::free(bool synchronize) {
    wait();
    if (synchronize)  // Lock this command pool if synchronizing
        commandPool->m_commandPoolMutex->lock();
    vkFreeCommandBuffers(linkedRenderEngine->m_device.device, commandPool->m_commandPool, 1, &commandBuffer);
    status = IE_COMMAND_BUFFER_STATE_NONE;
    if (synchronize)  // Unlock this command pool if synchronizing
        commandPool->m_commandPoolMutex->unlock();
}

IE::Graphics::CommandBuffer::CommandBuffer(
  IE::Graphics::RenderEngine *engineLink,
  IE::Graphics::CommandPool  *parentCommandPool
) :
        commandPool(parentCommandPool),
        linkedRenderEngine(engineLink),
        status(IE_COMMAND_BUFFER_STATE_NONE) {
}

void IE::Graphics::CommandBuffer::reset(bool synchronize) {
    if (synchronize)  // If synchronizing, lock this command pool
        commandPool->m_commandPoolMutex->lock();

    // Handle any needed status changes
    if (status == IE_COMMAND_BUFFER_STATE_INVALID) {
        linkedRenderEngine->getLogger().log(
          "Attempt to reset a command buffer that is invalid!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }

    // Reset
    vkResetCommandBuffer(commandBuffer, 0);

    // Update new status
    status = IE_COMMAND_BUFFER_STATE_INITIAL;

    if (synchronize)  // If synchronizing, unlock this command pool
        commandPool->m_commandPoolMutex->unlock();
}

void IE::Graphics::CommandBuffer::finish(bool synchronize) {
    if (synchronize)  // If synchronizing, lock this command pool
        commandPool->m_commandPoolMutex->lock();

    // Handle any needed status changes
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        linkedRenderEngine->getLogger().log(
          "Attempt to finish a command buffer that was not recording.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }

    // End
    vkEndCommandBuffer(commandBuffer);

    // Update status
    status = IE_COMMAND_BUFFER_STATE_EXECUTABLE;

    if (synchronize)  // If synchronizing, unlock this command pool
        commandPool->m_commandPoolMutex->unlock();
}

void IE::Graphics::CommandBuffer::execute(VkSemaphore input, VkSemaphore output, VkFence fence) {
    wait();
    commandPool->m_commandPoolMutex->lock();
    //    executionThread = std::thread{[&] {
    if (status == IE_COMMAND_BUFFER_STATE_RECORDING) {
        finish(false);
    } else if (status != IE_COMMAND_BUFFER_STATE_EXECUTABLE) {
        linkedRenderEngine->getLogger().log(
          "Attempt to execute a command buffer that is not recording or executable!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }

    VkPipelineStageFlags dstStageMask{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo{
      .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount   = static_cast<uint32_t>(input != nullptr),
      .pWaitSemaphores      = &input,
      .pWaitDstStageMask    = &dstStageMask,
      .commandBufferCount   = 1,
      .pCommandBuffers      = &commandBuffer,
      .signalSemaphoreCount = static_cast<uint32_t>(output != nullptr),
      .pSignalSemaphores    = &output,
    };

    // Set up fence if not already setup.
    bool fenceWasNullptr = fence == (VkFence) nullptr;
    if (fence == (VkFence) nullptr) {
        VkFenceCreateInfo fenceCreateInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vkCreateFence(linkedRenderEngine->m_device.device, &fenceCreateInfo, nullptr, &fence);
    }
    vkResetFences(linkedRenderEngine->m_device.device, 1, &fence);


    // Lock command buffer
    status = IE_COMMAND_BUFFER_STATE_PENDING;

    // Submit
    VkResult result = vkQueueSubmit(commandPool->m_queue, 1, &submitInfo, fence);


    // Wait for GPU to finish then unlock
    vkWaitForFences(linkedRenderEngine->m_device.device, 1, &fence, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS) {
        linkedRenderEngine->getLogger().log(
          "Failed to submit command buffer! Error: " + RenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }
    status = (recordFlags & IE_COMMAND_BUFFER_RECORD_ONE_TIME_SUBMIT) != 0U ? IE_COMMAND_BUFFER_STATE_INVALID : IE_COMMAND_BUFFER_STATE_EXECUTABLE;

    if (fenceWasNullptr)  // destroy the fence if it was created in this function
        vkDestroyFence(linkedRenderEngine->m_device.device, fence, nullptr);

    // All tasks are done. Command buffer can be unlocked.
    commandPool->m_commandPoolMutex->unlock();
    //	}};
}

void IE::Graphics::CommandBuffer::recordCopyBufferToImage(
  const std::shared_ptr<Buffer>                            &buffer,
  const std::shared_ptr<IE::Graphics::detail::ImageVulkan> &image,
  std::vector<VkBufferImageCopy>                            regions

) {
    commandPool->m_commandPoolMutex->lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->getLogger().log(
              "Attempt to record a buffer to image copy on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdCopyBufferToImage(
      commandBuffer,
      VK_NULL_HANDLE,
      image->m_id,
      image->m_layout,
      regions.size(),
      regions.data()
    );
    commandPool->m_commandPoolMutex->unlock();
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
    commandPool->m_commandPoolMutex->lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->getLogger().log(

              "Attempt to record a vertex buffer bind on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pVkBuffers.data(), pOffsets);
    commandPool->m_commandPoolMutex->unlock();
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
    commandPool->m_commandPoolMutex->lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->getLogger().log(
              "Attempt to record a vertex buffer bind on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdBindVertexBuffers2(
      commandBuffer,
      firstBinding,
      bindingCount,
      pVkBuffers.data(),
      pOffsets,
      pSizes,
      pStrides
    );
    commandPool->m_commandPoolMutex->unlock();
}

void IE::Graphics::CommandBuffer::recordBindIndexBuffer(
  const std::shared_ptr<Buffer> &buffer,
  uint32_t                       offset,
  VkIndexType                    indexType

) {
    commandPool->m_commandPoolMutex->lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->getLogger().log(
              "Attempt to record an index buffer bind on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdBindIndexBuffer(commandBuffer, VK_NULL_HANDLE, offset, indexType);
    commandPool->m_commandPoolMutex->unlock();
}

void IE::Graphics::CommandBuffer::recordBindPipeline(
  VkPipelineBindPoint                pipelineBindPoint,
  const std::shared_ptr<IEPipeline> &pipeline
) {
    commandPool->m_commandPoolMutex->lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->getLogger().log(
              "Attempt to record a pipeline bind on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline->pipeline);
    commandPool->m_commandPoolMutex->unlock();
}

void IE::Graphics::CommandBuffer::recordBindDescriptorSets(
  VkPipelineBindPoint                                  pipelineBindPoint,
  const std::shared_ptr<IEPipeline>                   &pipeline,
  uint32_t                                             firstSet,
  const std::vector<std::shared_ptr<IEDescriptorSet>> &descriptorSets,
  std::vector<uint32_t>                                dynamicOffsets
) {
    std::vector<VkDescriptorSet> pDescriptorSets{};
    pDescriptorSets.resize(descriptorSets.size());
    for (size_t i = 0; i < descriptorSets.size(); ++i) pDescriptorSets[i] = descriptorSets[i]->descriptorSet;
    commandPool->m_commandPoolMutex->lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->getLogger().log(

              "Attempt to record a descriptor set bind on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdBindDescriptorSets(
      commandBuffer,
      pipelineBindPoint,
      pipeline->pipelineLayout,
      firstSet,
      descriptorSets.size(),
      pDescriptorSets.data(),
      dynamicOffsets.size(),
      dynamicOffsets.data()
    );
    commandPool->m_commandPoolMutex->unlock();
}

void IE::Graphics::CommandBuffer::recordDrawIndexed(
  uint32_t indexCount,
  uint32_t instanceCount,
  uint32_t firstIndex,
  int32_t  vertexOffset,
  uint32_t firstInstance
) {
    commandPool->m_commandPoolMutex->lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->getLogger().log(
              "Attempt to record an indexed draw on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    commandPool->m_commandPoolMutex->unlock();
}

void IE::Graphics::CommandBuffer::recordSetViewport(
  uint32_t          firstViewPort,
  uint32_t          viewPortCount,
  const VkViewport *pViewPorts

) {
    commandPool->m_commandPoolMutex->lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->getLogger().log(
              "Attempt to record a viewport set on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdSetViewport(commandBuffer, firstViewPort, viewPortCount, pViewPorts);
    commandPool->m_commandPoolMutex->unlock();
}

void IE::Graphics::CommandBuffer::recordSetScissor(
  uint32_t        firstScissor,
  uint32_t        scissorCount,
  const VkRect2D *pScissors
) {
    commandPool->m_commandPoolMutex->lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->getLogger().log(
              "Attempt to record a scissor set on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    commandPool->m_commandPoolMutex->unlock();
}

void IE::Graphics::CommandBuffer::recordEndRenderPass() {
    commandPool->m_commandPoolMutex->lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->getLogger().log(
              "Attempt to record a render pass ending on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdEndRenderPass(commandBuffer);
    commandPool->m_commandPoolMutex->unlock();
}

void IE::Graphics::CommandBuffer::wait() {
    /**@todo Use mutexes to wait. */
    //    if (executionThread.joinable()) executionThread.join();
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