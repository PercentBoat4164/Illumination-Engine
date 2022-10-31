#include "RenderPassSeries.hpp"

#include "Image/ImageVulkan.hpp"
#include "RenderEngine.hpp"

IE::Graphics::RenderPassSeries::RenderPassSeries(IE::Graphics::RenderEngine *t_engineLink) :
        m_linkedRenderEngine(t_engineLink) {
}

auto IE::Graphics::RenderPassSeries::build() -> decltype(*this) {
    std::vector<IE::Graphics::Subpass *> subpasses{};
    for (auto &renderPass : m_renderPasses)
        for (auto &subpass : renderPass.m_subpasses) subpasses.push_back(&subpass);

    // Validate that all attachments were specified.
    bool allFound{true};
    for (auto *const subpass : subpasses) {
        for (const auto &requestedAttachment : subpass->m_attachments) {
            bool found{false};
            for (const auto &existingAttachment : m_attachmentPool) {
                if (requestedAttachment.first == existingAttachment.first) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                m_linkedRenderEngine->getLogger().log(
                  "Attachment '" + requestedAttachment.first + "' was not specified in this render pass series!",
                  IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
                );
                allFound = false;
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

    std::vector<VkSubpassDescription> subpassDescriptions{};

    // Generate all the VkSubpassDescriptions
    for (auto &subpass : subpasses) {
        for (const auto &attachment : subpass->m_attachments) {
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
                    subpass->preserve.push_back(reference.attachment);
                    break;
                case Subpass::IE_ATTACHMENT_USAGE_INPUT: subpass->input.push_back(reference); break;
                case Subpass::IE_ATTACHMENT_USAGE_COLOR: subpass->color.push_back(reference); break;
                case Subpass::IE_ATTACHMENT_USAGE_RESOLVE: subpass->resolve.push_back(reference); break;
                case Subpass::IE_ATTACHMENT_USAGE_DEPTH: subpass->depth.push_back(reference); break;
            }
        }

        if (!subpass->resolve.empty() && subpass->resolve.size() != subpass->color.size())
            m_linkedRenderEngine->getLogger().log(
              "If resolve attachments are specified, there must be the same number of resolve attachments as "
              "color attachments.",
              Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
            );
        if (subpass->depth.size() > 1)
            m_linkedRenderEngine->getLogger().log(
              "There can only be one depth attachment in a subpass.",
              Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
            );

        subpassDescriptions.push_back(
          {.flags                   = 0x0,
           .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
           .inputAttachmentCount    = static_cast<uint32_t>(subpass->input.size()),
           .pInputAttachments       = subpass->input.data(),
           .colorAttachmentCount    = static_cast<uint32_t>(subpass->color.size()),
           .pColorAttachments       = subpass->color.data(),
           .pResolveAttachments     = subpass->resolve.data(),
           .pDepthStencilAttachment = subpass->depth.data(),
           .preserveAttachmentCount = static_cast<uint32_t>(subpass->preserve.size()),
           .pPreserveAttachments    = subpass->preserve.data()}
        );
    }

    // Generate attachment descriptions.
    for (size_t i{0}; i < m_renderPasses.size(); ++i) {
        std::vector<VkAttachmentDescription> attachmentDescriptions;
        for (size_t j{0}; j < m_renderPasses[i].m_subpasses.size(); ++j) {
            for (const auto &attachment : m_renderPasses[i].m_subpasses[j].m_attachments) {
                // Get previous usage of this attachment by another render pass.
                std::pair<std::string, Subpass::AttachmentDescription> *previousAttachmentUsage{nullptr};
                for (size_t k{i == 0 ? i : i - 1}; k > 0; --k) {
                    for (int l{static_cast<int>(m_renderPasses[k].m_subpasses.size())}; l > 0; --l) {
                        for (auto &thisAttachment : m_renderPasses[k].m_subpasses[l].m_attachments) {
                            if (thisAttachment.second.m_usage != Subpass::IE_ATTACHMENT_USAGE_PRESERVE && thisAttachment.first == attachment.first) {
                                previousAttachmentUsage = &thisAttachment;
                                break;
                            }
                        }
                    }
                }
                if (previousAttachmentUsage == nullptr) {
                    // Look at the end to see if it should be loaded from the previous frame.
                    for (size_t k{m_renderPasses.size() - 1}; k > i; --k) {
                        for (int l{static_cast<int>(m_renderPasses[k].m_subpasses.size() - 1)}; l > 0; --l) {
                            for (auto &thisAttachment : m_renderPasses[k].m_subpasses[l].m_attachments) {
                                if (thisAttachment.second.m_usage != Subpass::IE_ATTACHMENT_USAGE_PRESERVE && thisAttachment.first == attachment.first) {
                                    previousAttachmentUsage = &thisAttachment;
                                    break;
                                }
                            }
                        }
                    }
                }

                // Get next usage of this attachment by another render pass.
                std::pair<std::string, Subpass::AttachmentDescription> *nextAttachmentUsage{nullptr};
                for (size_t k{i + 1}; k < m_renderPasses.size(); ++k) {
                    for (size_t l{0}; l < m_renderPasses[k].m_subpasses.size(); ++l) {
                        for (auto &thisAttachment : m_renderPasses[k].m_subpasses[l].m_attachments) {
                            if (thisAttachment.second.m_usage != Subpass::IE_ATTACHMENT_USAGE_PRESERVE && thisAttachment.first == attachment.first) {
                                nextAttachmentUsage = &thisAttachment;
                                break;
                            }
                        }
                    }
                }
                if (nextAttachmentUsage == nullptr) {
                    // Look at the beginning to see if it should be stored for the next frame.
                    for (size_t k{0}; k < i; ++k) {
                        for (size_t l{0}; l < m_renderPasses[k].m_subpasses.size(); ++l) {
                            for (auto &thisAttachment : m_renderPasses[k].m_subpasses[l].m_attachments) {
                                if (thisAttachment.second.m_usage != Subpass::IE_ATTACHMENT_USAGE_PRESERVE && thisAttachment.first == attachment.first) {
                                    nextAttachmentUsage = &thisAttachment;
                                    break;
                                }
                            }
                        }
                    }
                }

                // Find desired access operations.
                VkAttachmentLoadOp loadOp{
                  previousAttachmentUsage != nullptr ? (previousAttachmentUsage->second.m_consumption &
                                                        Subpass::IE_ATTACHMENT_CONSUMPTION_BITS_OUTPUT) != 0 ?
                                                       VK_ATTACHMENT_LOAD_OP_LOAD :
                                                       VK_ATTACHMENT_LOAD_OP_DONT_CARE :
                                                       VK_ATTACHMENT_LOAD_OP_DONT_CARE};
                VkAttachmentStoreOp storeOp{
                  nextAttachmentUsage != nullptr ? (nextAttachmentUsage->second.m_consumption &
                                                    Subpass::IE_ATTACHMENT_CONSUMPTION_BITS_INPUT) != 0 ?
                                                   VK_ATTACHMENT_STORE_OP_STORE :
                                                   VK_ATTACHMENT_STORE_OP_DONT_CARE :
                                                   VK_ATTACHMENT_STORE_OP_DONT_CARE};

                VkImageLayout finalLayout{
                  nextAttachmentUsage != nullptr ?
                    IE::Graphics::detail::ImageVulkan::layoutFromPreset(nextAttachmentUsage->second.m_type) :
                    attachment.second.m_usage == Subpass::IE_ATTACHMENT_USAGE_RESOLVE ?
                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR :
                    VK_IMAGE_LAYOUT_UNDEFINED};

                // Create description.
                attachmentDescriptions.push_back(
                  {.flags          = 0x0,
                   .format         = IE::Graphics::detail::ImageVulkan::formatFromPreset(attachment.second.m_type),
                   .samples        = VK_SAMPLE_COUNT_1_BIT,  ///@todo Add MSAA.
                   .loadOp         = loadOp,
                   .storeOp        = storeOp,
                   .stencilLoadOp  = loadOp,
                   .stencilStoreOp = storeOp,
                   .initialLayout  = IE::Graphics::detail::ImageVulkan::layoutFromPreset(attachment.second.m_type),
                   .finalLayout    = finalLayout}
                );
            }
        }
    }

    return *this;
}

auto IE::Graphics::RenderPassSeries::addRenderPass(IE::Graphics::RenderPass &t_pass) -> decltype(*this) {
    m_renderPasses.push_back(t_pass);
    m_renderPasses[m_renderPasses.size() - 1].m_linkedRenderEngine = m_linkedRenderEngine;
    for (const auto &subpass : t_pass.m_subpasses) {
        for (const auto &requestedAttachment : subpass.m_attachments) {
            bool found{false};
            for (const auto &existingAttachment : m_attachmentPool) {
                if (requestedAttachment.first == existingAttachment.first) {
                    found = true;
                    break;
                }
            }
            if (!found) m_attachmentPool.push_back({requestedAttachment.first, requestedAttachment.second.m_type});
        }
    }
    return *this;
}
