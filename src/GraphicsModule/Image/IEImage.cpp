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
	if (status != IE_IMAGE_STATUS_UNLOADED) {
		_destroy(*this);
	}
}

void IEImage::setAPI(const IEAPI &API) {
	IETexture::setAPI(API);
	if (API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		_uploadToRAM = &IEImage::_openglUploadToRAM;
		_uploadToVRAM = &IEImage::_openglUploadToVRAM;
		_update_vector = &IEImage::_openglUpdate_vector;
		_update_void = &IEImage::_openglUpdate_voidPtr;
		_unloadFromVRAM = &IEImage::_openglUnloadFromVRAM;
		_destroy = &IEImage::_openglDestroy;
	} else if (API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		_uploadToRAM = &IEImage::_vulkanUploadToRAM;
		_uploadToVRAM = &IEImage::_vulkanUploadToVRAM;
		_update_vector = &IEImage::_vulkanUpdate_vector;
		_update_void = &IEImage::_vulkanUpdate_voidPtr;
		_unloadFromVRAM = &IEImage::_vulkanUnloadFromVRAM;
		_destroy = &IEImage::_vulkanDestroy;
	}
}


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
	channels = createInfo->channels;
	data = createInfo->data;

	status = IE_IMAGE_STATUS_UNLOADED;
}


std::function<void(IEImage &)> IEImage::_uploadToRAM{nullptr};

void IEImage::uploadToRAM() {
	status = static_cast<IEImageStatus>(status | IE_IMAGE_STATUS_QUEUED_RAM);
	if (status & IE_IMAGE_STATUS_IN_VRAM) {
		_uploadToRAM(*this);
	}
	status = static_cast<IEImageStatus>(status & ~IE_IMAGE_STATUS_QUEUED_RAM | IE_IMAGE_STATUS_IN_RAM);
}

void IEImage::_openglUploadToRAM() {

}

void IEImage::_vulkanUploadToRAM() {
	std::shared_ptr<IEBuffer> stagingBuffer = std::make_shared<IEBuffer>();
	toBuffer(stagingBuffer, width * height * channels);
	stagingBuffer->uploadToRAM();
	data = stagingBuffer->data;
}

void IEImage::uploadToRAM(const std::vector<char> &data) {
	if (data.empty() && this->data.empty()) {
		status = static_cast<IEImageStatus>(status | IE_IMAGE_STATUS_QUEUED_RAM);
		return;
	}
	if (data.size() > width * height * channels) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to load in more data to RAM than can fit in image!");
	}
	if (status & IE_IMAGE_STATUS_QUEUED_RAM || status & IE_IMAGE_STATUS_IN_RAM) {
		if (data.size() > width * height * channels) {
			linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "");
		}
		this->data = data;
	}
}

void IEImage::uploadToRAM(void *data, std::size_t size) {
	if (size == 0) {
		status = static_cast<IEImageStatus>(status | IE_IMAGE_STATUS_QUEUED_RAM);
		return;
	}
	if (size > width * height * channels) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to load in more data to RAM than can fit in image!");
	}
	if (status & IE_IMAGE_STATUS_QUEUED_RAM || status & IE_IMAGE_STATUS_IN_RAM) {
		if (size > width * height * channels) {
			linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "");
		}
		std::size_t i = 0;
		std::generate(this->data.begin(), this->data.end(), [&] -> char { return *(char *) ((std::size_t) data + i++); });
	}
}


std::function<void(IEImage &)> IEImage::_uploadToVRAM{nullptr};

void IEImage::uploadToVRAM() {
	_uploadToVRAM(*this);
	status = static_cast<IEImageStatus>(status | IE_IMAGE_STATUS_IN_VRAM);
}

void IEImage::_openglUploadToVRAM() {
	glBindTexture(GL_TEXTURE_2D, textureID);
}

void IEImage::_vulkanUploadToVRAM() {
	VkImageLayout desiredLayout = layout;
	layout = VK_IMAGE_LAYOUT_UNDEFINED;

	_vulkanCreateImage();
	_vulkanCreateImageView();

	if (aspect & VK_IMAGE_ASPECT_COLOR_BIT) {

		// Used to build the image in video memory.
		std::shared_ptr<IEBuffer> scratchBuffer = std::make_shared<IEBuffer>(linkedRenderEngine, width * height * channels * getBytesInFormat(),
																			 VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, NULL);
		scratchBuffer->uploadToVRAM(data);
		scratchBuffer->toImage(shared_from_this());

	}
	// Set transition to requested layout from undefined or dst_optimal.
	if (layout != desiredLayout) {
		transitionLayout(desiredLayout);
	}
}

