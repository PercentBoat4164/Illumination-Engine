/* Include this file's header. */
#include "IEFramebuffer.hpp"

/* Include dependencies within this module. */
#include "GraphicsModule/IERenderEngine.hpp"
#include "GraphicsModule/RenderPass/IERenderPass.hpp"


void IEFramebuffer::create(IERenderEngine *engineLink, IEFramebuffer::CreateInfo *createInfo) {
	linkedRenderEngine = engineLink;
	
	std::array<VkImageLayout, 7> disallowedLayouts{VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL};
	
	framebuffers.resize(linkedRenderEngine->swapchainImageViews.size());
	
	for (const Attachment& attachment : createInfo->attachments) {
		if (std::find(disallowedLayouts.begin(), disallowedLayouts.end(), attachment.forceLayout) == disallowedLayouts.end()) {
			attachments.push_back(attachment.image);
		} else {
			linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to attach to framebuffer with invalid layout!" + std::to_string(attachment.forceLayout));
		}
	}

	// Copy createInfo data into this image
	renderPass = createInfo->renderPass;
	defaultColor = createInfo->defaultColor;

	// Verify that width and height are suitable
	width = static_cast<uint16_t>(linkedRenderEngine->swapchain.extent.width);
	height = static_cast<uint16_t>(linkedRenderEngine->swapchain.extent.height);

	if (linkedRenderEngine->swapchainImageViews[0] == VK_NULL_HANDLE) {
		throw std::runtime_error("IEFramebuffer::CreateInfo::swapchainImageViews[0] cannot be VK_NULL_HANDLE!");
	}
	if (!renderPass.lock()) {
		throw std::runtime_error("IEFramebuffer::CreateInfo::renderPass must exist!");
	}
	clearValues[0].color = VkClearColorValue{defaultColor[0], defaultColor[1], defaultColor[2], defaultColor[3]};
	clearValues[1].depthStencil = {1.0F, 0};
	clearValues[2].color = clearValues[0].color;

//	uint8_t msaaSamplesAllowed = getHighestMSAASampleCount(linkedRenderEngine->settings->msaaSamples);

	for (int i = 0; i < linkedRenderEngine->swapchain.image_count; ++i) {
		std::vector<VkImageView> framebufferAttachments{};
		framebufferAttachments.reserve(attachments.size() + 2);
		framebufferAttachments.push_back(linkedRenderEngine->swapchainImageViews[i]);
		framebufferAttachments.push_back(linkedRenderEngine->depthImage->view);
		for (const std::shared_ptr<IEImage>& image : attachments) {
			framebufferAttachments.push_back(image->view);
		}
		VkFramebufferCreateInfo framebufferCreateInfo{
				.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = renderPass.lock()->renderPass,
				.attachmentCount=static_cast<uint32_t>(framebufferAttachments.size()),
				.pAttachments=framebufferAttachments.data(),
				.width=linkedRenderEngine->swapchain.extent.width,
				.height=linkedRenderEngine->swapchain.extent.height,
				.layers=1
		};
		if (vkCreateFramebuffer(linkedRenderEngine->device.device, &framebufferCreateInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffers!");
		}
	}
}

IEFramebuffer::IEFramebuffer() = default;

IEFramebuffer::~IEFramebuffer() {
	for (VkFramebuffer framebuffer : framebuffers) {
		vkDestroyFramebuffer(linkedRenderEngine->device.device, framebuffer, nullptr);
	}
	for (std::shared_ptr<IEImage> &attachment : attachments) {
		attachment->destroy();
	}
}

VkFramebuffer IEFramebuffer::operator[](uint8_t index) {
	return framebuffers[index];
}

VkFramebuffer IEFramebuffer::getFramebuffer(uint8_t index) {
	return framebuffers[index];
}
