#pragma once

#include "Framebuffer.hpp"
#include "Subpass.hpp"

#include <map>
#include <memory>
#include <vector>

namespace IE::Graphics {
class RenderPass {
    // As subpasses are added, the named attachments that are required are added here.
    std::map<std::string, std::shared_ptr<IE::Graphics::Attachment>> m_attachments;
    std::vector<VkAttachmentDescription>                             m_attachmentStateHistory;
    std::vector<std::string>                                         m_outputAttachments;
    IE::Graphics::Framebuffer                                        m_framebuffer;
    std::vector<IE::Graphics::Subpass>                               m_subpasses;
    std::vector<VkSubpassDescription>                                m_subpassDescriptions;
    std::array<size_t, 2>                                            m_resolution;
    std::weak_ptr<IE::Graphics::RenderEngine>                        m_linkedRenderEngine;
    VkRenderPass                                                     m_renderPass{};

public:
    explicit RenderPass(std::weak_ptr<IE::Graphics::RenderEngine> t_engineLink);

    auto addSubpass() -> decltype(*this);

    auto addSubpass(const IE::Graphics::Subpass &t_subpass) -> decltype(*this);

    auto addSubpass(const std::vector<IE::Graphics::Subpass> &t_subpass) -> decltype(*this);

    auto setResolution(std::array<size_t, 2> t_resolution) -> decltype(*this);

    auto setResolution(size_t t_width, size_t t_height) -> decltype(*this);

    auto addOutput(const std::string &t_attachment) -> decltype(*this);

    auto addOutput(const std::vector<std::string> &t_attachments) -> decltype(*this);

    std::vector<std::weak_ptr<IE::Graphics::Attachment>> build();

    // Functions piped through to the topmost subpass
    auto takesInput(const std::string &t_attachment, IE::Graphics::Attachment::Preset t_preset) -> decltype(*this);

    auto takesInput(const std::vector<std::string> &t_attachments, IE::Graphics::Attachment::Preset t_preset)
      -> decltype(*this);

    auto require(const std::string &t_attachment, IE::Graphics::Attachment::Preset t_preset) -> decltype(*this);

    auto require(const std::vector<std::string> &t_attachments, IE::Graphics::Attachment::Preset t_preset)
      -> decltype(*this);

    auto recordsColorTo(const std::string &t_attachment, IE::Graphics::Attachment::Preset t_preset)
      -> decltype(*this);

    auto recordsColorTo(const std::vector<std::string> &t_attachments, IE::Graphics::Attachment::Preset t_preset)
      -> decltype(*this);

    auto resolvesTo(const std::string &t_attachment, IE::Graphics::Attachment::Preset t_preset) -> decltype(*this);

    auto resolvesTo(const std::vector<std::string> &t_attachments, IE::Graphics::Attachment::Preset t_preset)
      -> decltype(*this);

    auto recordsDepthStencilTo(const std::string &t_attachment, IE::Graphics::Attachment::Preset t_preset)
      -> RenderPass;

    auto setBindPoint(VkPipelineBindPoint t_bindPoint) -> decltype(*this);

private:
    VkAttachmentReference *buildAttachmentReference(const std::string &t_attachment);

    std::vector<VkAttachmentReference> buildReferencesFromSource(std::vector<std::string> &t_attachments);

    static std::vector<std::string> getAllAttachmentsInSubpass(const IE::Graphics::Subpass &subpass);

    VkAttachmentDescription buildAttachmentDescriptionForSubpass(size_t i, const std::string &t_attachment);

    IE::Graphics::Subpass *findNextAttachmentUseAfter(size_t i, const std::string &t_att);

    bool isRequired(const std::string &t_attachment);
};
}  // namespace IE::Graphics