std::function<void(IEImage &, const std::vector<char> &)> IEImage::_uploadToVRAM_vector{nullptr};

void IEImage::uploadToVRAM(const std::vector<char> &data) {

}

void IEImage::_openglUploadToVRAM_vector(const std::vector<char> &data) {

}

void IEImage::_vulkanUploadToVRAM_vector(const std::vector<char> &data) {

}

std::function<void(IEImage &, void *, std::size_t)> IEImage::_uploadToVRAM_void{nullptr};

void IEImage::uploadToVRAM(void *data, uint64_t size) {

}

void IEImage::_openglUploadToVRAM_void(void *data, std::size_t size) {

}

void IEImage::_vulkanUploadToVRAM_void(void *data, std::size_t size) {

}


std::function<void(IEImage &, const std::vector<char> &)> IEImage::_update_vector{nullptr};

void IEImage::update(const std::vector<char> &data) {
	_update_vector(*this, data);
}

void IEImage::_openglUpdate_vector(const std::vector<char> &data) {

}

void IEImage::_vulkanUpdate_vector(const std::vector<char> &data) {
	if (status & IE_IMAGE_STATUS_IN_RAM) {
		this->data = data;
	}
	if (status & IE_IMAGE_STATUS_QUEUED_VRAM) {

	}
	if (status & IE_IMAGE_STATUS_IN_VRAM) {
		// Used to build the image in video memory.
		std::shared_ptr<IEBuffer> scratchBuffer = std::make_shared<IEBuffer>(linkedRenderEngine, data.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
																			 VMA_MEMORY_USAGE_CPU_TO_GPU, NULL);
		scratchBuffer->uploadToVRAM(data);
		scratchBuffer->toImage(shared_from_this());
	}
}

std::function<void(IEImage &, void *, uint64_t)> IEImage::_update_void{nullptr};

void IEImage::update(void *data, uint64_t size) {
	_update_void(*this, data, size);
}

void IEImage::_openglUpdate_voidPtr(void *data, uint64_t size) {

}

void IEImage::_vulkanUpdate_voidPtr(void *data, uint64_t size) {
	if (status & IE_IMAGE_STATUS_IN_RAM) {
		this->data = std::vector<char>{(char *) data, (char *) data + size};
	}
	if (status & IE_IMAGE_STATUS_IN_VRAM) {
		// Used to build the image in video memory.
		std::shared_ptr<IEBuffer> scratchBuffer = std::make_shared<IEBuffer>(linkedRenderEngine, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
																			 VMA_MEMORY_USAGE_CPU_TO_GPU, NULL);
		scratchBuffer->uploadToVRAM(data, size);
		scratchBuffer->toImage(shared_from_this());
	}
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
	if (status != IE_IMAGE_STATUS_UNLOADED) {
		_destroy(*this);
	}
}

void IEImage::_openglDestroy() {

}

void IEImage::_vulkanDestroy() {
	_vulkanUnloadFromVRAM();
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
	vkCmdCopyImageToBuffer((linkedRenderEngine->graphicsCommandPool)->index(commandBufferIndex)->commandBuffer, image,
						   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer->buffer, 1, &region);
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
	IEImageMemoryBarrier imageMemoryBarrier{.newLayout=newLayout, .srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED, .dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED, .image=shared_from_this(), .subresourceRange={.aspectMask=aspect, .baseMipLevel=0, .levelCount=1,  // Will be used for mip mapping in the future
			.layerCount=1}};
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

void IEImage::_vulkanCreateImage() {
	// Set up image create info.
	VkImageCreateInfo imageCreateInfo{.sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, .imageType=type, .format=format, .extent=VkExtent3D{.width=width, .height=height, .depth=1}, .mipLevels=1, // mipLevels, Unused due to no implementation of mip-mapping support yet.
			.arrayLayers=1, .samples=static_cast<VkSampleCountFlagBits>(1), .tiling=tiling, .usage=usage, .sharingMode=VK_SHARING_MODE_EXCLUSIVE, .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,};

	// Set up allocation create info
	VmaAllocationCreateInfo allocationCreateInfo{.usage=allocationUsage,};

	if (width * height == 0) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN,
												 "Image width * height is zero! This may cause Vulkan to fail to create the image. This may have been caused by uploading to VRAM before updating.");
	}

	// Create image
	if (vmaCreateImage(linkedRenderEngine->allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create image!");
	}
}

