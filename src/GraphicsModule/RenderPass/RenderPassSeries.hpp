#pragma once

#include "Image/Image.hpp"
#include "RenderPass.hpp"

#include <vector>

namespace IE::Graphics {
class RenderPassSeries {
public:
    enum ProgressionStatus {
        IE_RENDER_PASS_SERIES_PROGRESSION_STATUS_END,
        IE_RENDER_PASS_SERIES_PROGRESSION_STATUS_CONTINUE,
    };

    explicit RenderPassSeries(IE::Graphics::RenderEngine *t_engineLink);

    auto build() -> decltype(*this);

    auto addRenderPass(RenderPass &t_pass) -> decltype(*this);

    bool start();

    void finish();

    ProgressionStatus nextPass();

    std::vector<std::pair<std::string, Image::Preset>> m_attachmentPool;
    size_t                                             m_currentPass{};
    std::vector<IE::Graphics::RenderPass>              m_renderPasses;
    IE::Graphics::RenderEngine                        *m_linkedRenderEngine;
    std::shared_ptr<IE::Graphics::CommandBuffer>       m_masterCommandBuffer;

    void execute(std::vector<VkCommandBuffer> t_commandBuffers);

private:
    std::tuple<
      std::vector<std::vector<VkAttachmentDescription>>,
      std::vector<std::vector<IE::Graphics::Image::Preset>>>
                                                   buildAttachmentDescriptions();
    std::vector<std::vector<VkSubpassDescription>> buildSubpassDescriptions();
    std::vector<std::vector<VkSubpassDependency>>  buildSubpassDependencies();
};
}  // namespace IE::Graphics