/* Include this file's header. */
#include "IEFramebuffer.hpp"

/* Include dependencies within this module. */
#include "GraphicsModule/IERenderEngine.hpp"
#include "IERenderPass.hpp"


void IEFramebuffer::copyCreateInfo(IEFramebuffer::CreateInfo *createInfo) {
	swapchainImageView = createInfo->swapchainImageView;
	renderPass = createInfo->renderPass;
	defaultColor = createInfo->defaultColor;
}

void IEFramebuffer::create(IERenderEngine *engineLink, IEFramebuffer::CreateInfo *createInfo) {
	linkedRenderEngine = engineLink;

	// Copy createInfo data into this image
	copyCreateInfo(createInfo);

	// Verify that width and height are suitable
	width = static_cast<uint16_t>(linkedRenderEngine->swapchain.extent.width);
	height = static_cast<uint16_t>(linkedRenderEngine->swapchain.extent.height);

	if (swapchainImageView == VK_NULL_HANDLE) {
		throw std::runtime_error("IEFramebuffer::CreateInfo::swapchainImageView cannot be VK_NULL_HANDLE!");
	}
	if (!renderPass.lock()) {
		throw std::runtime_error("IEFramebuffer::CreateInfo::renderPass cannot be a nullptr!");
	}
	clearValues[0].color = VkClearColorValue{defaultColor[0], defaultColor[1], defaultColor[2], defaultColor[3]};
	clearValues[1].depthStencil = {1.0F, 0};
	clearValues[2].color = clearValues[0].color;

	// Create framebuffer images
	IEImage::CreateInfo framebufferImageCreateInfo{
			.format=linkedRenderEngine->swapchain.image_format,
			.layout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.usage=VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.aspect=VK_IMAGE_ASPECT_COLOR_BIT,
			.allocationUsage=VMA_MEMORY_USAGE_GPU_ONLY,
			.width=width,
			.height=height,
	};

	uint8_t msaaSamplesAllowed = getHighestMSAASampleCount(linkedRenderEngine->settings->msaaSamples);

	if (msaaSamplesAllowed > VK_SAMPLE_COUNT_1_BIT) {
		colorImage = std::make_shared<IEImage>();
		colorImage->create(linkedRenderEngine, &framebufferImageCreateInfo);
	}
	framebufferImageCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
	framebufferImageCreateInfo.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	framebufferImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	framebufferImageCreateInfo.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthImage = std::make_shared<IEImage>();
	depthImage->create(linkedRenderEngine, &framebufferImageCreateInfo);
	std::vector<VkImageView> framebufferAttachments{msaaSamplesAllowed == VK_SAMPLE_COUNT_1_BIT ? swapchainImageView : colorImage->view,
													depthImage->view, swapchainImageView};
	VkFramebufferCreateInfo framebufferCreateInfo{
			.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = renderPass.lock()->renderPass,
			.attachmentCount=msaaSamplesAllowed == VK_SAMPLE_COUNT_1_BIT ? 2u : 3u,
			.pAttachments=framebufferAttachments.data(),
			.width=linkedRenderEngine->swapchain.extent.width,
			.height=linkedRenderEngine->swapchain.extent.height,
			.layers=1
	};
	if (vkCreateFramebuffer(linkedRenderEngine->device.device, &framebufferCreateInfo, nullptr, &framebuffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create framebuffers!");
	}
	deletionQueue.emplace_back([&] {
		vkDestroyFramebuffer(linkedRenderEngine->device.device, framebuffer, nullptr);
	});
}

IEFramebuffer::IEFramebuffer() = default;