#include "RenderPassSeries.hpp"

#include "Image/ImageVulkan.hpp"
#include "RenderEngine.hpp"

IE::Graphics::RenderPassSeries::RenderPassSeries(IE::Graphics::RenderEngine *t_engineLink) :
        m_linkedRenderEngine(t_engineLink) {
}

auto IE::Graphics::RenderPassSeries::build() -> decltype(*this) {
    // Validate that all attachments were specified.
    bool allFound{true};
    for (const auto &renderPass : m_renderPasses) {
        for (const auto &subpass : renderPass.m_subpasses) {
            for (const auto &requestedAttachment : subpass.m_attachments) {
                bool found{false};
                for (const auto &existingAttachment : m_attachmentPool) {
                    if (requestedAttachment.first == existingAttachment.first) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    m_linkedRenderEngine->getLogger().log(
                      "Attachment '" + requestedAttachment.first +
                        "' was not specified in this render pass series!",
                      IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
                    );
                    allFound = false;
                }
            }
        }
    }
    if (!allFound) {
        m_linkedRenderEngine->getLogger().log(
          "Stopping build of render pass series because not all attachments are known.",
          Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
        return *this;
    }

    // Generate all the VkSubpassDescriptions
    for (auto &renderPass : m_renderPasses) {
        for (auto &subpass : renderPass.m_subpasses) {
            for (const auto &attachment : subpass.m_attachments) {
                VkAttachmentReference reference{
                  .attachment = static_cast<uint32_t>(std::distance(
                    m_attachmentPool.begin(),
                    std::find_if(
                      m_attachmentPool.begin(),
                      m_attachmentPool.end(),
                      [&](std::pair<std::string, Image::Preset> &t) { return t.first == attachment.first; }
                    )
                  )),
                  .layout     = IE::Graphics::detail::ImageVulkan::layoutFromPreset(attachment.second.m_type)};
                switch (attachment.second.m_usage) {
                    case Subpass::IE_ATTACHMENT_USAGE_PRESERVE:
                        subpass.preserve.push_back(reference.attachment);
                        break;
                    case Subpass::IE_ATTACHMENT_USAGE_INPUT: subpass.input.push_back(reference); break;
                    case Subpass::IE_ATTACHMENT_USAGE_COLOR: subpass.color.push_back(reference); break;
                    case Subpass::IE_ATTACHMENT_USAGE_RESOLVE: subpass.resolve.push_back(reference); break;
                    case Subpass::IE_ATTACHMENT_USAGE_DEPTH: subpass.depth.push_back(reference); break;
                }
            }

            if (!subpass.resolve.empty() && subpass.resolve.size() != subpass.color.size())
                m_linkedRenderEngine->getLogger().log(
                  "If resolve attachments are specified, there must be the same number of resolve attachments as "
                  "color attachments.",
                  Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
                );
            if (subpass.depth.size() > 1)
                m_linkedRenderEngine->getLogger().log(
                  "There can only be one depth attachment in a subpass.",
                  Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
                );

            subpass.m_description = {
              .flags                   = 0x0,
              .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
              .inputAttachmentCount    = static_cast<uint32_t>(subpass.input.size()),
              .pInputAttachments       = subpass.input.data(),
              .colorAttachmentCount    = static_cast<uint32_t>(subpass.color.size()),
              .pColorAttachments       = subpass.color.data(),
              .pResolveAttachments     = subpass.resolve.data(),
              .pDepthStencilAttachment = subpass.depth.data(),
              .preserveAttachmentCount = static_cast<uint32_t>(subpass.preserve.size()),
              .pPreserveAttachments    = subpass.preserve.data()};
        }
    }

    return *this;
}

auto IE::Graphics::RenderPassSeries::addRenderPass(IE::Graphics::RenderPass &t_pass) -> decltype(*this) {
    m_renderPasses.push_back(t_pass);
    m_renderPasses[m_renderPasses.size() - 1].m_linkedRenderEngine = m_linkedRenderEngine;
    return *this;
}
