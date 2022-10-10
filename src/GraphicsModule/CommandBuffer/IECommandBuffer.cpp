#include "IECommandBuffer.hpp"

#include "Core/LogModule/IELogger.hpp"
#include "GraphicsModule/Shader/IEDescriptorSet.hpp"
#include "GraphicsModule/Shader/IEPipeline.hpp"
#include "IEDependency.hpp"
#include "IERenderEngine.hpp"

#include <thread>

void IECommandBuffer::allocate(bool synchronize) {
    // Prepare
    VkCommandBufferAllocateInfo allocateInfo{
      .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool        = commandPool->commandPool,
      .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};

    // Lock command pool
    if (synchronize) commandPool->commandPoolMutex.lock();

    // Handle any needed state changes
    if (status == IE_COMMAND_BUFFER_STATE_INITIAL) {
        linkedRenderEngine->settings->logger.log(
          "Attempt to allocate command buffers that are already allocated.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }

    // Allocate the command buffer
    VkResult result = vkAllocateCommandBuffers(linkedRenderEngine->device.device, &allocateInfo, &commandBuffer);
    if (result != VK_SUCCESS) {  // handle any potential errors
        linkedRenderEngine->settings->logger.log(
          "Failure to properly allocate command buffers! Error: " + IERenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
        free(false);
    } else {  // Change state if no errors
        status = IE_COMMAND_BUFFER_STATE_INITIAL;
    }

    // Unlock command pool
    if (synchronize)  // Unlock this command pool if synchronizing
        commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::record(bool synchronize, bool oneTimeSubmit) {
    // Prepare
    oneTimeSubmission = oneTimeSubmit;
    VkCommandBufferBeginInfo beginInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = static_cast<VkCommandBufferUsageFlags>(
        oneTimeSubmission ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0
      ),
    };

    if (synchronize)  // Lock this command pool if synchronizing
        commandPool->commandPoolMutex.lock();

    // Handle any needed state changes
    if (status == IE_COMMAND_BUFFER_STATE_RECORDING) {
        if (synchronize) commandPool->commandPoolMutex.unlock();
        return;
    }
    if (status == IE_COMMAND_BUFFER_STATE_NONE) allocate(false);
    if (status >= IE_COMMAND_BUFFER_STATE_EXECUTABLE) reset(synchronize);

    // Begin recording
    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // Update state
    status = IE_COMMAND_BUFFER_STATE_RECORDING;

    if (synchronize)  // Unlock this command pool if synchronizing
        commandPool->commandPoolMutex.unlock();
    if (result != VK_SUCCESS) {
        linkedRenderEngine->settings->logger.log(
          "Failure to properly begin command buffer recording! Error: " +
            IERenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    }
}

void IECommandBuffer::free(bool synchronize) {
    wait();
    if (synchronize)  // Lock this command pool if synchronizing
        commandPool->commandPoolMutex.lock();
    vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool->commandPool, 1, &commandBuffer);
    status = IE_COMMAND_BUFFER_STATE_NONE;
    if (synchronize)  // Unlock this command pool if synchronizing
        commandPool->commandPoolMutex.unlock();
}

IECommandBuffer::IECommandBuffer(
  IERenderEngine                       *engineLink,
  const std::shared_ptr<IECommandPool> &parentCommandPool
) {
    linkedRenderEngine = engineLink;
    commandPool        = parentCommandPool;
    status             = IE_COMMAND_BUFFER_STATE_NONE;
}

void IECommandBuffer::reset(bool synchronize) {
    clearAllDependencies();

    if (synchronize)  // If synchronizing, lock this command pool
        commandPool->commandPoolMutex.lock();

    // Handle any needed state changes
    if (status == IE_COMMAND_BUFFER_STATE_INVALID) {
        linkedRenderEngine->settings->logger.log(
          "Attempt to reset a command buffer that is invalid!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }

    // Reset
    vkResetCommandBuffer(commandBuffer, 0);

    // Update new state
    status = IE_COMMAND_BUFFER_STATE_INITIAL;

    if (synchronize)  // If synchronizing, unlock this command pool
        commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::finish(bool synchronize) {
    if (synchronize)  // If synchronizing, lock this command pool
        commandPool->commandPoolMutex.lock();

    // Handle any needed state changes
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        linkedRenderEngine->settings->logger.log(
          "Attempt to finish a command buffer that was not recording.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }

    // End
    vkEndCommandBuffer(commandBuffer);

    // Update state
    status = IE_COMMAND_BUFFER_STATE_EXECUTABLE;

    if (synchronize)  // If synchronizing, unlock this command pool
        commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::execute(VkSemaphore input, VkSemaphore output, VkFence fence) {
    wait();
    commandPool->commandPoolMutex.lock();
    //    executionThread = std::thread{[&] {
    if (status == IE_COMMAND_BUFFER_STATE_RECORDING) {
        finish(false);
    } else if (status != IE_COMMAND_BUFFER_STATE_EXECUTABLE) {
        linkedRenderEngine->settings->logger.log(

          "Attempt to execute a command buffer that is not recording or executable!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }
    VkSubmitInfo submitInfo{
      .sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .waitSemaphoreCount   = input != (VkSemaphore) nullptr,
      .pWaitSemaphores      = &input,
      .pWaitDstStageMask    = new VkPipelineStageFlags{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
      .commandBufferCount   = 1,
      .pCommandBuffers      = &commandBuffer,
      .signalSemaphoreCount = output != (VkSemaphore) nullptr,
      .pSignalSemaphores    = &output,
    };

    // Set up fence if not already setup.
    bool fenceWasNullptr = fence == (VkFence) nullptr;
    if (fence == (VkFence) nullptr) {
        VkFenceCreateInfo fenceCreateInfo{.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
        vkCreateFence(linkedRenderEngine->device.device, &fenceCreateInfo, nullptr, &fence);
    }
    vkResetFences(linkedRenderEngine->device.device, 1, &fence);


    // Lock command buffer
    status = IE_COMMAND_BUFFER_STATE_PENDING;

    // Submit
    VkResult result = vkQueueSubmit(commandPool->queue, 1, &submitInfo, fence);


    // Wait for GPU to finish then unlock
    vkWaitForFences(linkedRenderEngine->device.device, 1, &fence, VK_TRUE, UINT64_MAX);
    if (result != VK_SUCCESS) {
        linkedRenderEngine->settings->logger.log(
          "Failed to submit command buffer! Error: " + IERenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }
    status = oneTimeSubmission ? IE_COMMAND_BUFFER_STATE_INVALID : IE_COMMAND_BUFFER_STATE_EXECUTABLE;


    // Delete to avoid a memory leak
    delete submitInfo.pWaitDstStageMask;

    if (fenceWasNullptr)  // destroy the fence if it was created in this function
        vkDestroyFence(linkedRenderEngine->device.device, fence, nullptr);
    oneTimeSubmission = false;

    // All tasks are done. Command buffer can be unlocked.
    commandPool->commandPoolMutex.unlock();
    //	}};
}

void IECommandBuffer::recordPipelineBarrier(
  VkPipelineStageFlags                      srcStageMask,
  VkPipelineStageFlags                      dstStageMask,
  VkDependencyFlags                         dependencyFlags,
  const std::vector<VkMemoryBarrier>       &memoryBarriers,
  const std::vector<IEBufferMemoryBarrier> &bufferMemoryBarriers,
  const std::vector<IEImageMemoryBarrier>  &imageMemoryBarriers
) {
    std::vector<VkBufferMemoryBarrier> bufferBarriers{};
    bufferBarriers.reserve(bufferMemoryBarriers.size());
    for (const IEBufferMemoryBarrier &bufferMemoryBarrier : bufferMemoryBarriers) {
        addDependencies(bufferMemoryBarrier.getDependencies());
        bufferBarriers.push_back((VkBufferMemoryBarrier) bufferMemoryBarrier);
    }
    std::vector<VkImageMemoryBarrier> imageBarriers{};
    imageBarriers.reserve(imageMemoryBarriers.size());
    for (const IEImageMemoryBarrier &imageMemoryBarrier : imageMemoryBarriers) {
        addDependencies(imageMemoryBarrier.getDependencies());
        imageBarriers.push_back((VkImageMemoryBarrier) imageMemoryBarrier);
    }
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
              "Attempt to record a pipeline barrier on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdPipelineBarrier(
      commandBuffer,
      srcStageMask,
      dstStageMask,
      dependencyFlags,
      memoryBarriers.size(),
      memoryBarriers.data(),
      bufferBarriers.size(),
      bufferBarriers.data(),
      imageBarriers.size(),
      imageBarriers.data()
    );
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordPipelineBarrier(const IEDependencyInfo *dependencyInfo) {
    addDependencies(dependencyInfo->getDependencies());
    commandPool->commandPoolMutex.lock();
    if (status == IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
              "Attempt to record a pipeline barrier on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdPipelineBarrier2(commandBuffer, (const VkDependencyInfo *) dependencyInfo);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordCopyBufferToImage(
  const std::shared_ptr<IEBuffer> &buffer,
  const std::shared_ptr<IEImage>  &image,
  std::vector<VkBufferImageCopy>   regions
) {
    addDependencies({image, buffer});
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
              "Attempt to record a buffer to image copy on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdCopyBufferToImage(
      commandBuffer,
      buffer->buffer,
      image->image,
      image->layout,
      regions.size(),
      regions.data()
    );
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordCopyBufferToImage(IECopyBufferToImageInfo *copyInfo) {
    addDependencies(copyInfo->getDependencies());
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
              "Attempt to record a buffer to image copy on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdCopyBufferToImage2(commandBuffer, (const VkCopyBufferToImageInfo2 *) copyInfo);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordBindVertexBuffers(
  uint32_t                               firstBinding,
  uint32_t                               bindingCount,
  std::vector<std::shared_ptr<IEBuffer>> buffers,
  VkDeviceSize                          *pOffsets
) {
    std::vector<VkBuffer> pVkBuffers{};
    pVkBuffers.resize(buffers.size());
    for (size_t i = 0; i < buffers.size(); ++i) {
        pVkBuffers[i] = buffers[i]->buffer;
        addDependency((std::shared_ptr<IEDependency>) buffers[i]);
    }
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
              "Attempt to record a vertex buffer bind on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pVkBuffers.data(), pOffsets);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordBindVertexBuffers(
  uint32_t                                      firstBinding,
  uint32_t                                      bindingCount,
  const std::vector<std::shared_ptr<IEBuffer>> &buffers,
  VkDeviceSize                                 *pOffsets,
  VkDeviceSize                                 *pSizes,
  VkDeviceSize                                 *pStrides
) {
    std::vector<VkBuffer> pVkBuffers{};
    pVkBuffers.resize(buffers.size());
    for (size_t i = 0; i < buffers.size(); ++i) {
        pVkBuffers[i] = buffers[i]->buffer;
        addDependency(buffers[i]);
    }
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
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
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordBindIndexBuffer(
  const std::shared_ptr<IEBuffer> &buffer,
  uint32_t                         offset,
  VkIndexType                      indexType
) {
    addDependency(buffer);
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
              "Attempt to record an index buffer bind on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdBindIndexBuffer(commandBuffer, buffer->buffer, offset, indexType);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordBindPipeline(
  VkPipelineBindPoint                pipelineBindPoint,
  const std::shared_ptr<IEPipeline> &pipeline
) {
    addDependency(pipeline);
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
              "Attempt to record a pipeline bind on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline->pipeline);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordBindDescriptorSets(
  VkPipelineBindPoint                                  pipelineBindPoint,
  const std::shared_ptr<IEPipeline>                   &pipeline,
  uint32_t                                             firstSet,
  const std::vector<std::shared_ptr<IEDescriptorSet>> &descriptorSets,
  std::vector<uint32_t>                                dynamicOffsets
) {
    addDependency(pipeline);
    std::vector<VkDescriptorSet> pDescriptorSets{};
    pDescriptorSets.resize(descriptorSets.size());
    for (size_t i = 0; i < descriptorSets.size(); ++i) {
        pDescriptorSets[i] = descriptorSets[i]->descriptorSet;
        addDependency(descriptorSets[i]);
    }
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
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
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordDrawIndexed(
  uint32_t indexCount,
  uint32_t instanceCount,
  uint32_t firstIndex,
  int32_t  vertexOffset,
  uint32_t firstInstance
) {
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
              "Attempt to record an indexed draw on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordBeginRenderPass(IERenderPassBeginInfo *pRenderPassBegin, VkSubpassContents contents) {
    addDependencies(pRenderPassBegin->getDependencies());
    VkRenderPassBeginInfo renderPassBeginInfo = *(VkRenderPassBeginInfo *) pRenderPassBegin;
    renderPassBeginInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext                 = pRenderPassBegin->pNext;
    renderPassBeginInfo.renderPass            = pRenderPassBegin->renderPass->renderPass;
    renderPassBeginInfo.framebuffer =
      pRenderPassBegin->framebuffer->getFramebuffer(pRenderPassBegin->framebufferIndex);
    renderPassBeginInfo.renderArea      = pRenderPassBegin->renderArea;
    renderPassBeginInfo.clearValueCount = pRenderPassBegin->clearValueCount;
    renderPassBeginInfo.pClearValues    = pRenderPassBegin->pClearValues;
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
              "Attempt to record a render pass beginning on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, contents);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordSetViewport(
  uint32_t          firstViewPort,
  uint32_t          viewPortCount,
  const VkViewport *pViewPorts
) {
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
              "Attempt to record a viewport set on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdSetViewport(commandBuffer, firstViewPort, viewPortCount, pViewPorts);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordSetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *pScissors) {
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
              "Attempt to record a scissor set on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordEndRenderPass() {
    commandPool->commandPoolMutex.lock();
    if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (status != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(
              "Attempt to record a render pass ending on a command buffer that is not recording!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        }
    }
    vkCmdEndRenderPass(commandBuffer);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::wait() {
    if (executionThread.joinable()) executionThread.join();
}

void IECommandBuffer::destroy() {
    free();
    clearAllDependencies();
}

IECommandBuffer::~IECommandBuffer() {
    destroy();
}

void IECommandBuffer::invalidate() {
    status = IE_COMMAND_BUFFER_STATE_INVALID;
    clearAllDependencies();
}

bool IECommandBuffer::canBeDestroyed(const std::shared_ptr<IEDependency> &, bool) {
    return false;
}

void IECommandBuffer::freeDependencies() {
    invalidate();
}
