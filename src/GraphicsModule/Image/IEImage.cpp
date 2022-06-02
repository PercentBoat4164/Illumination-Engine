/* Include this file's header. */
#include "IEImage.hpp"

/* Include dependencies within this module. */
#include "Buffer/IEBuffer.hpp"
#include "IERenderEngine.hpp"

/* Include dependencies from Core. */
#include "Core/LogModule/IELogger.hpp"


[[maybe_unused]] [[nodiscard]] uint8_t IEImage::getHighestMSAASampleCount(uint8_t requested) const {
	uint8_t count = 1;
	VkImageFormatProperties properties{};
	vkGetPhysicalDeviceImageFormatProperties(linkedRenderEngine->device.physical_device, format, type, tiling, usage, flags, &properties);
	return std::max(count, std::min(static_cast<uint8_t>(properties.sampleCounts), std::min(requested, linkedRenderEngine->settings->msaaSamples)));
}

[[maybe_unused]] [[nodiscard]] float IEImage::getHighestAnisotropyLevel(float requested) const {
	float anisotropyLevel = 1.0F;
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(linkedRenderEngine->device.physical_device, &properties);
	return std::max(anisotropyLevel, std::min(properties.limits.maxSamplerAnisotropy, requested));
}

IEImage::IEImage(IERenderEngine *engineLink, IEImage::CreateInfo *createInfo) {
	create(engineLink, createInfo);
}

IEImage::~IEImage() {
	/**@todo Switch to using status enum as opposed to queuing up deletion actions.*/
	destroy();
}

void IEImage::setAPI(const IEAPI &API) {
	if (API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		_create = &IEImage::_openglCreate;
		_loadFromRAMToVRAM_vector = &IEImage::_openglLoadFromRAMToVRAM_vector;
		_loadFromRAMToVRAM_voidPtr = &IEImage::_openglLoadFromRAMToVRAM_voidPtr;
		_unloadFromVRAM = &IEImage::_openglUnloadFromVRAM;
		_destroy = &IEImage::_openglDestroy;
	} else if (API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		_create = &IEImage::_vulkanCreate;
		_loadFromRAMToVRAM_vector = &IEImage::_vulkanLoadFromRAMToVRAM_vector;
		_loadFromRAMToVRAM_voidPtr = &IEImage::_vulkanLoadFromRAMToVRAM_voidPtr;
		_unloadFromVRAM = &IEImage::_vulkanUnloadFromVRAM;
		_destroy = &IEImage::_vulkanDestroy;
	}
}


std::function<void(IEImage &)> IEImage::_create{nullptr};

void IEImage::create(IERenderEngine *engineLink, IEImage::CreateInfo *createInfo) {
	linkedRenderEngine = engineLink;

	// Copy createInfo data into this image
	format = createInfo->format;
	layout = createInfo->layout;
	type = createInfo->type;
	usage = createInfo->usage;
	flags = createInfo->flags;
	aspect = createInfo->aspect;
	allocationUsage = createInfo->allocationUsage;
	width = createInfo->width;
	height = createInfo->height;
	_create(*this);
}

void IEImage::_openglCreate() {

}

void IEImage::_vulkanCreate() {
	VkImageLayout desiredLayout = layout;
	layout = VK_IMAGE_LAYOUT_UNDEFINED;

	// Set up image create info.
	VkImageCreateInfo imageCreateInfo{
			.sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType=type,
			.format=format,
			.extent=VkExtent3D{
					.width=width,
					.height=height,
					.depth=1
			},
			.mipLevels=1, // mipLevels, Unused due to no implementation of mip-mapping support yet.
			.arrayLayers=1,
			.samples=static_cast<VkSampleCountFlagBits>(1),
			.tiling=tiling,
			.usage=usage,
			.sharingMode=VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
	};

	// Set up allocation create info
	VmaAllocationCreateInfo allocationCreateInfo{
			.usage=allocationUsage,
	};

	if (width * height == 0) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN,
												 "Image width * height is zero! This may cause Vulkan to fail to create the image.");
	}

	// Create image
	if (vmaCreateImage(linkedRenderEngine->allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create image!");
	}

	// Set up image view create info
	VkImageViewCreateInfo imageViewCreateInfo{
			.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image=image,
			.viewType=VK_IMAGE_VIEW_TYPE_2D, /**@todo Add support for more than just 2D images.*/
			.format=format,
			.components=VkComponentMapping{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
										   VK_COMPONENT_SWIZZLE_IDENTITY},  // Unused. All components are mapped to default data.
			.subresourceRange=VkImageSubresourceRange{
					.aspectMask=aspect,
					.baseMipLevel=0,
					.levelCount=1,  // Unused. Mip-mapping is not yet implemented.
					.baseArrayLayer=0,
					.layerCount=1,
			},
	};

	// Create image view
	if (vkCreateImageView(linkedRenderEngine->device.device, &imageViewCreateInfo, nullptr, &view) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}

	// Set transition to requested layout from undefined or dst_optimal.
	if (layout != desiredLayout) {
		transitionLayout(desiredLayout);
	}
}


