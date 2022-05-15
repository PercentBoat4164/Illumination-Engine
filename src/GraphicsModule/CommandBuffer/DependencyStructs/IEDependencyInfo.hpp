#pragma once

#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include "IEBufferMemoryBarrier.hpp"
#include "IEImageMemoryBarrier.hpp"

class IEDependencyInfo {
public:
    const void *pNext;
    VkDependencyFlags dependencyFlags;
    std::vector<VkMemoryBarrier2> memoryBarriers;
    std::vector<IEBufferMemoryBarrier> bufferMemoryBarriers;
    std::vector<IEImageMemoryBarrier> imageMemoryBarriers;

    [[nodiscard]] std::vector<std::shared_ptr<IEBuffer>> getBuffers() const;

    [[nodiscard]] std::vector<std::shared_ptr<IEImage>> getImages() const;

    [[nodiscard]] std::vector<std::shared_ptr<IEDependency>> getDependencies() const;

    explicit operator VkDependencyInfo();

private:
    std::vector<VkBufferMemoryBarrier2> bufferBarriers{};
    std::vector<VkImageMemoryBarrier2> imageBarriers{};
};