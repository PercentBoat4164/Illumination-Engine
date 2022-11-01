#pragma once

#include "Image/Image.hpp"
#include "RenderPass.hpp"

#include <vector>

namespace IE::Graphics {
class RenderPassSeries {
public:
    explicit RenderPassSeries(IE::Graphics::RenderEngine *t_engineLink);

    auto build() -> decltype(*this);

    auto addRenderPass(RenderPass &t_pass) -> decltype(*this);

    std::vector<std::pair<std::string, Image::Preset>> m_attachmentPool;
    std::vector<IE::Graphics::RenderPass>              m_renderPasses;
    IE::Graphics::RenderEngine                        *m_linkedRenderEngine;
};
}  // namespace IE::Graphics