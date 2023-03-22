#pragma once

#include "Core/Core.hpp"
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

    auto addRenderPass(const std::shared_ptr<RenderPass> &t_pass) -> decltype(*this);

    std::vector<std::pair<std::string, Image::Preset>>     m_attachmentPool;
    size_t                                                 m_currentPass{};
    std::vector<std::shared_ptr<IE::Graphics::RenderPass>> m_renderPasses;
    IE::Graphics::RenderEngine                            *m_linkedRenderEngine;
    std::shared_ptr<IE::Graphics::CommandBuffer>           m_masterCommandBuffer;

    Core::Threading::CoroutineTask<void> execute(std::shared_ptr<CommandBuffer> commandBuffer);

    void destroy() {
        for (auto &renderPass : m_renderPasses) renderPass->destroy();
        if (m_masterCommandBuffer) m_masterCommandBuffer->free();
    }

private:
    std::tuple<
      std::vector<std::vector<VkAttachmentDescription>>,
      std::vector<std::vector<IE::Graphics::Image::Preset>>>
                                                   buildAttachmentDescriptions();
    std::vector<std::vector<VkSubpassDescription>> buildSubpassDescriptions();
    std::vector<std::vector<VkSubpassDependency>>  buildSubpassDependencies();
};
}  // namespace IE::Graphics