#pragma once

#include <atomic>
#include <memory>
#include <mutex>
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

    IE::Graphics::RenderEngine *linkedRenderEngine{};
    VkFence                     fence{};
    std::shared_ptr<std::mutex> fenceMutex{};
    std::atomic<Status>         status{IE_FENCE_STATUS_INVALID};

    explicit Fence(IE::Graphics::RenderEngine *t_engineLink, bool t_signaled = true);

    Fence(const IE::Graphics::Fence &);

    Fence() = default;

    void create(IE::Graphics::RenderEngine *t_engineLink, bool t_signaled = true);

    ~Fence();
};
}  // namespace IE::Graphics
