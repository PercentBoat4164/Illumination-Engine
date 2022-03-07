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

IEBuffer *IECommandBuffer::addDependency(const IEBuffer &dependency) {
    dependencies.emplace_back(dependency);
    return &std::get<IEBuffer>(dependencies[dependencies.size() - 1]);
}

IEImage *IECommandBuffer::addDependency(const IEImage &dependency) {
    dependencies.emplace_back(dependency);
    return &std::get<IEImage>(dependencies[dependencies.size() - 1]);
}

IEPipeline *IECommandBuffer::addDependency(const IEPipeline &dependency) {
    dependencies.emplace_back(dependency);
    return &std::get<IEPipeline>(dependencies[dependencies.size() - 1]);
}
