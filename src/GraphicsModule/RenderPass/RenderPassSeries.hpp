#pragma once

#include "GraphicsModule/Image/AttachmentVulkan.hpp"
#include "RenderPass.hpp"

#include <vector>

namespace IE::Graphics {
class RenderPassSeries {
public:
    RenderPassSeries(IE::Graphics::RenderEngine *t_engineLink);

    auto addRenderPass(IE::Graphics::RenderPass::Presets t_preset) -> decltype(*this) {
        m_renderPasses.emplace_back(m_linkedRenderEngine, t_preset);
        return *this;
    }

    void build() {
    }

private:
    std::vector<IE::Graphics::RenderPass>                                   m_renderPasses;
    IE::Graphics::RenderEngine                                             *m_linkedRenderEngine;
    std::unordered_map<std::string, IE::Graphics::detail::AttachmentVulkan> m_attachments;
};
}  // namespace IE::Graphics