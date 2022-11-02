#pragma once

#include "GraphicsModule/RenderPass/Framebuffer.hpp"
#include "GraphicsModule/RenderPass/Subpass.hpp"

#include <map>
#include <memory>
#include <vector>

namespace IE::Graphics {
class RenderPassSeries;

class RenderPass {
public:
    enum Preset {
        IE_RENDER_PASS_PRESET_CUSTOM = 0x0,
    };

    Preset                             m_preset;
    IE::Graphics::RenderEngine        *m_linkedRenderEngine{};
    VkRenderPass                       m_renderPass{};
    IE::Graphics::Framebuffer          m_framebuffer;
    std::vector<IE::Graphics::Subpass> m_subpasses;

    explicit RenderPass(Preset t_preset);

    auto addSubpass(IE::Graphics::Subpass &t_subpass) -> decltype(*this) {
        m_subpasses.push_back(t_subpass);
        return *this;
    }
};
}  // namespace IE::Graphics