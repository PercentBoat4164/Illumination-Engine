#include "RenderPassSeries.hpp"

#include "Image/ImageVulkan.hpp"
#include "RenderEngine.hpp"

#include <vulkan/vulkan.h>

IE::Graphics::RenderPassSeries::RenderPassSeries(IE::Graphics::RenderEngine *t_engineLink) :
        m_linkedRenderEngine(t_engineLink) {
}

std::vector<std::vector<VkAttachmentDescription>> IE::Graphics::RenderPassSeries::buildAttachmentDescriptions() {
    std::vector<std::vector<VkAttachmentDescription>> attachmentDescriptions(m_renderPasses.size());
    for (size_t i{0}; i < m_renderPasses.size(); ++i) {
        for (size_t j{0}; j < m_renderPasses[i].m_subpasses.size(); ++j) {
            for (const auto &attachment : m_renderPasses[i].m_subpasses[j].m_attachments) {
                // Get previous usage of this attachment by another render pass.
                std::pair<std::string, Subpass::AttachmentDescription> *previousAttachmentUsage{nullptr};
                for (size_t k{i == 0 ? i : i - 1}; k > 0; --k) {
                    for (int l{static_cast<int>(m_renderPasses[k].m_subpasses.size())}; l > 0; --l) {
                        for (auto &thisAttachment : m_renderPasses[k].m_subpasses[l].m_attachments) {
                            if (thisAttachment.first == attachment.first) {
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
                                if (thisAttachment.first == attachment.first) {
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
                            if (thisAttachment.first == attachment.first) {
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
                                if (thisAttachment.first == attachment.first) {
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

                // Create description.
                attachmentDescriptions[i].push_back(
                  {.flags  = 0x0,
                   .format = IE::Graphics::detail::ImageVulkan::formatFromPreset(
                     attachment.second.m_preset,
                     m_linkedRenderEngine
                   ),
                   .samples        = VK_SAMPLE_COUNT_1_BIT,
                   .loadOp         = loadOp,
                   .storeOp        = storeOp,
                   .stencilLoadOp  = loadOp,
                   .stencilStoreOp = storeOp,
                   .initialLayout  = previousAttachmentUsage != nullptr ?
                      IE::Graphics::detail::ImageVulkan::layoutFromPreset(attachment.second.m_preset) :
                      VK_IMAGE_LAYOUT_UNDEFINED,
                   .finalLayout    = nextAttachmentUsage != nullptr ?
                        IE::Graphics::detail::ImageVulkan::layoutFromPreset(nextAttachmentUsage->second.m_preset) :
                        IE::Graphics::detail::ImageVulkan::layoutFromPreset(attachment.second.m_preset)}
                );
            }
        }
    }
    return attachmentDescriptions;
}

std::vector<std::vector<VkSubpassDescription>> IE::Graphics::RenderPassSeries::buildSubpassDescriptions() {
    std::vector<std::vector<VkSubpassDescription>> subpassDescriptions(m_renderPasses.size());
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
                  .layout     = IE::Graphics::detail::ImageVulkan::layoutFromPreset(attachment.second.m_preset)};
                switch (IE::Graphics::detail::ImageVulkan::intentFromPreset(attachment.second.m_preset)) {
                    case Image::IE_IMAGE_INTENT_COLOR: subpass.m_color.push_back(reference); break;
                    case Image::IE_IMAGE_INTENT_DEPTH: subpass.m_depth.push_back(reference); break;
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
    return subpassDescriptions;
}

std::vector<std::vector<VkSubpassDependency>> IE::Graphics::RenderPassSeries::buildSubpassDependencies() {
    std::vector<std::vector<VkSubpassDependency>> subpassDependencies(m_renderPasses.size());
    for (size_t i{0}; i < m_renderPasses.size(); ++i) {
        for (size_t j{0}; j < m_renderPasses[i].m_subpasses.size(); ++j) {
            for (auto &attachment : m_renderPasses[i].m_subpasses[j].m_attachments) {
                // Find the next usage of each attachment.
                size_t k{i};
                size_t l{j + 1};
                for (; k < m_renderPasses.size(); ++k) {
                    for (; l < m_renderPasses[k].m_subpasses.size(); ++l) {
                        for (auto &thisAttachment : m_renderPasses[k].m_subpasses[l].m_attachments) {
                            if (thisAttachment.first == attachment.first) {
                                // Record dependency as external if the dependent subpass is not in this render
                                // pass.
                                l = k == i ? l : VK_SUBPASS_EXTERNAL;
                                // Record the dependency.
                                bool                 dependencyAlreadyExists{};
                                VkPipelineStageFlags srcStage{
                                  IE::Graphics::detail::ImageVulkan::stageFromPreset(attachment.second.m_preset)};
                                VkPipelineStageFlags dstStage{
                                  IE::Graphics::detail::ImageVulkan::stageFromPreset(thisAttachment.second.m_preset
                                  )};
                                for (auto &dependency : subpassDependencies[i]) {
                                    if (dependency.srcSubpass == j && dependency.dstSubpass == k && ((dependency.dstStageMask & dstStage) != 0U)) {
                                        dependency.srcAccessMask |=
                                          IE::Graphics::detail::ImageVulkan::accessFlagsFromPreset(
                                            attachment.second.m_preset
                                          );
                                        dependency.dstAccessMask |=
                                          IE::Graphics::detail::ImageVulkan::accessFlagsFromPreset(
                                            thisAttachment.second.m_preset
                                          );
                                        dependency.srcStageMask |= srcStage;
                                        dependency.dstStageMask |= dstStage;
                                        dependencyAlreadyExists = true;
                                        break;
                                    }
                                }
                                if (!dependencyAlreadyExists) {
                                    subpassDependencies[i].push_back(
                                      {.srcSubpass    = static_cast<uint32_t>(j),
                                       .dstSubpass    = static_cast<uint32_t>(l),
                                       .srcStageMask  = srcStage,
                                       .dstStageMask  = l == VK_SUBPASS_EXTERNAL ? srcStage : dstStage,
                                       .srcAccessMask = IE::Graphics::detail::ImageVulkan::accessFlagsFromPreset(
                                         attachment.second.m_preset
                                       ),
                                       .dstAccessMask = IE::Graphics::detail::ImageVulkan::accessFlagsFromPreset(
                                         thisAttachment.second.m_preset
                                       )}
                                    );
                                }
                                // The only purpose of this goto is to escape this nested loop.
                                // The logic for this is that the attachment's dependency has already been found,
                                //      so we can skip checking the rest of the subpasses.
                                goto EXIT_FIND_SUBPASS_DEPENDENCY_LOOP;
                            }
                        }
                    }
                    l = 0;
                }
EXIT_FIND_SUBPASS_DEPENDENCY_LOOP:;
            }
        }
    }
    return subpassDependencies;
}

auto IE::Graphics::RenderPassSeries::build() -> decltype(*this) {
    // Generate all the VkSubpassDescriptions
    std::vector<std::vector<VkSubpassDescription>> subpassDescriptions{buildSubpassDescriptions()};

    // Generate attachment descriptions.
    std::vector<std::vector<VkAttachmentDescription>> attachmentDescriptions{buildAttachmentDescriptions()};

    // Generate subpass dependencies.
    std::vector<std::vector<VkSubpassDependency>> subpassDependencies{buildSubpassDependencies()};

    // Build render passes.
    for (size_t i{0}; i < m_renderPasses.size(); ++i)
        m_renderPasses[i].build(this, attachmentDescriptions[i], subpassDescriptions[i], subpassDependencies[i]);
    return *this;
}

auto IE::Graphics::RenderPassSeries::addRenderPass(IE::Graphics::RenderPass &t_pass) -> decltype(*this) {
    m_renderPasses.push_back(t_pass);
    for (const auto &subpass : t_pass.m_subpasses) {
        for (const auto &requestedAttachment : subpass.m_attachments) {
            bool found{false};
            for (const auto &existingAttachment : m_attachmentPool) {
                if (requestedAttachment.first == existingAttachment.first) {
                    found = true;
                    break;
                }
            }
            if (!found)
                m_attachmentPool.push_back({requestedAttachment.first, requestedAttachment.second.m_preset});
        }
    }
    return *this;
}

bool IE::Graphics::RenderPassSeries::start(size_t frameNumber) {
    return m_renderPasses[m_currentPass].start(m_masterCommandBuffer, m_framebuffer);
}

bool IE::Graphics::RenderPassSeries::nextPass() {
    m_currentPass = m_currentPass >= m_renderPasses.size() ? m_currentPass + 1 : 0;
    m_renderPasses[m_currentPass].nextPass(m_masterCommandBuffer);
    return true;
}

bool IE::Graphics::RenderPassSeries::finish() {
    return true;
}
