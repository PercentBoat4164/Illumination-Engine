#pragma once

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class RenderEngine;
class RenderPass;
class Image;

class Framebuffer {
public:
    size_t                              m_resolution[2];
    VkFramebuffer                       m_framebuffer;
    IE::Graphics::RenderPass           *renderPass;
    std::vector<std::shared_ptr<Image>> attachments{};
    IE::Graphics::RenderEngine         *m_linkedRenderEngine;
};
}  // namespace IE::Graphics