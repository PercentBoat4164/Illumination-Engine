#include "RenderPass.hpp"

void IE::Graphics::RenderPass::addSubPass(VkSubpassDescription t_description, VkSubpassDependency t_dependency) {
	m_subPasses.push_back(IE::Graphics::SubPass::create(t_description, t_dependency));
}

void IE::Graphics::RenderPass::addAttachment(const std::weak_ptr<IE::Graphics::Attachment> &t_attachment) {
	m_attachments.emplace_back(t_attachment);
	getAttachmentIndex[t_attachment.lock().get()] = m_attachments.size();
}

IE::Graphics::RenderPass IE::Graphics::RenderPass::setResolution(std::array<size_t, 2> t_dimensions) {
	m_dimensions = t_dimensions;
	return *this;
}
