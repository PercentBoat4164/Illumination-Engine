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
    void
    build(IE::Graphics::RenderPass *t_renderPass, std::vector<IE::Graphics::Image::Preset> &t_attachmentPresets);
};
}  // namespace IE::Graphics