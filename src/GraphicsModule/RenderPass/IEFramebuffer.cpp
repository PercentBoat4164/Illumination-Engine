/* Include this file's header. */
#include "IEFramebuffer.hpp"

/* Include dependencies within this module. */
#include "GraphicsModule/IERenderEngine.hpp"
#include "GraphicsModule/RenderPass/IERenderPass.hpp"


void IEFramebuffer::create(IERenderEngine *engineLink, IEFramebuffer::CreateInfo *createInfo) {
	linkedRenderEngine = engineLink;
	
	std::array<VkImageLayout, 7> disallowedLayouts{VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL};
	
	framebuffers.resize(createInfo->swapchainImageViews.size());
	
	for (const Attachment& attachment : createInfo->attachments) {
		if (std::find(disallowedLayouts.begin(), disallowedLayouts.end(), attachment.forceLayout) == disallowedLayouts.end()) {
			attachments.push_back(attachment.image);
		}
	}
	
	
//	format = linkedRenderEngine->swapchain.image_format;
//	layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//	usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
//	aspect = VK_IMAGE_ASPECT_COLOR_BIT;
//	allocationUsage = VMA_MEMORY_USAGE_GPU_ONLY;

	// Copy createInfo data into this image
	renderPass = createInfo->renderPass;
	defaultColor = createInfo->defaultColor;

	// Verify that width and height are suitable
	width = static_cast<uint16_t>(linkedRenderEngine->swapchain.extent.width);
	height = static_cast<uint16_t>(linkedRenderEngine->swapchain.extent.height);

//	if (swapchainImageView == VK_NULL_HANDLE) {
//		throw std::runtime_error("IEFramebuffer::CreateInfo::swapchainImageView cannot be VK_NULL_HANDLE!");
//	}
//	if (!renderPass.lock()) {
//		throw std::runtime_error("IEFramebuffer::CreateInfo::renderPass cannot be a nullptr!");
//	}
	clearValues[0].color = VkClearColorValue{defaultColor[0], defaultColor[1], defaultColor[2], defaultColor[3]};
	clearValues[1].depthStencil = {1.0F, 0};
	clearValues[2].color = clearValues[0].color;

//	uint8_t msaaSamplesAllowed = getHighestMSAASampleCount(linkedRenderEngine->settings->msaaSamples);
//
//	if (msaaSamplesAllowed > VK_SAMPLE_COUNT_1_BIT) {
//		colorImage = std::make_shared<IEImage>();
//		colorImage->create(linkedRenderEngine, &framebufferImageCreateInfo);
//	}
//	framebufferImageCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
//	framebufferImageCreateInfo.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
//	framebufferImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
//	framebufferImageCreateInfo.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
//	depthImage = std::make_shared<IEImage>();
//	depthImage->create(linkedRenderEngine, &framebufferImageCreateInfo);
	std::vector<VkImageView> framebufferAttachments{linkedRenderEngine->depthImage->view, linkedRenderEngine->depthImage->view};
	for (const std::shared_ptr<IEImage>& image : attachments) {
		framebufferAttachments.push_back(image->view);
	}
	for (int i = 0; i < framebuffers.size(); ++i) {
		framebufferAttachments[0] = createInfo->swapchainImageViews[i];
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
}

VkFramebuffer IEFramebuffer::getNextFramebuffer() {
	framebufferNumber = ++framebufferNumber % framebuffers.size();
	return framebuffers[framebufferNumber];
}

VkFramebuffer IEFramebuffer::getThisFramebuffer() {
	return framebuffers[framebufferNumber];
}
