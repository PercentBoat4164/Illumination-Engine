#include "RenderPassSeries.hpp"

#include "Image/ImageVulkan.hpp"
#include "RenderEngine.hpp"

IE::Graphics::RenderPassSeries::RenderPassSeries(IE::Graphics::RenderEngine *t_engineLink) :
        m_linkedRenderEngine(t_engineLink) {
}

auto IE::Graphics::RenderPassSeries::build() -> decltype(*this) {
    std::vector<std::vector<VkSubpassDescription>> subpassDescriptions(m_renderPasses.size());
    // Generate all the VkSubpassDescriptions
    for (size_t i{0}; i < m_renderPasses.size(); ++i) {
        for (auto &subpass : m_renderPasses[i].m_subpasses) {
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
                        subpass.m_preserve.push_back(reference.attachment);
                        break;
                    case Subpass::IE_ATTACHMENT_USAGE_INPUT: subpass.m_input.push_back(reference); break;
                    case Subpass::IE_ATTACHMENT_USAGE_COLOR: subpass.m_color.push_back(reference); break;
                    case Subpass::IE_ATTACHMENT_USAGE_RESOLVE: subpass.m_resolve.push_back(reference); break;
                    case Subpass::IE_ATTACHMENT_USAGE_DEPTH: subpass.m_depth.push_back(reference); break;
                }
            }

            if (!subpass.m_resolve.empty() && subpass.m_resolve.size() != subpass.m_color.size())
                m_linkedRenderEngine->getLogger().log(
                  "If resolve attachments are specified, there must be the same number of resolve attachments as "
                  "color attachments.",
                  Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
                );
            if (subpass.m_depth.size() > 1)
                m_linkedRenderEngine->getLogger().log(
                  "There can only be one depth attachment in a subpass.",
                  Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
                );

            subpassDescriptions[i].push_back(
              {.flags                   = 0x0,
               .pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS,
               .inputAttachmentCount    = static_cast<uint32_t>(subpass.m_input.size()),
               .pInputAttachments       = subpass.m_input.data(),
               .colorAttachmentCount    = static_cast<uint32_t>(subpass.m_color.size()),
               .pColorAttachments       = subpass.m_color.data(),
               .pResolveAttachments     = subpass.m_resolve.data(),
               .pDepthStencilAttachment = subpass.m_depth.data(),
               .preserveAttachmentCount = static_cast<uint32_t>(subpass.m_preserve.size()),
               .pPreserveAttachments    = subpass.m_preserve.data()}
            );
        }
    }

    // Generate attachment descriptions.
    std::vector<std::vector<VkAttachmentDescription>> attachmentDescriptions(m_renderPasses.size());
    for (size_t i{0}; i < m_renderPasses.size(); ++i) {
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
                attachmentDescriptions[i].push_back(
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

    // Generate subpass dependencies.
    std::vector<std::vector<VkSubpassDependency>> subpassDependencies(m_renderPasses.size());
    for (size_t i{0}; i < m_renderPasses.size(); ++i) {
        std::vector<std::pair<
          std::vector<
            std::pair<std::pair<Subpass::AttachmentDescription, Subpass::AttachmentDescription>, uint32_t>>,
          uint32_t>>
          dependencies(m_renderPasses[i].m_subpasses.size());
        // Generate a vector of dependency information.
        for (size_t j{0}; j < m_renderPasses[i].m_subpasses.size(); ++j) {
            std::vector<
              std::pair<std::pair<Subpass::AttachmentDescription, Subpass::AttachmentDescription>, uint32_t>>
              thisSubpassDependencies{};
            for (auto &attachment : m_renderPasses[i].m_subpasses[j].m_attachments) {
                bool attachmentWrites{
                  (attachment.second.m_consumption & Subpass::IE_ATTACHMENT_CONSUMPTION_BITS_INPUT) != 0};
                // Find the next subpass in this render pass that uses this attachment.
                for (size_t k{j + 1}; k < m_renderPasses[i].m_subpasses.size(); ++k) {
                    std::pair<std::string, Subpass::AttachmentDescription> *otherAttachment{};
                    bool                                                    thisAttachmentUsedNextHere{};
                    for (auto &thisAttachment : m_renderPasses[i].m_subpasses[k].m_attachments) {
                        bool thisAttachmentWrites{
                          (thisAttachment.second.m_consumption & Subpass::IE_ATTACHMENT_CONSUMPTION_BITS_INPUT) !=
                          0};
                        // Record dependency information if this attachment mandates a subpass dependency.
                        if (attachment.first == thisAttachment.first && (attachmentWrites || thisAttachmentWrites)) {
                            thisAttachmentUsedNextHere = true;
                            otherAttachment            = &thisAttachment;
                            break;
                        }
                    }
                    if (thisAttachmentUsedNextHere)
                        thisSubpassDependencies.push_back({
                          {attachment.second, otherAttachment->second},
                          k
                        });
                }
            }
            dependencies.push_back({thisSubpassDependencies, j});
        }

        // Generate dependencies.
        for (auto &subpassDeps : dependencies) {
            for (auto &dependency : subpassDeps.first) {
                // If the dependency already exists, modify it.
                for (auto &dep : subpassDependencies[subpassDeps.second]) {
                    if (dep.srcSubpass == subpassDeps.second && dep.dstSubpass == dependency.second) {
                        dep.srcStageMask |= m_renderPasses[i].m_subpasses[subpassDeps.second].m_stage;
                        dep.dstStageMask |= m_renderPasses[i].m_subpasses[dependency.second].m_stage;
                        dep.srcAccessMask |=
                          IE::Graphics::detail::ImageVulkan::accessFlagsFromPreset(dependency.first.first.m_type);
                        dep.dstAccessMask |=
                          IE::Graphics::detail::ImageVulkan::accessFlagsFromPreset(dependency.first.second.m_type);
                    }
                }
                // otherwise, add it.
                subpassDependencies[subpassDeps.second].push_back({
                  .srcSubpass   = subpassDeps.second,
                  .dstSubpass   = dependency.second,
                  .srcStageMask = m_renderPasses[i].m_subpasses[subpassDeps.second].m_stage,
                  .dstStageMask = m_renderPasses[i].m_subpasses[dependency.second].m_stage,
                  .srcAccessMask =
                    IE::Graphics::detail::ImageVulkan::accessFlagsFromPreset(dependency.first.first.m_type),
                  .dstAccessMask =
                    IE::Graphics::detail::ImageVulkan::accessFlagsFromPreset(dependency.first.second.m_type),
                });
            }
        }
    }

    // Build render passes.

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
