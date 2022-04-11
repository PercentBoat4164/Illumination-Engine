#pragma once

class IEDependent;

class IEImage;

class IEFramebuffer;

class IEBuffer;

class IEPipeline;

class IEDescriptorSet;

class IERenderPass;

#include <vector>
#include <vulkan/vulkan.h>

class IEDependency {
protected:
    std::vector<IEDependent*> dependents{};

public:
    void addDependent(IEDependent *dependent);

    bool isDependencyOf(IEDependent *dependent);

    void removeDependent(IEDependent *dependent);

    void removeAllDependents();

    bool hasNoDependents();

    virtual ~IEDependency() = 0;

    void wait();
};


typedef struct IEImageMemoryBarrier {
    const void *pNext;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    VkImageLayout newLayout;
    uint32_t srcQueueFamilyIndex;
    uint32_t dstQueueFamilyIndex;
    IEImage *image;
    VkImageSubresourceRange subresourceRange;

    [[nodiscard]] std::vector<IEBuffer*> getBuffers() const;;

    [[nodiscard]] std::vector<IEImage*> getImages() const;

    [[nodiscard]] std::vector<IEPipeline*> getPipelines() const;

    [[nodiscard]] std::vector<IEDescriptorSet*> getDescriptorSets() const;

    [[nodiscard]] std::vector<IERenderPass*> getRenderPasses() const;

    [[nodiscard]] std::vector<IEDependency*> getDependencies() const;

    explicit operator VkImageMemoryBarrier() const;

    explicit operator VkImageMemoryBarrier2() const;
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

    [[nodiscard]] std::vector<IEBuffer*> getBuffers() const;;

    [[nodiscard]] std::vector<IEImage*> getImages() const;

    [[nodiscard]] std::vector<IEPipeline*> getPipelines() const;

    [[nodiscard]] std::vector<IEDescriptorSet*> getDescriptorSets() const;

    [[nodiscard]] std::vector<IERenderPass*> getRenderPasses() const;

    [[nodiscard]] std::vector<IEDependency*> getDependencies() const;

    explicit operator VkBufferMemoryBarrier() const;

    explicit operator VkBufferMemoryBarrier2() const;

} IEBufferMemoryBarrier;

typedef struct IEDependencyInfo {
    const void *pNext;
    VkDependencyFlags dependencyFlags;
    std::vector<VkMemoryBarrier2> memoryBarriers;
    std::vector<IEBufferMemoryBarrier> bufferMemoryBarriers;
    std::vector<IEImageMemoryBarrier> imageMemoryBarriers;

    [[nodiscard]] std::vector<IEBuffer*> getBuffers() const;;

    [[nodiscard]] std::vector<IEImage*> getImages() const;

    [[nodiscard]] std::vector<IEPipeline*> getPipelines() const;

    [[nodiscard]] std::vector<IEDescriptorSet*> getDescriptorSets() const;

    [[nodiscard]] std::vector<IERenderPass*> getRenderPasses() const;

    [[nodiscard]] std::vector<IEDependency*> getDependencies() const;

    explicit operator VkDependencyInfo();

private:
    std::vector<VkBufferMemoryBarrier2> bufferBarriers{};
    std::vector<VkImageMemoryBarrier2> imageBarriers{};
} IEDependencyInfo;

typedef struct IECopyBufferToImageInfo {
    const void *pNext;
    IEBuffer *srcBuffer;
    IEImage *dstImage;
    std::vector<VkBufferImageCopy2> regions;

    [[nodiscard]] std::vector<IEBuffer*> getBuffers() const;;

    [[nodiscard]] std::vector<IEImage*> getImages() const;

    [[nodiscard]] std::vector<IEPipeline*> getPipelines() const;

    [[nodiscard]] std::vector<IEDescriptorSet*> getDescriptorSets() const;

    [[nodiscard]] std::vector<IERenderPass*> getRenderPasses() const;

    [[nodiscard]] std::vector<IEDependency*> getDependencies() const;

    explicit operator VkCopyBufferToImageInfo2() const;
} IECopyBufferToImageInfo;

typedef struct IERenderPassBeginInfo {
    const void *pNext;
    IERenderPass *renderPass;
    IEFramebuffer *framebuffer;
    VkRect2D renderArea{};
    uint32_t clearValueCount;
    const VkClearValue *pClearValues;

    [[nodiscard]] std::vector<IEBuffer*> getBuffers() const;;

    [[nodiscard]] std::vector<IEImage*> getImages() const;

    [[nodiscard]] std::vector<IEPipeline*> getPipelines() const;

    [[nodiscard]] std::vector<IEDescriptorSet*> getDescriptorSets() const;

    [[nodiscard]] std::vector<IERenderPass*> getRenderPasses() const;

    [[nodiscard]] std::vector<IEDependency*> getDependencies() const;

    explicit operator VkRenderPassBeginInfo() const;
} IERenderPassBeginInfo;