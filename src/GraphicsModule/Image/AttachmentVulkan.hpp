#pragma once

#include "Attachment.hpp"

#include "ImageVulkan.hpp"

#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
	class RenderPass;
}

namespace IE::Graphics::detail {
	class AttachmentVulkan : public IE::Graphics::Attachment, public IE::Graphics::detail::ImageVulkan {
	private:
		static const std::unordered_map<IE::Graphics::Attachment::Preset, VkAttachmentLoadOp> loadOpFromPreset;
		static const std::unordered_map<IE::Graphics::Attachment::Preset, VkAttachmentStoreOp> storeOpFromPreset;
		static const std::unordered_map<IE::Graphics::Attachment::Preset, VkAttachmentLoadOp> stencilLoadOpFromPreset;
		static const std::unordered_map<IE::Graphics::Attachment::Preset, VkAttachmentStoreOp> stencilStoreOpFromPreset;
		static const std::unordered_map<IE::Graphics::Attachment::Preset, VkImageLayout> initialLayoutFromPreset;
		static const std::unordered_map<IE::Graphics::Attachment::Preset, VkImageLayout> finalLayoutFromPreset;
	
	public:
		VkAttachmentLoadOp m_loadOp;
		VkAttachmentStoreOp m_storeOp;
		VkAttachmentLoadOp m_stencilLoadOp;
		VkAttachmentStoreOp m_stencilStoreOp;
		VkImageLayout m_initialLayout;
		VkImageLayout m_finalLayout;
		
		template<typename... Args>
		explicit AttachmentVulkan(const std::weak_ptr<IERenderEngine> &t_engineLink,
								  IE::Graphics::Attachment::Preset t_preset = IE_ATTACHMENT_PRESET_COLOR_OUTPUT, Args... t_args);
		
		VkAttachmentReference getReference(IE::Graphics::RenderPass *t_renderPass);
		
		VkAttachmentDescription getDescription();
	};
}