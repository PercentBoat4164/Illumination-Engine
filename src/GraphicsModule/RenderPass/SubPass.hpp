#pragma once

#include "Image/AttachmentVulkan.hpp"

#include <vulkan/vulkan_core.h>

#include <memory>

namespace IE::Graphics {
	class SubPass {
	public:
		std::vector<std::shared_ptr<IE::Graphics::detail::AttachmentVulkan>> m_inputAttachments;
		std::vector<std::shared_ptr<IE::Graphics::detail::AttachmentVulkan>> m_colorAttachments;
		std::vector<std::shared_ptr<IE::Graphics::detail::AttachmentVulkan>> m_resolveAttachments;
		std::vector<std::shared_ptr<IE::Graphics::detail::AttachmentVulkan>> m_depthStencilAttachments;
		VkPipelineBindPoint m_bindPoint;
		
		VkSubpassDescription getDescription(IE::Graphics::RenderPass *t_renderPass);
	};
}