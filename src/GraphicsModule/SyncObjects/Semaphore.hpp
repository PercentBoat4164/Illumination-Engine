#pragma once

#include <atomic>
#include <future>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class RenderEngine;
}  // namespace IE::Graphics

namespace IE::Graphics {
class Semaphore {
public:
    enum Status {
        IE_SEMAPHORE_STATUS_INVALID = 0x0,
        IE_SEMAPHORE_STATUS_VALID   = 0x1,
        IE_SEMAPHORE_STATUS_WAITING = 0x2,
    };

    IE::Graphics::RenderEngine *linkedRenderEngine;
    VkSemaphore                 semaphore{};
    std::shared_ptr<std::mutex> semaphoreMutex;
    std::atomic<Status>         status{IE_SEMAPHORE_STATUS_INVALID};

    explicit Semaphore(IE::Graphics::RenderEngine *t_engineLink);

    Semaphore(const IE::Graphics::Semaphore &t_other);

    Semaphore() = default;

    ~Semaphore();

    void create(IE::Graphics::RenderEngine *t_engineLink);

    std::future<void> wait();

    void blocking_wait();
};
}  // namespace IE::Graphics