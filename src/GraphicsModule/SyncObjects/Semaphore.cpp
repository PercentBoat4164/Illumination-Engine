#include "Semaphore.hpp"

#include "RenderEngine.hpp"

IE::Graphics::Semaphore::Semaphore(std::weak_ptr<IE::Graphics::RenderEngine> t_engineLink) :
        linkedRenderEngine(std::move(t_engineLink)) {
    VkSemaphoreCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
    };
    vkCreateSemaphore(linkedRenderEngine.lock()->getDevice(), &createInfo, nullptr, &semaphore);
    linkedRenderEngine.lock()->getLogger().log("Created Fence");
}

std::future<void> IE::Graphics::Semaphore::wait() {
    std::packaged_task<void()> wait{[&] {
        blocking_wait();
    }};
    linkedRenderEngine.lock()->getCore()->threadPool.submit([&] { wait(); });
    return wait.get_future();
}

void IE::Graphics::Semaphore::blocking_wait() {
    VkSemaphoreWaitInfo waitInfo{
      .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
      .pNext          = nullptr,
      .flags          = 0,
      .semaphoreCount = 1,
      .pSemaphores    = &semaphore,
      .pValues        = nullptr,
    };

    // The waiting process can happen on multiple threads at once, but destruction cannot happen while waiting is
    // happening. Therefore, this synchronization does not the waiting if the lock cannot be gotten, but does
    // ensure that the mutex is locked when waiting.
    bool locked{semaphoreMutex->try_lock()};
    linkedRenderEngine.lock()->getLogger().log("Waiting for Fence");
    vkWaitSemaphores(linkedRenderEngine.lock()->getDevice(), &waitInfo, std::numeric_limits<uint64_t>::max());
    if (locked) semaphoreMutex->unlock();
}