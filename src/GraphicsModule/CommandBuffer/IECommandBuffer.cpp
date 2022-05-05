#include <thread>

#include "IECommandBuffer.hpp"
#include "IEDependency.hpp"
#include "IERenderEngine.hpp"
#include "IEDescriptorSet.hpp"
#include "Core/LogModule/IELogger.hpp"
#include "IEPipeline.hpp"


void IECommandBuffer::allocate(bool synchronize) {
    // Prepare
    VkCommandBufferAllocateInfo allocateInfo{
            .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool=commandPool->commandPool,
            .level=VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount=1
    };

    // Lock command pool
    if (synchronize) {
        commandPool->commandPoolMutex.lock();
    }

    // Handle any needed state changes
    if (state == IE_COMMAND_BUFFER_STATE_INITIAL) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to allocate command buffers that are already allocated.");
    }

    // Allocate the command buffer
    VkResult result = vkAllocateCommandBuffers(linkedRenderEngine->device.device, &allocateInfo, &commandBuffer);
    if (result != VK_SUCCESS) {  // handle any potential errors
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failure to properly allocate command buffers! Error: " + IERenderEngine::translateVkResultCodes(result));
        free(false);
    } else {  // Change state if no errors
        state = IE_COMMAND_BUFFER_STATE_INITIAL;
    }

    // Unlock command pool
    if (synchronize) {  // Unlock this command pool if synchronizing
        commandPool->commandPoolMutex.unlock();
    }
}

void IECommandBuffer::record(bool synchronize, bool oneTimeSubmit) {
    // Prepare
    oneTimeSubmission = oneTimeSubmit;
    VkCommandBufferBeginInfo beginInfo {
        .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags=static_cast<VkCommandBufferUsageFlags>(oneTimeSubmission ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0),
    };

    if (synchronize) {  // Lock this command pool if synchronizing
        commandPool->commandPoolMutex.lock();
    }

    // Handle any needed state changes
    if (state == IE_COMMAND_BUFFER_STATE_RECORDING) {
        if (synchronize) {
            commandPool->commandPoolMutex.unlock();
        }
        return;
    }
    if (state == IE_COMMAND_BUFFER_STATE_NONE) {
        allocate(false);
    }
    if (state >= IE_COMMAND_BUFFER_STATE_EXECUTABLE) {
        reset(false);
    }

    // Begin recording
    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);

    // Update state
    state = IE_COMMAND_BUFFER_STATE_RECORDING;

    if (synchronize) {  // Unlock this command pool if synchronizing
        commandPool->commandPoolMutex.unlock();
    }
    if (result != VK_SUCCESS) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Failure to properly begin command buffer recording! Error: " + IERenderEngine::translateVkResultCodes(result));
    }
}

void IECommandBuffer::free(bool synchronize) {
    wait();
    if (synchronize) {  // Lock this command pool if synchronizing
        commandPool->commandPoolMutex.lock();
    }
    vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool->commandPool, 1, &commandBuffer);
    state = IE_COMMAND_BUFFER_STATE_NONE;
    if (synchronize) {  // Unlock this command pool if synchronizing
        commandPool->commandPoolMutex.unlock();
    }
}

IECommandBuffer::IECommandBuffer(IERenderEngine *linkedRenderEngine, IECommandPool *commandPool) {
    this->linkedRenderEngine = linkedRenderEngine;
    this->commandPool = commandPool;
    state = IE_COMMAND_BUFFER_STATE_NONE;
}

void IECommandBuffer::reset(bool synchronize) {
    removeAllDependencies();

    if (synchronize) {  // If synchronizing, lock this command pool
        commandPool->commandPoolMutex.lock();
    }

    // Handle any needed state changes
    if (state == IE_COMMAND_BUFFER_STATE_INVALID) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to reset a command buffer that is invalid!");
    }

    // Reset
    vkResetCommandBuffer(commandBuffer, 0);

    // Update new state
    state = IE_COMMAND_BUFFER_STATE_INITIAL;

    if (synchronize) {  // If synchronizing, unlock this command pool
        commandPool->commandPoolMutex.unlock();
    }
}

void IECommandBuffer::finish(bool synchronize) {
    if (synchronize) {  // If synchronizing, lock this command pool
        commandPool->commandPoolMutex.lock();
    }

    // Handle any needed state changes
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to finish a command buffer that was not recording.");
    }

    // End
    vkEndCommandBuffer(commandBuffer);

    // Update state
    state = IE_COMMAND_BUFFER_STATE_EXECUTABLE;

    if (synchronize) {  // If synchronizing, unlock this command pool
        commandPool->commandPoolMutex.unlock();
    }
}

