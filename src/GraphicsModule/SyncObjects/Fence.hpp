#pragma once

#include <memory>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class RenderEngine;
}  // namespace IE::Graphics

namespace IE::Graphics {
class Fence {
    std::weak_ptr<IE::Graphics::RenderEngine> linkedRenderEngine;
    VkFence                                   fence{};
    std::shared_ptr<std::mutex>               fenceMutex;

public:
    explicit Fence(std::weak_ptr<IE::Graphics::RenderEngine> weakPtr, bool signaled);
};
}  // namespace IE::Graphics