std::function<void(IEImage &, const std::vector<char> &)> IEImage::_loadFromRAMToVRAM_vector{nullptr};

void IEImage::loadFromRAMToVRAM(const std::vector<char> &data) {
	_loadFromRAMToVRAM_vector(*this, data);
}

void IEImage::_openglLoadFromRAMToVRAM_vector(const std::vector<char> &data) {

}

void IEImage::_vulkanLoadFromRAMToVRAM_vector(const std::vector<char> &data) {
	// Used to build the image in memory.
	std::shared_ptr<IEBuffer> scratchBuffer = std::make_shared<IEBuffer>(linkedRenderEngine, data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
																		 VMA_MEMORY_USAGE_CPU_TO_GPU);
	scratchBuffer->loadFromDiskToRAM(data);
	scratchBuffer->loadFromRAMToVRAM();
	scratchBuffer->toImage(shared_from_this());
	linkedRenderEngine->graphicsCommandPool->index(0)->execute();
}

std::function<void(IEImage &, void *, uint64_t)> IEImage::_loadFromRAMToVRAM_voidPtr{nullptr};

void IEImage::loadFromRAMToVRAM(void *data, uint64_t size) {
	_loadFromRAMToVRAM_voidPtr(*this, data, size);
}

void IEImage::_openglLoadFromRAMToVRAM_voidPtr(void *data, uint64_t size) {

}

void IEImage::_vulkanLoadFromRAMToVRAM_voidPtr(void *data, uint64_t size) {
	// Used to build the image in memory.
	std::shared_ptr<IEBuffer> scratchBuffer = std::make_shared<IEBuffer>(linkedRenderEngine, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
																		 VMA_MEMORY_USAGE_CPU_TO_GPU);
	scratchBuffer->loadFromDiskToRAM(data, size);
	scratchBuffer->loadFromRAMToVRAM();
	scratchBuffer->toImage(shared_from_this());
	linkedRenderEngine->graphicsCommandPool->index(0)->execute();
}


std::function<void(IEImage &)> IEImage::_unloadFromVRAM{nullptr};

void IEImage::unloadFromVRAM() {
	_unloadFromVRAM(*this);
}

void IEImage::_openglUnloadFromVRAM() {

}

void IEImage::_vulkanUnloadFromVRAM() {
	vmaDestroyImage(linkedRenderEngine->allocator, image, allocation);
	vkDestroyImageView(linkedRenderEngine->device.device, view, nullptr);
	vkDestroySampler(linkedRenderEngine->device.device, sampler, nullptr);
	invalidateDependents();
}


std::function<void(IEImage &)> IEImage::_destroy{nullptr};

void IEImage::destroy() {
	_destroy(*this);
}

void IEImage::_openglDestroy() {

}

void IEImage::_vulkanDestroy() {
	vmaDestroyImage(linkedRenderEngine->allocator, image, allocation);
	vkDestroyImageView(linkedRenderEngine->device.device, view, nullptr);
	vkDestroySampler(linkedRenderEngine->device.device, sampler, nullptr);
	invalidateDependents();
}


void IEImage::toBuffer(const std::shared_ptr<IEBuffer> &buffer, uint32_t commandBufferIndex) const {
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = format == linkedRenderEngine->swapchain.image_format ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = {0, 0, 0};
	region.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
	vkCmdCopyImageToBuffer((linkedRenderEngine->graphicsCommandPool)->index(commandBufferIndex)->commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						   buffer->buffer, 1,
						   &region);
	/**@todo Needs to be done in a way that these dependencies get added.*/
}

void IEImage::transitionLayout(VkImageLayout newLayout) {
	if (layout == newLayout) {
		return;
	}
	if (newLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN,
												 "Attempt to transition to an undefined layout (VK_IMAGE_LAYOUT_UNDEFINED)!");
		return;
	}
	IEImageMemoryBarrier imageMemoryBarrier{
			.newLayout=newLayout,
			.srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,
			.image=shared_from_this(),
			.subresourceRange={
					.aspectMask=static_cast<VkImageAspectFlags>(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ||
																newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT |
																														VK_IMAGE_ASPECT_STENCIL_BIT
																													  : VK_IMAGE_ASPECT_COLOR_BIT),
					.baseMipLevel=0,
					.levelCount=1,  // Will be used for mip mapping in the future
					.layerCount=1
			}
	};
	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	if (layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (layout == VK_IMAGE_LAYOUT_UNDEFINED &&
			   newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL | newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	} else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	} else if (layout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	} else {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to transition with unknown parameters!");
		return;
	}
	linkedRenderEngine->graphicsCommandPool->index(0)->recordPipelineBarrier(sourceStage, destinationStage, 0, {}, {}, {imageMemoryBarrier});
	layout = newLayout;
}
