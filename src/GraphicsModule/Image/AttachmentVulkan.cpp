#include "AttachmentVulkan.hpp"

#include "RenderPass/RenderPass.hpp"

VkAttachmentDescription
IE::Graphics::detail::AttachmentVulkan::getDescription() {
	return VkAttachmentDescription{
			.flags=0x0,
			.format=m_format,
			.samples=m_samples,
			.loadOp=m_loadOp,
			.storeOp=m_storeOp,
			.stencilLoadOp=m_stencilLoadOp,
			.stencilStoreOp=m_stencilStoreOp,
			.initialLayout=m_initialLayout != VK_IMAGE_LAYOUT_UNDEFINED ? m_initialLayout : m_layout,
			.finalLayout=m_finalLayout != VK_IMAGE_LAYOUT_UNDEFINED ? m_finalLayout : m_layout,
	};
}

VkAttachmentReference IE::Graphics::detail::AttachmentVulkan::getReference(IE::Graphics::RenderPass *t_renderPass) {
	return VkAttachmentReference{
			.attachment=t_renderPass->getAttachmentIndex[this],
			.layout=m_layout,
	};
}

template<typename... Args>
IE::Graphics::detail::AttachmentVulkan::AttachmentVulkan(const std::weak_ptr<IERenderEngine> &t_engineLink,
														 IE::Graphics::Attachment::Preset t_preset, Args... t_args):
		IE::Graphics::Image(t_engineLink, t_args...),
		m_loadOp{loadOpFromPreset.at(t_preset)},
		m_storeOp{storeOpFromPreset.at(t_preset)},
		m_stencilLoadOp{stencilLoadOpFromPreset.at(t_preset)},
		m_stencilStoreOp{stencilStoreOpFromPreset.at(t_preset)},
		m_initialLayout{initialLayoutFromPreset.at(t_preset)},
		m_finalLayout{finalLayoutFromPreset.at(t_preset)} {}
