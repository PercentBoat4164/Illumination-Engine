#pragma once

#include <atomic>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class RenderEngine;
}  // namespace IE::Graphics

namespace IE::Graphics {
class Fence {
public:
    enum Status {
        IE_FENCE_STATUS_INVALID = 0x0,
        IE_FENCE_STATUS_VALID   = 0x1,
        IE_FENCE_STATUS_WAITING = 0x2,
    };

    std::weak_ptr<IE::Graphics::RenderEngine> linkedRenderEngine;
    VkFence                                   fence{};
    std::shared_ptr<std::mutex>               fenceMutex{};
    std::atomic<Status>                       status{IE_FENCE_STATUS_INVALID};

    explicit Fence(std::weak_ptr<IE::Graphics::RenderEngine> weakPtr, bool signaled=false);

    Fence(const IE::Graphics::Fence &);

    Fence() = default;

    void create(std::weak_ptr<IE::Graphics::RenderEngine> weakPtr, bool signaled=false);

    ~Fence();
};
}  // namespace IE::Graphics