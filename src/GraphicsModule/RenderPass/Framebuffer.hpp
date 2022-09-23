#pragma once

#include "RenderEngine.hpp"

#include <memory>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class Framebuffer {
public:
    VkFramebuffer m_framebuffer;

    std::weak_ptr<std::atomic<IE::Graphics::RenderEngine *>> linkedRenderEngine;
};
}  // namespace IE::Graphics