#pragma once

#include "GraphicsModule/CommandBuffer/CommandBuffer.hpp"
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
    IE::Graphics::RenderPassSeries    *m_renderPassSeries{};
    VkRenderPass                       m_renderPass{};
    size_t                             m_currentPass;
    std::vector<IE::Graphics::Subpass> m_subpasses;

    explicit RenderPass(Preset t_preset);

    auto addSubpass(IE::Graphics::Subpass &t_subpass) -> decltype(*this) {
        m_subpasses.push_back(t_subpass);
        return *this;
    }

    void build(
      RenderPassSeries                     *t_renderPassSeries,
      std::vector<VkAttachmentDescription> &t_attachmentDescriptions,
      std::vector<VkSubpassDescription>    &t_subpassDescriptions,
      std::vector<VkSubpassDependency>     &t_subpassDependency
    );

    bool nextPass(std::shared_ptr<IE::Graphics::CommandBuffer> masterCommandBuffer) {
        if (m_currentPass < m_subpasses.size()) {}
        return true;
    }

    bool start(std::shared_ptr<CommandBuffer> masterCommandBuffer, Framebuffer framebuffer);
};
}  // namespace IE::Graphics