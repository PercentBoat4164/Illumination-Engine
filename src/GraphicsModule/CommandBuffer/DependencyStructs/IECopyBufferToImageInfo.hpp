#pragma once

#include <memory>
#include <vulkan/vulkan.h>
#include "Buffer/IEBuffer.hpp"
#include "Image/IEImage.hpp"

class IECopyBufferToImageInfo {
public:
    const void *pNext;
    std::shared_ptr<IEBuffer> srcBuffer;
    std::shared_ptr<IEImage> dstImage;
    std::vector<VkBufferImageCopy2> regions;

    [[nodiscard]] std::vector<std::shared_ptr<IEBuffer>> getBuffers() const;

    [[nodiscard]] std::vector<std::shared_ptr<IEImage>> getImages() const;

    [[nodiscard]] std::vector<std::shared_ptr<IEDependency>> getDependencies() const;

    explicit operator VkCopyBufferToImageInfo2() const;
};