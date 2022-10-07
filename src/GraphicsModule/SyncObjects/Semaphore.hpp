#pragma once

#include <future>
#include <mutex>
#include <utility>
#include <memory>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class RenderEngine;
}  // namespace IE::Graphics

namespace IE::Graphics {
class Semaphore {
public:
    std::weak_ptr<IE::Graphics::RenderEngine> linkedRenderEngine;
    VkSemaphore                               semaphore{};
    std::shared_ptr<std::mutex>               semaphoreMutex;

    explicit Semaphore(std::weak_ptr<IE::Graphics::RenderEngine> t_engineLink);

    std::future<void> wait();

    void blocking_wait();
};
}  // namespace IE::Graphics