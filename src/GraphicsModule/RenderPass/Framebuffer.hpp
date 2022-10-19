#pragma once

#include <memory>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class RenderEngine;

class Framebuffer {
public:
    VkFramebuffer m_framebuffer;

    IE::Graphics::RenderEngine *linkedRenderEngine;
};
}  // namespace IE::Graphics