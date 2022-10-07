#include "Fence.hpp"

#include "RenderEngine.hpp"

#include <utility>

IE::Graphics::Fence::Fence(std::weak_ptr<IE::Graphics::RenderEngine> t_engineLink, bool signaled) :
        linkedRenderEngine(std::move(t_engineLink)) {
    VkFenceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0u,
    };
    std::unique_lock<std::mutex> lock(*fenceMutex);
    vkCreateFence(linkedRenderEngine.lock()->getDevice(), &createInfo, nullptr, &fence);
    linkedRenderEngine.lock()->getLogger().log("Created Fence");
}