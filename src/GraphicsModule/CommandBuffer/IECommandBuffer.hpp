#pragma once

class IECommandPool;

class IERenderEngine;

#include <vulkan/vulkan.h>
#include <vector>
#include <variant>
#include "Buffer/IEBuffer.hpp"
#include "Image/IEImage.hpp"
#include "IEDependent.hpp"


typedef enum IECommandBufferState {
    IE_COMMAND_BUFFER_STATE_NONE = 0x0,
    IE_COMMAND_BUFFER_STATE_INITIAL = 0x1,
    IE_COMMAND_BUFFER_STATE_RECORDING = 0x2,
    IE_COMMAND_BUFFER_STATE_EXECUTABLE = 0x3,
    IE_COMMAND_BUFFER_STATE_PENDING = 0x4,
    IE_COMMAND_BUFFER_STATE_INVALID = 0x5
} IECommandBufferState;


typedef struct IEImageMemoryBarrier {
    const void *pNext;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    VkImageLayout newLayout;
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;
    IEImage *image;
    VkImageSubresourceRange subresourceRange;

    [[nodiscard]] IEImage *getImage() const {
        return image;
    }

    explicit operator VkImageMemoryBarrier() const {
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

    explicit operator VkImageMemoryBarrier2() const {
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
} IEImageMemoryBarrier;


typedef struct IEBufferMemoryBarrier {
    const void *pNext;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;
    IEBuffer *buffer;
    VkDeviceSize offset;
    VkDeviceSize size;

    [[nodiscard]] IEBuffer *getBuffer() const {
        return buffer;
    };

    explicit operator VkBufferMemoryBarrier() const {
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

    explicit operator VkBufferMemoryBarrier2() const {
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

} IEBufferMemoryBarrier;

typedef struct IEDependencyInfo {
    const void *pNext;
    VkDependencyFlags dependencyFlags;
    std::vector<VkMemoryBarrier2> memoryBarriers;
    std::vector<IEBufferMemoryBarrier> bufferMemoryBarriers;
    std::vector<IEImageMemoryBarrier> imageMemoryBarriers;

    [[nodiscard]] std::vector<IEBuffer *> getBuffers() const {
        std::vector<IEBuffer *> buffers{};
        buffers.reserve(bufferMemoryBarriers.size());
        for (IEBufferMemoryBarrier bufferBarrier : bufferMemoryBarriers) {
            buffers.push_back(bufferBarrier.buffer);
        }
        return buffers;
    }

    [[nodiscard]] std::vector<IEImage *> getImages() const {
        std::vector<IEImage *> images{};
        images.reserve(imageMemoryBarriers.size());
        for (IEImageMemoryBarrier imageBarrier : imageMemoryBarriers) {
            images.push_back(imageBarrier.image);
        }
        return images;
    }

    [[nodiscard]] std::vector<IEDependency *> getDependencies() const {
        std::vector<IEDependency *> dependencies{};
        dependencies.reserve(imageMemoryBarriers.size() + bufferMemoryBarriers.size());
        for (IEImageMemoryBarrier imageBarrier : imageMemoryBarriers) {
            dependencies.push_back(imageBarrier.image);
        }
        for (IEBufferMemoryBarrier bufferBarrier : bufferMemoryBarriers) {
            dependencies.push_back(bufferBarrier.buffer);
        }
        return dependencies;
    }

    explicit operator VkDependencyInfo() {
        bufferBarriers.resize(bufferMemoryBarriers.size());
        for (IEBufferMemoryBarrier bufferBarrier : bufferMemoryBarriers) {
            bufferBarriers.emplace_back((VkBufferMemoryBarrier2)bufferBarrier);
        }
        imageBarriers.resize(imageMemoryBarriers.size());
        for (IEImageMemoryBarrier imageBarrier : imageMemoryBarriers) {
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

private:
    std::vector<VkBufferMemoryBarrier2> bufferBarriers{};
    std::vector<VkImageMemoryBarrier2> imageBarriers{};
} IEDependencyInfo;

typedef struct IECopyBufferToImageInfo {
    const void *pNext;
    IEBuffer *srcBuffer;
    IEImage *dstImage;
    std::vector<VkBufferImageCopy2> regions;

    [[nodiscard]] std::vector<IEImage *> getImages() const {
        return {dstImage};
    }

    [[nodiscard]] std::vector<IEBuffer *> getBuffers() const {
        return {srcBuffer};
    }

    [[nodiscard]] std::vector<IEDependency *> getDependencies() const {
        return {dstImage, srcBuffer};
    }

    explicit operator VkCopyBufferToImageInfo2() const {
        return {
                .sType=VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
                .pNext=pNext,
                .srcBuffer=srcBuffer->buffer,
                .dstImage=dstImage->image,
                .regionCount=static_cast<uint32_t>(regions.size()),
                .pRegions=regions.data()
        };
    }
} IECopyBufferToImageInfo;

class IECommandBuffer : public IEDependent {
public:
    VkCommandBuffer commandBuffer{};
    IECommandPool *commandPool;
    IERenderEngine *linkedRenderEngine;
    IECommandBufferState state;
    bool oneTimeSubmission{false};

    IECommandBuffer(IERenderEngine *linkedRenderEngine, IECommandPool *commandPool);

    void wait() override;

    /**
     * @brief Allocate this command buffer as a primary command buffer.
     */
    void allocate();

    /**
     * @brief Prepare this command buffer for recording.
     */
    void record(bool oneTimeSubmit=false);

    void free();

    void reset();

    void execute();

    void finish();

    void recordPipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, const std::vector<VkMemoryBarrier> &memoryBarriers, const std::vector<IEBufferMemoryBarrier> &bufferMemoryBarriers, const std::vector<IEImageMemoryBarrier> &imageMemoryBarriers);

    void recordPipelineBarrier(const IEDependencyInfo *dependencyInfo);

    void recordCopyBufferToImage(IEBuffer *buffer, IEImage *image, std::vector<VkBufferImageCopy> regions);

    void recordCopyBufferToImage(IECopyBufferToImageInfo *copyInfo);
};