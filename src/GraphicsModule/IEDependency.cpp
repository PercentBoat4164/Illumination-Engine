#include <algorithm>
#include "IEDependency.hpp"

#include "IEDependent.hpp"

#include "Image/IEImage.hpp"
#include "Buffer/IEBuffer.hpp"
#include "IEPipeline.hpp"
#include "IEDescriptorSet.hpp"
#include "IERenderPass.hpp"

void IEDependency::addDependent(IEDependent *dependent) {
    if (!isDependencyOf(dependent)) {
        dependents.push_back(dependent);
    }
    if (!dependent->isDependentOn(this)) {
        dependent->addDependency(this);
    }
}

bool IEDependency::isDependencyOf(IEDependent *dependent) {
    return std::any_of(dependents.begin(), dependents.end(), [dependent](IEDependent *thisDependent) { return thisDependent == dependent; });
}

bool IEDependency::hasNoDependents() const {
    return dependents.empty();
}

void IEDependency::removeDependent(IEDependent *dependent) {
    dependents.erase(std::remove(dependents.begin(), dependents.end(), dependent), dependents.end());
    if (dependent->isDependentOn(this)) {
        dependent->removeDependency(this);
    }
}

void IEDependency::removeAllDependents() {
    for (IEDependent *dependent : dependents) {
        dependent->removeDependency(this);
    }
}

void IEDependency::wait() {
    for (IEDependent *dependent : dependents) {
        dependent->wait();
    }
}

void IEDependency::destroy() {
    removeAllDependents();
}

IEDependency::~IEDependency() {
    destroy();
}

std::vector<IEImage *> IEImageMemoryBarrier::getImages() const {
    return {image};
}

IEImageMemoryBarrier::operator VkImageMemoryBarrier() const {
    return {
            .sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext=pNext,
            .srcAccessMask=srcAccessMask,
            .dstAccessMask=dstAccessMask,
            .newLayout=newLayout,
            .srcQueueFamilyIndex=srcQueueFamilyIndex,
            .dstQueueFamilyIndex=dstQueueFamilyIndex,
            .image=image->image,
            .subresourceRange=subresourceRange
    };
}

IEImageMemoryBarrier::operator VkImageMemoryBarrier2() const {
    return {
            .sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext=pNext,
            .srcAccessMask=srcAccessMask,
            .dstAccessMask=dstAccessMask,
            .newLayout=newLayout,
            .srcQueueFamilyIndex=srcQueueFamilyIndex,
            .dstQueueFamilyIndex=dstQueueFamilyIndex,
            .image=image->image,
            .subresourceRange=subresourceRange
    };
}

std::vector<IEDependency *> IEImageMemoryBarrier::getDependencies() const {
    return {};
}

std::vector<IERenderPass *> IEImageMemoryBarrier::getRenderPasses() const {
    return {};
}

std::vector<IEDescriptorSet *> IEImageMemoryBarrier::getDescriptorSets() const {
    return {};
}

std::vector<IEPipeline *> IEImageMemoryBarrier::getPipelines() const {
    return {};
}

std::vector<IEBuffer *> IEImageMemoryBarrier::getBuffers() const {
    return {};
}

