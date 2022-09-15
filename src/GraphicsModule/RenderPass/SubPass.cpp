#include "SubPass.hpp"

#include "RenderPass.hpp"

constexpr bool operator==(VkAttachmentReference t_first, VkAttachmentReference t_second) {
	return t_first.attachment == t_second.attachment && t_first.layout == t_second.layout;
}

VkSubpassDescription IE::Graphics::SubPass::getDescription(IE::Graphics::RenderPass *t_renderPass) {
	std::vector<VkAttachmentReference> input(m_inputAttachments.size());
	size_t i{};
	std::generate(input.begin(), input.end(), [&] { return m_inputAttachments[i++]->getReference(t_renderPass); });
	i = 0;
	std::vector<VkAttachmentReference> color(m_colorAttachments.size());
	std::generate(color.begin(), color.end(), [&] { return m_colorAttachments[i++]->getReference(t_renderPass); });
	i = 0;
	std::vector<VkAttachmentReference> resolve(m_resolveAttachments.size());
	std::generate(resolve.begin(), resolve.end(), [&] { return m_resolveAttachments[i++]->getReference(t_renderPass); });
	i = 0;
	std::vector<VkAttachmentReference> depthStencil(m_depthStencilAttachments.size());
	std::generate(depthStencil.begin(), depthStencil.end(), [&] { return m_depthStencilAttachments[i++]->getReference(t_renderPass); });
	i = 0;
	std::vector<uint32_t> preserve(t_renderPass->m_attachments.size() - input.size() - color.size() - resolve.size() - depthStencil.size());
	for (const std::weak_ptr<IE::Graphics::Attachment> &attachment: t_renderPass->m_attachments) {
		VkAttachmentReference reference{std::static_pointer_cast<IE::Graphics::detail::AttachmentVulkan>(attachment.lock())->getReference(t_renderPass)};
		if (std::any_of(input.begin(), input.end(), [&reference](VkAttachmentReference other) { return other == reference; }) ||
			std::any_of(color.begin(), color.end(), [&reference](VkAttachmentReference other) { return other == reference; }) ||
			std::any_of(resolve.begin(), resolve.end(), [&reference](VkAttachmentReference other) { return other == reference; }) ||
			std::any_of(depthStencil.begin(), depthStencil.end(), [&reference](VkAttachmentReference other) { return other == reference; })) {
			preserve.push_back(t_renderPass->getAttachmentIndex[attachment.lock().get()]);
		}
	}
	
	return VkSubpassDescription{
			.flags=0x0,  // These flags all require an associated extension
			.pipelineBindPoint=m_bindPoint,
			.inputAttachmentCount=static_cast<uint32_t>(input.size()),
			.pInputAttachments=input.data(),
			.colorAttachmentCount=static_cast<uint32_t>(color.size()),
			.pColorAttachments=color.data(),
			.pResolveAttachments=resolve.data(),
			.pDepthStencilAttachment=depthStencil.data(),
			.preserveAttachmentCount=static_cast<uint32_t>(preserve.size()),
			.pPreserveAttachments=preserve.data(),
	};
}
