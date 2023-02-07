#include "Semaphore.hpp"

#include "RenderEngine.hpp"

IE::Graphics::Semaphore::Semaphore(IE::Graphics::RenderEngine *t_engineLink) {
    create(t_engineLink);
}

std::future<void> IE::Graphics::Semaphore::wait() {
    std::packaged_task<void()> wait{[&] {
        blocking_wait();
    }};
    IE::Core::Core::getInst().getThreadPool()->submit([&] { wait(); });
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
    linkedRenderEngine->getLogger().log("Waiting for Semaphore");
    status = IE_SEMAPHORE_STATUS_WAITING;
    vkWaitSemaphores(linkedRenderEngine->m_device.device, &waitInfo, std::numeric_limits<uint64_t>::max());
    status = IE_SEMAPHORE_STATUS_VALID;
    if (locked) semaphoreMutex->unlock();
}

IE::Graphics::Semaphore::~Semaphore() {
    std::lock_guard<std::mutex> lock(*semaphoreMutex);
    status = IE_SEMAPHORE_STATUS_INVALID;
    vkDestroySemaphore(linkedRenderEngine->m_device.device, semaphore, nullptr);
}

IE::Graphics::Semaphore::Semaphore(const IE::Graphics::Semaphore &t_other) {
    if (this != &t_other) {
        std::scoped_lock<std::mutex, std::mutex> lock(*semaphoreMutex, *t_other.semaphoreMutex);
        status.store(t_other.status.load());
        semaphore          = t_other.semaphore;
        linkedRenderEngine = t_other.linkedRenderEngine;
    }
}

void IE::Graphics::Semaphore::create(IE::Graphics::RenderEngine *t_engineLink) {
    linkedRenderEngine = t_engineLink;
    semaphoreMutex     = std::make_shared<std::mutex>();
    VkSemaphoreCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
    };
    VkResult result{vkCreateSemaphore(linkedRenderEngine->m_device.device, &createInfo, nullptr, &semaphore)};
    if (result != VK_SUCCESS)
        linkedRenderEngine->getLogger().log(
          "Failed to create Fence with error: " + IE::Graphics::RenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    else {
        status = IE_SEMAPHORE_STATUS_VALID;
        linkedRenderEngine->getLogger().log("Created Semaphore");
    }
}
