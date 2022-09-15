#include "ShadowRenderPass.hpp"

#include "IERenderEngine.hpp"
#include "Image/AttachmentVulkan.hpp"

IE::Graphics::ShadowRenderPass IE::Graphics::ShadowRenderPass::setMapCount(size_t t_maps) {
	m_outputAttachments.resize(t_maps);
	m_attachments.resize(t_maps, {m_linkedRenderEngine, IE::Graphics::Attachment::Preset::IE_ATTACHMENT_PRESET_SHADOW_MAP, });
}

std::vector<std::shared_ptr<IE::Graphics::Image>> IE::Graphics::ShadowRenderPass::getMaps() {
	std::vector<std::shared_ptr<IE::Graphics::Image>> maps(m_outputAttachments.size());
	size_t i{0};
	std::generate(maps.begin(), maps.end(), [&] { return std::static_pointer_cast<IE::Graphics::Image>(m_outputAttachments[i++].lock()); });
	return maps;
}

void IE::Graphics::ShadowRenderPass::build() {
	std::vector<VkAttachmentDescription> attachmentDescriptions(m_attachments.size());
	size_t i{0};
	std::generate(attachmentDescriptions.begin(), attachmentDescriptions.end(), [&] {
		return std::dynamic_pointer_cast<IE::Graphics::detail::AttachmentVulkan>(m_attachments[i++].lock())->getDescription(
				VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL);
	});
	std::vector<VkSubpassDescription> subPassDescriptions(m_subPasses.size());
	i = 0;
	
	m_subPasses.resize(m_attachments.size());
	
	
	VkRenderPassCreateInfo renderPassCreateInfo{
			.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.pNext=nullptr,
			.flags=0x0,
			.attachmentCount=static_cast<uint32_t>(attachmentDescriptions.size()),
			.pAttachments=attachmentDescriptions.data(),
			.subpassCount=1,
			.pSubpasses=subPassDescriptions.data(),
			.dependencyCount=0,
			.pDependencies=nullptr
	};
	
	vkCreateRenderPass(m_linkedRenderEngine.lock()->device.device, &renderPassCreateInfo, nullptr, &m_renderPass);
}