void IECommandBuffer::execute(VkSemaphore input, VkSemaphore output, VkFence fence) {
    wait();
    commandPool->commandPoolMutex.lock();
    executionThread = std::thread {[this](VkSemaphore thisInput, VkSemaphore thisOutput, VkFence thisFence) {
        if (state == IE_COMMAND_BUFFER_STATE_RECORDING) {
            finish(false);
        } else if (state != IE_COMMAND_BUFFER_STATE_EXECUTABLE) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to execute a command buffer that is not recording or executable!");
        }
        VkSubmitInfo submitInfo{
                .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                .waitSemaphoreCount = thisInput != nullptr,
                .pWaitSemaphores = &thisInput,
                .pWaitDstStageMask = new VkPipelineStageFlags{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
                .commandBufferCount = 1,
                .pCommandBuffers = &commandBuffer,
                .signalSemaphoreCount = thisOutput != nullptr,
                .pSignalSemaphores = &thisOutput,
        };

        // Set up fence if not already setup.
        bool fenceWasNullptr = thisFence == nullptr;
        if (thisFence == nullptr) {
            VkFenceCreateInfo fenceCreateInfo{
                    .sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
            };
            vkCreateFence(linkedRenderEngine->device.device, &fenceCreateInfo, nullptr, &thisFence);
        }
        vkResetFences(linkedRenderEngine->device.device, 1, &thisFence);


        // Lock command buffer
        state = IE_COMMAND_BUFFER_STATE_PENDING;

        // Submit
        VkResult result = vkQueueSubmit(commandPool->queue, 1, &submitInfo, thisFence);


        // Wait for GPU to finish then unlock
        vkWaitForFences(linkedRenderEngine->device.device, 1, &thisFence, VK_TRUE, UINT64_MAX);
        if (result != VK_SUCCESS) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to submit command buffer! Error: " + IERenderEngine::translateVkResultCodes(result));
        }
        state = oneTimeSubmission ? IE_COMMAND_BUFFER_STATE_INVALID : IE_COMMAND_BUFFER_STATE_EXECUTABLE;


        // Delete to avoid a memory leak
        delete submitInfo.pWaitDstStageMask;

        if (fenceWasNullptr) {  // destroy the fence if it was created in this _function
            vkDestroyFence(linkedRenderEngine->device.device, thisFence, nullptr);
        }
        oneTimeSubmission = false;


        commandPool->commandPoolMutex.unlock();
    }, input, output, fence};
}

