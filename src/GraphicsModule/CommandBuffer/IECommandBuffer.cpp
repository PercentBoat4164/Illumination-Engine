#include <thread>
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

void IECommandBuffer::record(bool oneTimeSubmit) {
    oneTimeSubmission = oneTimeSubmit;
    if (state == IE_COMMAND_BUFFER_STATE_RECORDING) {
        return;
    }
    if (state == IE_COMMAND_BUFFER_STATE_NONE) {
        allocate();
    }
    if (state >= IE_COMMAND_BUFFER_STATE_EXECUTABLE) {
        reset();
    }
    VkCommandBufferBeginInfo beginInfo {
        .sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags=static_cast<VkCommandBufferUsageFlags>(oneTimeSubmission ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0),
    };
    VkResult result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (result != VK_SUCCESS) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Failure to properly begin command buffer recording! Error: " + IERenderEngine::translateVkResultCodes(result));
    }
    state = IE_COMMAND_BUFFER_STATE_RECORDING;
}

void IECommandBuffer::free() {
    vkFreeCommandBuffers(linkedRenderEngine->device.device, commandPool->commandPool, 1, &commandBuffer);
    state = IE_COMMAND_BUFFER_STATE_NONE;
}

IECommandBuffer::IECommandBuffer(IERenderEngine *linkedRenderEngine, IECommandPool *commandPool) {
    this->linkedRenderEngine = linkedRenderEngine;
    this->commandPool = commandPool;
    state = IE_COMMAND_BUFFER_STATE_NONE;
}

void IECommandBuffer::reset() {
    while (state == IE_COMMAND_BUFFER_STATE_PENDING) {}
    if (state == IE_COMMAND_BUFFER_STATE_INVALID) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to reset a command buffer that is invalid!");
    }
    vkResetCommandBuffer(commandBuffer, 0);
    state = IE_COMMAND_BUFFER_STATE_INITIAL;
}

void IECommandBuffer::finish() {
    if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to finish a command buffer that was not recording.");
    }
    vkEndCommandBuffer(commandBuffer);
    state = IE_COMMAND_BUFFER_STATE_EXECUTABLE;
}

void IECommandBuffer::execute() {
    if (state == IE_COMMAND_BUFFER_STATE_RECORDING) {
        finish();
    }
    else if (state != IE_COMMAND_BUFFER_STATE_EXECUTABLE) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to execute a command buffer that is not executable!");
    }
    VkSubmitInfo submitInfo {
            .sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount=1,
            .pCommandBuffers=&commandBuffer
    };
    VkFence fence;
    VkFenceCreateInfo fenceCreateInfo {
            .sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
    };
    vkCreateFence(linkedRenderEngine->device.device, &fenceCreateInfo, nullptr, &fence);
    vkQueueSubmit(commandPool->queue, 1, &submitInfo, fence);
    state = IE_COMMAND_BUFFER_STATE_PENDING;
    new std::thread{[=] {
        vkWaitForFences(linkedRenderEngine->device.device, 1, &fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(linkedRenderEngine->device.device, fence, nullptr);
        state = oneTimeSubmission ? IE_COMMAND_BUFFER_STATE_INVALID : IE_COMMAND_BUFFER_STATE_EXECUTABLE;
        oneTimeSubmission = false;
    }};
}

void IECommandBuffer::recordPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const std::vector<VkMemoryBarrier>& memoryBarriers, const std::vector<IEBufferMemoryBarrier>& bufferMemoryBarriers, const std::vector<IEImageMemoryBarrier> &imageMemoryBarriers) {
    if (state == IE_COMMAND_BUFFER_STATE_INITIAL) {
        record();
    }
    else if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a pipeline barrier on a command buffer that is not recording!");
    }
    std::vector<VkBufferMemoryBarrier> bufferBarriers{};
    bufferBarriers.reserve(bufferMemoryBarriers.size());
    for (IEBufferMemoryBarrier bufferMemoryBarrier : bufferMemoryBarriers) {
        addDependency(bufferMemoryBarrier.getBuffer());
        bufferBarriers.push_back((VkBufferMemoryBarrier)bufferMemoryBarrier);
    }
    std::vector<VkImageMemoryBarrier> imageBarriers{};
    imageBarriers.reserve(imageMemoryBarriers.size());
    for (IEImageMemoryBarrier imageMemoryBarrier : imageMemoryBarriers) {
        addDependency(imageMemoryBarrier.getImage());
        imageBarriers.push_back((VkImageMemoryBarrier)imageMemoryBarrier);
    }
    vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarriers.size(), memoryBarriers.data(), bufferBarriers.size(), bufferBarriers.data(), imageBarriers.size(), imageBarriers.data());
}

void IECommandBuffer::recordPipelineBarrier(const IEDependencyInfo *dependencyInfo) {
    if (state == IE_COMMAND_BUFFER_STATE_INITIAL) {
        record();
    }
    else if (state != IE_COMMAND_BUFFER_STATE_RECORDING) {
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempt to record a pipeline barrier on a command buffer that is not recording!");
    }
    addDependencies(dependencyInfo->getDependencies());
    vkCmdPipelineBarrier2(commandBuffer, (const VkDependencyInfo *)dependencyInfo);
}

void IECommandBuffer::addImage(IEImage *image) {
    dependencies.emplace_back(image);
}

void IECommandBuffer::addBuffer(IEBuffer *buffer) {
    dependencies.emplace_back(buffer);
}
