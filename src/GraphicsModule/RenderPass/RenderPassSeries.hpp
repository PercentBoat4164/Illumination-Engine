#pragma once

#include "Image/Image.hpp"
#include "RenderPass.hpp"

#include <vector>

namespace IE::Graphics {
class RenderPassSeries {
public:
    explicit RenderPassSeries(IE::Graphics::RenderEngine *t_engineLink);

    auto specifyAttachments(const std::vector<std::pair<std::string, Image::Preset>> &t_attachments)
      -> decltype(*this) {
        m_attachmentPool.resize(m_attachmentPool.size() + t_attachments.size());
        for (const auto &attachment : t_attachments) m_attachmentPool.push_back(attachment);
        return *this;
    }

    auto build() -> decltype(*this);

    std::vector<std::pair<std::string, Image::Preset>> m_attachmentPool;
    std::vector<IE::Graphics::RenderPass>              m_renderPasses;
    IE::Graphics::RenderEngine                        *m_linkedRenderEngine;

    auto                                               addRenderPass(RenderPass &t_pass) -> decltype(*this);
};
}  // namespace IE::Graphics