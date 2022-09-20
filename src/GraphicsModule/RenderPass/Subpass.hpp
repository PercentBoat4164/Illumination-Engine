#pragma once

#include <vulkan/vulkan_core.h>
#include "Image/Attachment.hpp"

#include <vector>
#include <string>

namespace IE::Graphics {
	class Subpass {
	public:
		std::vector<std::string> m_inputAttachments;  // Names of input attachments in the dependency
		std::vector<std::string> m_colorAttachments;
		std::vector<std::string> m_resolveAttachments;
		std::string m_depthStencilAttachment;
		std::vector<std::string> m_requiredAttachments;
		
		VkPipelineBindPoint m_bindPoint;
		std::vector<VkAttachmentDescription> m_attachmentDescriptions;
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
		
		auto recordsDepthStencilTo(const std::string &t_attachment) -> decltype(*this);
		
		auto setBindPoint(VkPipelineBindPoint t_bindPoint) -> decltype(*this);
	};
}