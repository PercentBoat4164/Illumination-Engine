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

    bool start(size_t frameNumber);

    bool finish();

    bool nextPass();

    std::vector<std::pair<std::string, Image::Preset>> m_attachmentPool;
    size_t                                             m_currentPass;
    std::vector<IE::Graphics::RenderPass>              m_renderPasses;
    IE::Graphics::Framebuffer                          m_framebuffer;
    IE::Graphics::RenderEngine                        *m_linkedRenderEngine;
    std::shared_ptr<IE::Graphics::CommandBuffer>       m_masterCommandBuffer;

private:
    std::vector<std::vector<VkAttachmentDescription>> buildAttachmentDescriptions();
    std::vector<std::vector<VkSubpassDescription>>    buildSubpassDescriptions();
    std::vector<std::vector<VkSubpassDependency>>     buildSubpassDependencies();
};
}  // namespace IE::Graphics