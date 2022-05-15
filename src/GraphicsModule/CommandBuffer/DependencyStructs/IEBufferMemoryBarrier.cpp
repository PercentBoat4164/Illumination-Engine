#include <vulkan/vulkan.h>
#include "IEBufferMemoryBarrier.hpp"

std::vector<std::shared_ptr<IEBuffer>> IEBufferMemoryBarrier::getBuffers() const {
    return {buffer};
}

std::vector<std::shared_ptr<IEDependency>> IEBufferMemoryBarrier::getDependencies() const {
    return {buffer};
}

IEBufferMemoryBarrier::operator VkBufferMemoryBarrier() const {
    return {
            .sType=VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .pNext=pNext,
            .srcAccessMask=srcAccessMask,
            .dstAccessMask=dstAccessMask,
            .srcQueueFamilyIndex=srcQueueFamilyIndex,
            .dstQueueFamilyIndex=dstQueueFamilyIndex,
            .buffer=buffer->buffer,
            .offset=offset,
            .size=size
    };
}

IEBufferMemoryBarrier::operator VkBufferMemoryBarrier2() const {
    return {
            .sType=VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .pNext=pNext,
            .srcAccessMask=srcAccessMask,
            .dstAccessMask=dstAccessMask,
            .srcQueueFamilyIndex=srcQueueFamilyIndex,
            .dstQueueFamilyIndex=dstQueueFamilyIndex,
            .buffer=buffer->buffer,
            .offset=offset,
            .size=size
    };
}
