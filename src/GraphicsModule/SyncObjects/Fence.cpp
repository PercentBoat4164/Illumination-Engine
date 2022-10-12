#include "Fence.hpp"

#include "RenderEngine.hpp"

#include <utility>

IE::Graphics::Fence::Fence(IE::Graphics::RenderEngine *t_engineLink, bool t_signaled) {
    create(t_engineLink, t_signaled);
}

IE::Graphics::Fence::~Fence() {
    std::unique_lock<std::mutex> lock(*fenceMutex);
    status = IE_FENCE_STATUS_INVALID;
    vkDestroyFence(linkedRenderEngine->getDevice(), fence, nullptr);
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

void IE::Graphics::Fence::create(IE::Graphics::RenderEngine *t_engineLink, bool t_signaled) {
    linkedRenderEngine = t_engineLink;
    fenceMutex         = std::make_shared<std::mutex>();
    VkFenceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = t_signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u,
    };
    std::unique_lock<std::mutex> lock(*fenceMutex);
    VkResult result{vkCreateFence(linkedRenderEngine->getDevice(), &createInfo, nullptr, &fence)};
    if (result != VK_SUCCESS)
        linkedRenderEngine->getLogger().log(
          "Failed to create Fence with error: " + IE::Graphics::RenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    else {
        status = IE_FENCE_STATUS_VALID;
        linkedRenderEngine->getLogger().log("Created Fence");
    }
}
