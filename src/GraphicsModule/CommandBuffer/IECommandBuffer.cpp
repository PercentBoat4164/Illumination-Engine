#include "IECommandBuffer.hpp"

#include "IERenderEngine.hpp"
#include "Core/LogModule/IELogger.hpp"


void IECommandBuffer::allocate() {
    VkCommandBufferAllocateInfo allocateInfo {
        .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool=commandPool->commandPool,
        .level=VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount=1
    };
    VkResult result = vkAllocateCommandBuffers(linkedRenderEngine->device.device, &allocateInfo, &commandBuffer);
    if (result != VK_SUCCESS) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failure to properly allocate command buffers! Error: " + IERenderEngine::translateVkResultCodes(result));
        free();
    }
    else {
        state = IE_COMMAND_BUFFER_STATE_INITIAL;
    }
}

void IECommandBuffer::record() {
    if (state == IE_COMMAND_BUFFER_STATE_NONE) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Attempt to record to command buffer before allocating!");
        allocate();
    }
    if (state > IE_COMMAND_BUFFER_STATE_INITIAL) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG, "Attempt to record to command buffer that has already finish recording and has not been reset!");
        reset();
    }
    VkCommandBufferBeginInfo beginInfo {
        .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags=0
    };
    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Failure to properly begin command buffer recording! Error: " + IERenderEngine::translateVkResultCodes(result));
    }

}

void IECommandBuffer::free() {
    vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool->commandPool, 1, &commandBuffer);
}

IECommandBuffer::IECommandBuffer(IERenderEngine *linkedRenderEngine, IECommandPool *commandPool) {
    this->linkedRenderEngine = linkedRenderEngine;
    this->commandPool = commandPool;
    state = IE_COMMAND_BUFFER_STATE_NONE;
}

void IECommandBuffer::reset() {
    vkResetCommandBuffer(commandBuffer, 0);
    state = IE_COMMAND_BUFFER_STATE_INITIAL;
}

void IECommandBuffer::finish() {
    if (state == IE_COMMAND_BUFFER_STATE_RECORDING) {
        vkEndCommandBuffer(commandBuffer);
    }
    state = IE_COMMAND_BUFFER_STATE_EXECUTABLE;
}

void IECommandBuffer::execute() {
    if (state == IE_COMMAND_BUFFER_STATE_RECORDING) {
        finish();
    }
    else if (state != IE_COMMAND_BUFFER_STATE_EXECUTABLE) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to execute a command buffer that is not executable");
    }
    VkSubmitInfo submitInfo {
        .sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount=1,
        .pCommandBuffers=&commandBuffer
    };
    vkQueueWaitIdle(commandPool->queue);
    vkQueueSubmit(commandPool->queue, 1, &submitInfo, VK_NULL_HANDLE);
    state = IE_COMMAND_BUFFER_STATE_PENDING;
}

void IECommandBuffer::recordPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const std::vector<VkMemoryBarrier>& memoryBarriers, const std::vector<IEBufferMemoryBarrier*>& bufferMemoryBarriers, const std::vector<IEImageMemoryBarrier*> &imageMemoryBarriers) {
    std::vector<VkBufferMemoryBarrier> bufferBarriers{};
    bufferBarriers.reserve(bufferMemoryBarriers.size());
    for (IEBufferMemoryBarrier *bufferMemoryBarrier : bufferMemoryBarriers) {
        addDependency(bufferMemoryBarrier->getBuffer());
        bufferBarriers.push_back(*(VkBufferMemoryBarrier*)bufferMemoryBarrier);
    }
    std::vector<VkImageMemoryBarrier> imageBarriers{};
    imageBarriers.reserve(imageMemoryBarriers.size());
    for (IEImageMemoryBarrier *imageMemoryBarrier : imageMemoryBarriers) {
        addDependency(imageMemoryBarrier->getImage());
        imageBarriers.push_back(*(VkImageMemoryBarrier*)imageMemoryBarrier);
    }
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarriers.size(), memoryBarriers.data(), bufferBarriers.size(), bufferBarriers.data(), imageBarriers.size(), imageBarriers.data());
}

void IECommandBuffer::recordPipelineBarrier(const IEDependencyInfo *dependencyInfo) const {
    vkCmdPipelineBarrier2(commandBuffer, (const VkDependencyInfo *)dependencyInfo);
}