void IECommandBuffer::recordPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const std::vector<VkMemoryBarrier>& memoryBarriers, const std::vector<IEBufferMemoryBarrier>& bufferMemoryBarriers, const std::vector<IEImageMemoryBarrier> &imageMemoryBarriers) {
    std::vector<VkBufferMemoryBarrier> bufferBarriers{};
    bufferBarriers.reserve(bufferMemoryBarriers.size());
    for (const IEBufferMemoryBarrier& bufferMemoryBarrier : bufferMemoryBarriers) {
        addDependencies(bufferMemoryBarrier.getDependencies());
        bufferBarriers.push_back((VkBufferMemoryBarrier)bufferMemoryBarrier);
    }
    std::vector<VkImageMemoryBarrier> imageBarriers{};
    imageBarriers.reserve(imageMemoryBarriers.size());
    for (const IEImageMemoryBarrier& imageMemoryBarrier : imageMemoryBarriers) {
        addDependencies(imageMemoryBarrier.getDependencies());
        imageBarriers.push_back((VkImageMemoryBarrier)imageMemoryBarrier);
    }
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a pipeline barrier on a command buffer that is not recording!");
        }
    }
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarriers.size(), memoryBarriers.data(), bufferBarriers.size(), bufferBarriers.data(), imageBarriers.size(), imageBarriers.data());
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordPipelineBarrier(const IEDependencyInfo *dependencyInfo) {
    addDependencies(dependencyInfo->getDependencies());
    commandPool->commandPoolMutex.lock();
    if (state == IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a pipeline barrier on a command buffer that is not recording!");
        }
    }
    vkCmdPipelineBarrier2(commandBuffer, (const VkDependencyInfo *)dependencyInfo);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordCopyBufferToImage(IEBuffer *buffer, IEImage *image, std::vector<VkBufferImageCopy> regions) {
    addDependencies({image, buffer});
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a buffer to image copy on a command buffer that is not recording!");
        }
    }
    vkCmdCopyBufferToImage(commandBuffer, buffer->buffer, image->image, image->imageLayout, regions.size(), regions.data());
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordCopyBufferToImage(IECopyBufferToImageInfo *copyInfo) {
    addDependencies(copyInfo->getDependencies());
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a buffer to image copy on a command buffer that is not recording!");
        }
    }
    vkCmdCopyBufferToImage2(commandBuffer, (const VkCopyBufferToImageInfo2 *)copyInfo);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordBindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const std::vector<IEBuffer*>& buffers, VkDeviceSize *pOffsets) {
    VkBuffer pVkBuffers[buffers.size()];
    for (int i = 0; i < buffers.size(); ++i) {
        pVkBuffers[i] = buffers[i]->buffer;
        addDependency(buffers[i]);
    }
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a vertex buffer bind on a command buffer that is not recording!");
        }
    }
    vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pVkBuffers, pOffsets);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordBindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const std::vector<IEBuffer *>& buffers, VkDeviceSize *pOffsets, VkDeviceSize *pSizes, VkDeviceSize *pStrides) {
    VkBuffer pVkBuffers[buffers.size()];
    for (int i = 0; i < buffers.size(); ++i) {
        pVkBuffers[i] = buffers[i]->buffer;
        addDependency(buffers[i]);
    }
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a vertex buffer bind on a command buffer that is not recording!");
        }
    }
    vkCmdBindVertexBuffers2(commandBuffer, firstBinding, bindingCount, pVkBuffers, pOffsets, pSizes, pStrides);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordBindIndexBuffer(IEBuffer *buffer, uint32_t offset, VkIndexType indexType) {
    addDependency(buffer);
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record an index buffer bind on a command buffer that is not recording!");
        }
    }
    vkCmdBindIndexBuffer(commandBuffer, buffer->buffer, offset, indexType);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordBindPipeline(VkPipelineBindPoint pipelineBindPoint, IEPipeline *pipeline) {
    addDependency(pipeline);
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a pipeline bind on a command buffer that is not recording!");
        }
    }
    vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline->pipeline);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordBindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, IEPipeline *pipeline, uint32_t firstSet, const std::vector<IEDescriptorSet *>& descriptorSets, std::vector<uint32_t> dynamicOffsets) {
    addDependency(pipeline);
    VkDescriptorSet pDescriptorSets[descriptorSets.size()];
    for (int i = 0; i < descriptorSets.size(); ++i) {
        pDescriptorSets[i] = descriptorSets[i]->descriptorSet;
        addDependency(descriptorSets[i]);
    }
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a descriptor set bind on a command buffer that is not recording!");
        }
    }
    vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, pipeline->pipelineLayout, firstSet, descriptorSets.size(), pDescriptorSets, dynamicOffsets.size(), dynamicOffsets.data());
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record an indexed draw on a command buffer that is not recording!");
        }
    }
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordBeginRenderPass(IERenderPassBeginInfo *pRenderPassBegin, VkSubpassContents contents) {
    addDependencies(pRenderPassBegin->getDependencies());
    VkRenderPassBeginInfo renderPassBeginInfo = *(VkRenderPassBeginInfo *)pRenderPassBegin;
    renderPassBeginInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext=pRenderPassBegin->pNext;
    renderPassBeginInfo.renderPass=pRenderPassBegin->renderPass->renderPass;
    renderPassBeginInfo.framebuffer=pRenderPassBegin->framebuffer->framebuffer;
    renderPassBeginInfo.renderArea=pRenderPassBegin->renderArea;
    renderPassBeginInfo.clearValueCount=pRenderPassBegin->clearValueCount;
    renderPassBeginInfo.pClearValues=pRenderPassBegin->pClearValues;
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a render pass beginning on a command buffer that is not recording!");
        }
    }
    vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, contents);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordSetViewport(uint32_t firstViewPort, uint32_t viewPortCount, const VkViewport *pViewPorts) {
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a viewport set on a command buffer that is not recording!");
        }
    }
    vkCmdSetViewport(commandBuffer, firstViewPort, viewPortCount, pViewPorts);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordSetScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *pScissors) {
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a scissor set on a command buffer that is not recording!");
        }
    }
    vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::recordEndRenderPass() {
    commandPool->commandPoolMutex.lock();
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        record(false);
        if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a render pass ending on a command buffer that is not recording!");
        }
    }
    vkCmdEndRenderPass(commandBuffer);
    commandPool->commandPoolMutex.unlock();
}

void IECommandBuffer::wait() {
    if (executionThread.joinable()) {
        executionThread.join();
    }
}

void IECommandBuffer::destroy() {
    free();
    removeAllDependencies();
}

IECommandBuffer::~IECommandBuffer() {
    destroy();
}
