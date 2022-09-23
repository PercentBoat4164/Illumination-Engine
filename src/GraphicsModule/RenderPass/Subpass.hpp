#pragma once

#include "Image/Attachment.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class Subpass {
public:
    std::vector<std::string> m_inputAttachments;  // Names of input attachments in the dependency
    std::vector<std::string> m_colorAttachments;
    std::vector<std::string> m_resolveAttachments;
    std::string              m_depthStencilAttachment;
    std::vector<std::string> m_requiredAttachments;

    std::unordered_map<std::string, IE::Graphics::Attachment::Preset> attachmentPresets;
    VkPipelineBindPoint                                               m_bindPoint;
    std::vector<VkAttachmentDescription>                              m_attachmentDescriptions;
    static const VkSubpassContents m_contents{VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS};

    Subpass();

    auto takesInput(const std::string &t_attachment) -> decltype(*this);

    auto takesInput(const std::vector<std::string> &t_attachments) -> decltype(*this);

    auto require(const std::string &t_attachment) -> decltype(*this);

    auto require(const std::vector<std::string> &t_attachments) -> decltype(*this);

    auto recordsColorTo(const std::string &t_attachment) -> decltype(*this);

    auto recordsColorTo(const std::vector<std::string> &t_attachments) -> decltype(*this);

    auto resolvesTo(const std::string &t_attachment) -> decltype(*this);

    auto resolvesTo(const std::vector<std::string> &t_attachments) -> decltype(*this);

    auto recordsDepthStencilTo(const std::string &t_attachment, Attachment::Preset preset) -> Subpass;

    auto setBindPoint(VkPipelineBindPoint t_bindPoint) -> decltype(*this);
};
}  // namespace IE::Graphics