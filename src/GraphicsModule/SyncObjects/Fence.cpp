#include "Fence.hpp"

#include "RenderEngine.hpp"

#include <utility>

IE::Graphics::Fence::Fence(std::weak_ptr<IE::Graphics::RenderEngine> t_engineLink, bool t_signaled) {
    create(t_engineLink, t_signaled);
}

IE::Graphics::Fence::~Fence() {
    std::unique_lock<std::mutex> lock(*fenceMutex);
    status = IE_FENCE_STATUS_INVALID;
    vkDestroyFence(linkedRenderEngine.lock()->getDevice(), fence, nullptr);
}

IE::Graphics::Fence::Fence(const IE::Graphics::Fence &t_other) {
    if (this != &t_other) {
        std::unique_lock<std::mutex> lock(*fenceMutex);
        std::unique_lock<std::mutex> otherLock(*t_other.fenceMutex);
        status.store(t_other.status.load());
        fence              = t_other.fence;
        linkedRenderEngine = t_other.linkedRenderEngine;
    }
}

void IE::Graphics::Fence::create(std::weak_ptr<IE::Graphics::RenderEngine> t_engineLink, bool t_signaled) {
    linkedRenderEngine = t_engineLink;
    fenceMutex         = std::make_shared<std::mutex>();
    VkFenceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = t_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u,
    };
    std::unique_lock<std::mutex> lock(*fenceMutex);
    vkCreateFence(linkedRenderEngine.lock()->getDevice(), &createInfo, nullptr, &fence);
    status = IE_FENCE_STATUS_VALID;
    linkedRenderEngine.lock()->getLogger().log("Created Fence");
}
