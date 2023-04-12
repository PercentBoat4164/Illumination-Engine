#pragma once

#include "GraphicsModule/Image/Image.hpp"

#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class RenderEngine;
class RenderPass;

class Framebuffer {
public:
    size_t                              m_resolution[2];
    VkFramebuffer                       m_framebuffer;
    IE::Graphics::RenderPass           *m_renderPass;
    std::vector<std::shared_ptr<Image>> attachments{};
    IE::Graphics::RenderEngine         *m_linkedRenderEngine;

    void

    build(IE::Graphics::RenderPass *t_renderPass, std::vector<IE::Graphics::Image::Preset> &t_attachmentPresets);

    void destroy();

    ~Framebuffer();
};
}  // namespace IE::Graphics