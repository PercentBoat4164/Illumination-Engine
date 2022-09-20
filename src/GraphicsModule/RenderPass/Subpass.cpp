#include <iostream>
#include "Subpass.hpp"

template<typename T>
constexpr std::vector<T> &operator<<(std::vector<T> &t_first, const std::vector<T> &t_second) {
	auto oldFirstEnd = t_first.end();
	size_t i{};
	t_first.resize(t_first.size() + t_second.size());
	std::generate(oldFirstEnd, t_first.end(), [&t_second, &i] { return t_second[i++]; });
	return t_first;
}

auto IE::Graphics::Subpass::takesInput(const std::string &t_attachment) -> decltype(*this) {
	m_inputAttachments.push_back(t_attachment);
	m_requiredAttachments.push_back(t_attachment);
	return *this;
}

auto IE::Graphics::Subpass::takesInput(const std::vector<std::string> &t_attachments) -> decltype(*this) {
	m_inputAttachments << t_attachments;
	m_requiredAttachments << t_attachments;
	return *this;
}

auto IE::Graphics::Subpass::recordsColorTo(const std::string &t_attachment) -> decltype(*this) {
	m_colorAttachments.push_back(t_attachment);
	return *this;
}

auto IE::Graphics::Subpass::recordsColorTo(const std::vector<std::string> &t_attachments) -> decltype(*this) {
	m_colorAttachments << t_attachments;
	return *this;
}

auto IE::Graphics::Subpass::resolvesTo(const std::string &t_attachment) -> decltype(*this) {
	m_resolveAttachments.push_back(t_attachment);
	return *this;
}

auto IE::Graphics::Subpass::resolvesTo(const std::vector<std::string> &t_attachments) -> decltype(*this) {
	m_resolveAttachments << t_attachments;
	return *this;
}

auto IE::Graphics::Subpass::recordsDepthStencilTo(const std::string &t_attachment) -> decltype(*this) {
	if (!m_depthStencilAttachment.empty()) {
		std::cout << "Cannot overwrite previous depth / stencil map";
		return *this;
	}
	m_depthStencilAttachment = t_attachment;
	return *this;
}

auto IE::Graphics::Subpass::setBindPoint(VkPipelineBindPoint t_bindPoint) -> decltype(*this) {
	m_bindPoint = t_bindPoint;
	return *this;
}

IE::Graphics::Subpass::Subpass() :
		m_inputAttachments(),
		m_colorAttachments(),
		m_resolveAttachments(),
		m_depthStencilAttachment(),
		m_bindPoint{VK_PIPELINE_BIND_POINT_GRAPHICS} {}

auto IE::Graphics::Subpass::require(const std::string &t_attachment) -> decltype(*this) {
	m_requiredAttachments.push_back(t_attachment);
	return *this;
}

auto IE::Graphics::Subpass::require(const std::vector<std::string> &t_attachments) -> decltype(*this) {
	m_requiredAttachments << t_attachments;
	return *this;
}