std::vector<IEBuffer *> IEBufferMemoryBarrier::getBuffers() const {
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

std::vector<IEImage *> IEBufferMemoryBarrier::getImages() const {
    return {};
}

std::vector<IEPipeline *> IEBufferMemoryBarrier::getPipelines() const {
    return {};
}

std::vector<IEDescriptorSet *> IEBufferMemoryBarrier::getDescriptorSets() const {
    return {};
}

std::vector<IERenderPass *> IEBufferMemoryBarrier::getRenderPasses() const {
    return {};
}

std::vector<IEDependency *> IEBufferMemoryBarrier::getDependencies() const {
    return {};
}

std::vector<IEBuffer *> IEDependencyInfo::getBuffers() const {
    std::vector<IEBuffer *> buffers{};
    buffers.reserve(bufferMemoryBarriers.size());
    for (const IEBufferMemoryBarrier& bufferBarrier : bufferMemoryBarriers) {
        buffers.push_back(bufferBarrier.buffer);
    }
    return buffers;
}

std::vector<IEImage *> IEDependencyInfo::getImages() const {
    std::vector<IEImage *> images{};
    images.reserve(imageMemoryBarriers.size());
    for (const IEImageMemoryBarrier& imageBarrier : imageMemoryBarriers) {
        images.push_back(imageBarrier.image);
    }
    return images;
}

IEDependencyInfo::operator VkDependencyInfo() {
    bufferBarriers.resize(bufferMemoryBarriers.size());
    for (const IEBufferMemoryBarrier& bufferBarrier : bufferMemoryBarriers) {
        bufferBarriers.emplace_back((VkBufferMemoryBarrier2)bufferBarrier);
    }
    imageBarriers.resize(imageMemoryBarriers.size());
    for (const IEImageMemoryBarrier& imageBarrier : imageMemoryBarriers) {
        imageBarriers.emplace_back((VkImageMemoryBarrier2)imageBarrier);
    }
    return {
            .sType=VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .pNext=pNext,
            .dependencyFlags=dependencyFlags,
            .memoryBarrierCount=static_cast<uint32_t>(memoryBarriers.size()),
            .pMemoryBarriers=memoryBarriers.data(),
            .bufferMemoryBarrierCount=static_cast<uint32_t>(bufferMemoryBarriers.size()),
            .pBufferMemoryBarriers=bufferBarriers.data(),
            .imageMemoryBarrierCount=static_cast<uint32_t>(imageMemoryBarriers.size()),
            .pImageMemoryBarriers=imageBarriers.data(),
    };
}

std::vector<IEPipeline *> IEDependencyInfo::getPipelines() const {
    return {};
}

std::vector<IEDescriptorSet *> IEDependencyInfo::getDescriptorSets() const {
    return {};
}

std::vector<IERenderPass *> IEDependencyInfo::getRenderPasses() const {
    return {};
}

std::vector<IEDependency *> IEDependencyInfo::getDependencies() const {
    return {};
}

std::vector<IEImage *> IECopyBufferToImageInfo::getImages() const {
    return {dstImage};
}

std::vector<IEBuffer *> IECopyBufferToImageInfo::getBuffers() const {
    return {srcBuffer};
}

IECopyBufferToImageInfo::operator VkCopyBufferToImageInfo2() const {
    return {
            .sType=VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
            .pNext=pNext,
            .srcBuffer=srcBuffer->buffer,
            .dstImage=dstImage->image,
            .regionCount=static_cast<uint32_t>(regions.size()),
            .pRegions=regions.data()
    };
}

std::vector<IEDependency *> IECopyBufferToImageInfo::getDependencies() const {
    return {};
}

std::vector<IERenderPass *> IECopyBufferToImageInfo::getRenderPasses() const {
    return {};
}

std::vector<IEDescriptorSet *> IECopyBufferToImageInfo::getDescriptorSets() const {
    return {};
}

std::vector<IEPipeline *> IECopyBufferToImageInfo::getPipelines() const {
    return {};
}

IERenderPassBeginInfo::operator VkRenderPassBeginInfo() const {
    return {
            .sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext=pNext,
            .renderPass=renderPass->renderPass,
            .framebuffer=framebuffer->framebuffer,
            .renderArea=renderArea,
            .clearValueCount=clearValueCount,
            .pClearValues=pClearValues
    };
}

std::vector<IEImage *> IERenderPassBeginInfo::getImages() const {
    return {framebuffer};
}

std::vector<IERenderPass *> IERenderPassBeginInfo::getRenderPasses() const {
    return {renderPass};
}

std::vector<IEDependency *> IERenderPassBeginInfo::getDependencies() const {
    return {};
}

std::vector<IEDescriptorSet *> IERenderPassBeginInfo::getDescriptorSets() const {
    return {};
}

std::vector<IEPipeline *> IERenderPassBeginInfo::getPipelines() const {
    return {};
}

std::vector<IEBuffer *> IERenderPassBeginInfo::getBuffers() const {
    return {};
}
