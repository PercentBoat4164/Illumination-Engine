#pragma once

#include "IEBufferMemoryBarrier.hpp"
#include "IEImageMemoryBarrier.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan.h>

class IEDependencyInfo {
public:
    const void                        *pNext;
    VkDependencyFlags                  dependencyFlags;
    std::vector<VkMemoryBarrier2>      memoryBarriers;
    std::vector<IEBufferMemoryBarrier> bufferMemoryBarriers;
    std::vector<IEImageMemoryBarrier>  imageMemoryBarriers;

    [[nodiscard]] std::vector<std::shared_ptr<IEBuffer>> getBuffers() const;

    [[nodiscard]] std::vector<std::shared_ptr<IE::Graphics::Image>> getImages() const;

    explicit operator VkDependencyInfo();

private:
    std::vector<VkBufferMemoryBarrier2> bufferBarriers{};
    std::vector<VkImageMemoryBarrier2>  imageBarriers{};
};