#pragma once

#include "SubPass.hpp"
#include "Image/Attachment.hpp"

#include <vulkan/vulkan_core.h>

#include <vector>
#include <array>
#include <unordered_map>

namespace IE::Graphics {
	/** @brief An implementation of the render pass concept in modern graphics programming
	 * @details This class is not meant to be instantiated, instead specific render pass types are created. Calling the constructor of the other
	 * render pass types will use the functions available from this class to create themselves with the required specifics. The results may then be
	 * upcast to this type for general use.
	 * @todo Implement a CustomRenderPass type that allows the user to build their own render pass from scratch.
	 */
	class RenderPass {
	public:
		std::vector<std::unique_ptr<IE::Graphics::SubPass>> m_subPasses;
		std::array<size_t, 2> m_dimensions;
		std::vector<std::weak_ptr<IE::Graphics::Attachment>> m_attachments;
		std::vector<std::weak_ptr<IE::Graphics::Attachment>> m_outputAttachments;
		VkRenderPass m_renderPass;
		std::weak_ptr<IERenderEngine> m_linkedRenderEngine;
		
		RenderPass();
		
		void addSubPass(VkSubpassDescription t_description, VkSubpassDependency t_dependency);
		
		void addAttachment(const std::weak_ptr<IE::Graphics::Attachment> &t_attachment);
		
		std::unordered_map<IE::Graphics::Attachment *, uint32_t> getAttachmentIndex;
		
		auto setResolution(std::array<size_t, 2> t_dimensions) -> std::remove_reference<decltype(*this)>::type;
	};
}