void IEImage::_vulkanCreateImageView() {
	// Set up image view create info
	VkImageViewCreateInfo imageViewCreateInfo{.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image=image, .viewType=VK_IMAGE_VIEW_TYPE_2D, /**@todo Add support for more than just 2D images.*/
			.format=format, .components=VkComponentMapping{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
														   VK_COMPONENT_SWIZZLE_IDENTITY},  // Unused. All components are mapped to default data.
			.subresourceRange=VkImageSubresourceRange{.aspectMask=aspect, .baseMipLevel=0, .levelCount=1,  // Unused. Mip-mapping is not yet implemented.
					.baseArrayLayer=0, .layerCount=1,},};

	// Create image view
	if (vkCreateImageView(linkedRenderEngine->device.device, &imageViewCreateInfo, nullptr, &view) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture image view!");
	}
}

uint8_t IEImage::getBytesInFormat() const {
	switch (format) {
		case VK_FORMAT_UNDEFINED:
		case VK_FORMAT_MAX_ENUM:
		default:
			linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN,
													 "Attempt to get number of bytes in pixel of format VK_FORMAT_UNDEFINED!");
			return 0x0;
		case VK_FORMAT_R8_UNORM:
		case VK_FORMAT_R8_SNORM:
		case VK_FORMAT_R8_USCALED:
		case VK_FORMAT_R8_SSCALED:
		case VK_FORMAT_R8_UINT:
		case VK_FORMAT_R8_SINT:
		case VK_FORMAT_R8_SRGB:
		case VK_FORMAT_R4G4_UNORM_PACK8:
			return 0x1;
		case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
		case VK_FORMAT_R5G6B5_UNORM_PACK16:
		case VK_FORMAT_B5G6R5_UNORM_PACK16:
		case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
		case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
		case VK_FORMAT_R8G8_UNORM:
		case VK_FORMAT_R8G8_SNORM:
		case VK_FORMAT_R8G8_USCALED:
		case VK_FORMAT_R8G8_SSCALED:
		case VK_FORMAT_R8G8_UINT:
		case VK_FORMAT_R8G8_SINT:
		case VK_FORMAT_R8G8_SRGB:
		case VK_FORMAT_R16_UNORM:
		case VK_FORMAT_R16_SNORM:
		case VK_FORMAT_R16_USCALED:
		case VK_FORMAT_R16_SSCALED:
		case VK_FORMAT_R16_UINT:
		case VK_FORMAT_R16_SINT:
		case VK_FORMAT_R16_SFLOAT:
		case VK_FORMAT_D16_UNORM:
		case VK_FORMAT_R10X6_UNORM_PACK16:
		case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
		case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
			return 0x2;
		case VK_FORMAT_R8G8B8_UNORM:
		case VK_FORMAT_R8G8B8_SNORM:
		case VK_FORMAT_R8G8B8_USCALED:
		case VK_FORMAT_R8G8B8_SSCALED:
		case VK_FORMAT_R8G8B8_UINT:
		case VK_FORMAT_R8G8B8_SINT:
		case VK_FORMAT_R8G8B8_SRGB:
		case VK_FORMAT_B8G8R8_UNORM:
		case VK_FORMAT_B8G8R8_SNORM:
		case VK_FORMAT_B8G8R8_USCALED:
		case VK_FORMAT_B8G8R8_SSCALED:
		case VK_FORMAT_B8G8R8_UINT:
		case VK_FORMAT_B8G8R8_SINT:
		case VK_FORMAT_B8G8R8_SRGB:
		case VK_FORMAT_D16_UNORM_S8_UINT:
		case VK_FORMAT_D24_UNORM_S8_UINT:
		case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
		case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
		case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
		case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
		case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
		case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
			return 0x3;
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_R8G8B8A8_USCALED:
		case VK_FORMAT_R8G8B8A8_SSCALED:
		case VK_FORMAT_R8G8B8A8_UINT:
		case VK_FORMAT_R8G8B8A8_SINT:
		case VK_FORMAT_R8G8B8A8_SRGB:
		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_B8G8R8A8_SNORM:
		case VK_FORMAT_B8G8R8A8_USCALED:
		case VK_FORMAT_B8G8R8A8_SSCALED:
		case VK_FORMAT_B8G8R8A8_UINT:
		case VK_FORMAT_B8G8R8A8_SINT:
		case VK_FORMAT_B8G8R8A8_SRGB:
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
		case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		case VK_FORMAT_A8B8G8R8_UINT_PACK32:
		case VK_FORMAT_A8B8G8R8_SINT_PACK32:
		case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
		case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
		case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
		case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
		case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		case VK_FORMAT_A2R10G10B10_SINT_PACK32:
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
		case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
		case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:
		case VK_FORMAT_A2B10G10R10_SINT_PACK32:
		case VK_FORMAT_R16G16_UNORM:
		case VK_FORMAT_R16G16_SNORM:
		case VK_FORMAT_R16G16_USCALED:
		case VK_FORMAT_R16G16_SSCALED:
		case VK_FORMAT_R16G16_UINT:
		case VK_FORMAT_R16G16_SINT:
		case VK_FORMAT_R16G16_SFLOAT:
		case VK_FORMAT_R32_UINT:
		case VK_FORMAT_R32_SINT:
		case VK_FORMAT_R32_SFLOAT:
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
		case VK_FORMAT_X8_D24_UNORM_PACK32:
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_G8B8G8R8_422_UNORM:
		case VK_FORMAT_B8G8R8G8_422_UNORM:
		case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
			return 0x4;
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return 0x5;
		case VK_FORMAT_R16G16B16_UNORM:
		case VK_FORMAT_R16G16B16_SNORM:
		case VK_FORMAT_R16G16B16_USCALED:
		case VK_FORMAT_R16G16B16_SSCALED:
		case VK_FORMAT_R16G16B16_UINT:
		case VK_FORMAT_R16G16B16_SINT:
		case VK_FORMAT_R16G16B16_SFLOAT:
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
		case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
		case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
		case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
		case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
		case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
		case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
		case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
		case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
			return 0x6;
		case VK_FORMAT_R16G16B16A16_UNORM:
		case VK_FORMAT_R16G16B16A16_SNORM:
		case VK_FORMAT_R16G16B16A16_USCALED:
		case VK_FORMAT_R16G16B16A16_SSCALED:
		case VK_FORMAT_R16G16B16A16_UINT:
		case VK_FORMAT_R16G16B16A16_SINT:
		case VK_FORMAT_R16G16B16A16_SFLOAT:
		case VK_FORMAT_R32G32_UINT:
		case VK_FORMAT_R32G32_SINT:
		case VK_FORMAT_R32G32_SFLOAT:
		case VK_FORMAT_R64_UINT:
		case VK_FORMAT_R64_SINT:
		case VK_FORMAT_R64_SFLOAT:
		case VK_FORMAT_S8_UINT:
		case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
		case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
		case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
		case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
		case VK_FORMAT_BC4_UNORM_BLOCK:
		case VK_FORMAT_BC4_SNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
		case VK_FORMAT_EAC_R11_UNORM_BLOCK:
		case VK_FORMAT_EAC_R11_SNORM_BLOCK:
		case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
		case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
		case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
		case VK_FORMAT_R12X4_UNORM_PACK16:
		case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
		case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
		case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
		case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
		case VK_FORMAT_G16B16G16R16_422_UNORM:
		case VK_FORMAT_B16G16R16G16_422_UNORM:
		case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
		case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
		case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
		case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
		case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
		case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
		case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
		case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG:
			return 0x8;
		case VK_FORMAT_R32G32B32_UINT:
		case VK_FORMAT_R32G32B32_SINT:
		case VK_FORMAT_R32G32B32_SFLOAT:
			return 0x12;
		case VK_FORMAT_R32G32B32A32_UINT:
		case VK_FORMAT_R32G32B32A32_SINT:
		case VK_FORMAT_R32G32B32A32_SFLOAT:
		case VK_FORMAT_R64G64_UINT:
		case VK_FORMAT_R64G64_SINT:
		case VK_FORMAT_R64G64_SFLOAT:
		case VK_FORMAT_BC2_UNORM_BLOCK:
		case VK_FORMAT_BC2_SRGB_BLOCK:
		case VK_FORMAT_BC3_UNORM_BLOCK:
		case VK_FORMAT_BC3_SRGB_BLOCK:
		case VK_FORMAT_BC5_UNORM_BLOCK:
		case VK_FORMAT_BC5_SNORM_BLOCK:
		case VK_FORMAT_BC6H_UFLOAT_BLOCK:
		case VK_FORMAT_BC6H_SFLOAT_BLOCK:
		case VK_FORMAT_BC7_UNORM_BLOCK:
		case VK_FORMAT_BC7_SRGB_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
		case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
		case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
		case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
		case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
		case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
		case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
		case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
		case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
		case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
		case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
		case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
		case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
		case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
		case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
		case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
		case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
		case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
		case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
		case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
		case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
		case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
		case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
		case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
		case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
		case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
		case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
		case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
		case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
		case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
		case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
		case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
		case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
		case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
			return 0x16;
		case VK_FORMAT_R64G64B64_UINT:
		case VK_FORMAT_R64G64B64_SINT:
		case VK_FORMAT_R64G64B64_SFLOAT:
			return 0x24;
		case VK_FORMAT_R64G64B64A64_UINT:
		case VK_FORMAT_R64G64B64A64_SINT:
		case VK_FORMAT_R64G64B64A64_SFLOAT:
			return 0x32;
	}
}
