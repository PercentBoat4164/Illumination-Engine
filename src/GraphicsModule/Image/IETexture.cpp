/* Include this file's header. */
#include "IETexture.hpp"

/* Include dependencies within this module. */
#include "Buffer/IEBuffer.hpp"
#include "IERenderEngine.hpp"

/* Include dependencies from Core. */
#include "Core/LogModule/IELogger.hpp"

#include "assimp/texture.h"


IETexture::IETexture(IERenderEngine *engineLink, IETexture::CreateInfo *createInfo) {
	create(engineLink, createInfo);
}

void IETexture::setAPI(const IEAPI &API) {
	if (API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		_uploadToVRAM = &IETexture::_openglUploadToVRAM;
		_update_vector = &IETexture::_openglUpdate_vector;
		_update_voidPtr = &IETexture::_openglUpdate_voidPtr;
		_update_aiTexture = &IETexture::_openglUpdate_aiTexture;
		_unloadFromVRAM = &IETexture::_openglUnloadFromVRAM;
		_destroy = &IETexture::_openglDestroy;
	} else if (API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		_uploadToVRAM = &IETexture::_vulkanUploadToVRAM;
		_update_vector = &IETexture::_vulkanUpdate_vector;
		_update_voidPtr = &IETexture::_vulkanUpdate_voidPtr;
		_update_aiTexture = &IETexture::_vulkanUpdate_aiTexture;
		_unloadFromVRAM = &IETexture::_vulkanUnloadFromVRAM;
		_destroy = &IETexture::_vulkanDestroy;
	}
}


void IETexture::create(IERenderEngine *engineLink, IETexture::CreateInfo *createInfo) {
	linkedRenderEngine = engineLink;
	layout = createInfo->layout;
	format = createInfo->format;
	type = createInfo->type;
	usage = createInfo->usage;

	/**@todo Implement mip-mapping again.*/
	// Determine image data based on settings and input data
//        auto maxMipLevel = static_cast<uint8_t>(std::floor(std::log2(std::max(width, height))) + 1);
//        mipLevels = mipMapping && linkedRenderEngine->settings.mipMapping ? std::min(std::max(maxMipLevel, static_cast<uint8_t>(1)), static_cast<uint8_t>(linkedRenderEngine->settings.mipMapLevel)) : 1;
}


std::function<void(IETexture &)> IETexture::_uploadToVRAM{nullptr};

void IETexture::uploadToVRAM() {
	_uploadToVRAM(*this);

	// Update status
	status = static_cast<IEImageStatus>(status | IE_IMAGE_STATUS_IN_VRAM);
}

void IETexture::_openglUploadToVRAM() {

}

void IETexture::_vulkanUploadToVRAM() {
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
			.tiling=VK_IMAGE_TILING_OPTIMAL,
			.usage=usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			.sharingMode=VK_SHARING_MODE_EXCLUSIVE,
			.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
	};

	// Set up allocation create info
	VmaAllocationCreateInfo allocationCreateInfo{
			.usage=allocationUsage,
	};

	// Create image
	if (vmaCreateImage(linkedRenderEngine->allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create texture image!");
	}

	// Set up image view create info.
	VkImageViewCreateInfo imageViewCreateInfo{
			.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image=image,
			.viewType=VK_IMAGE_VIEW_TYPE_2D, /**@todo Add support for more than just 2D images.*/
			.format=format,
			.components=VkComponentMapping{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
										   VK_COMPONENT_SWIZZLE_IDENTITY},  // Unused. All components are mapped to default data.
			.subresourceRange=VkImageSubresourceRange{
					.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT,
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

	// Determine the anisotropy level to use.
//        anisotropyLevel = linkedRenderEngine->settings.anisotropicFilterLevel > 0 * std::min(anisotropyLevel, linkedRenderEngine->device.physical_device.properties.limits.maxSamplerAnisotropy);

	// Set up image sampler create info.
	VkSamplerCreateInfo samplerCreateInfo{
			.sType=VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter=VK_FILTER_NEAREST,
			.minFilter=VK_FILTER_NEAREST,
			.mipmapMode=VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU=VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeV=VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.addressModeW=VK_SAMPLER_ADDRESS_MODE_REPEAT,
			.mipLodBias=linkedRenderEngine->settings->mipMapLevel,
//                .anisotropyEnable=linkedRenderEngine->settings.anisotropicFilterLevel > 0,
//                .maxAnisotropy=anisotropyLevel,
			.compareEnable=VK_FALSE,
			.compareOp=VK_COMPARE_OP_ALWAYS,
			.minLod=0.0F,
//                .maxLod=static_cast<float>(mipLevels),
			.borderColor=VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
			.unnormalizedCoordinates=VK_FALSE,
	};

	// Create image sampler
	if (vkCreateSampler(linkedRenderEngine->device.device, &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}

	// Set transition to requested layout from undefined or dst_optimal.
	if (desiredLayout != VK_IMAGE_LAYOUT_UNDEFINED && layout != desiredLayout) {
		transitionLayout(desiredLayout);
	}
	linkedRenderEngine->graphicsCommandPool->index(0)->execute();
}

void IETexture::uploadToVRAM(const std::vector<char> &data) {
	uploadToVRAM();

	update(data);
}

void IETexture::uploadToVRAM(void *data, uint64_t size) {
	uploadToVRAM();

	update(data, size);
}

void IETexture::uploadToVRAM(aiTexture *texture) {
	uploadToVRAM();

	update(texture);
}


std::function<void(IETexture &, const std::vector<char> &)> IETexture::_update_vector{nullptr};

void IETexture::update(const std::vector<char> &data) {
	_update_vector(*this, data);
}

std::function<void(IETexture &, void *, uint64_t)> IETexture::_update_voidPtr{nullptr};

void IETexture::update(void *data, uint64_t size) {
	_update_voidPtr(*this, data, size);
}

std::function<void(IETexture &, aiTexture *)> IETexture::_update_aiTexture{nullptr};

void IETexture::update(aiTexture *texture) {
	_update_aiTexture(*this, texture);
}

void IETexture::_openglUpdate_aiTexture(aiTexture *texture) {

}

void IETexture::_vulkanUpdate_aiTexture(aiTexture *texture) {
	stbi_uc *tempData;
	if (texture->mHeight == 0) {
		tempData = stbi_load_from_memory((unsigned char *) texture->pcData, (int) texture->mWidth, reinterpret_cast<int *>(&width),
										 reinterpret_cast<int *>(&height), reinterpret_cast<int *>(&channels), 4);
	} else {
		tempData = stbi_load(texture->mFilename.C_Str(), reinterpret_cast<int *>(&width), reinterpret_cast<int *>(&height),
							 reinterpret_cast<int *>(&channels), 4);
	}
	channels = 4;  /**@todo Make number of channels imported change the number of channels in VRAM.*/
	std::vector<char> data = std::vector<char>{(char *) tempData, (char *) ((uint64_t) tempData + width * height * channels)};
	if (data.empty()) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN,
												 std::string{"Failed to load image data from file: '"} + texture->mFilename.C_Str() + "' due to " +
												 stbi_failure_reason());
	}

	_vulkanUpdate_vector(data);
}


std::function<void(IETexture &)> IETexture::_unloadFromVRAM{nullptr};

void IETexture::unloadFromVRAM() {
	_unloadFromVRAM(*this);
}


std::function<void(IETexture &)> IETexture::_destroy{nullptr};

void IETexture::destroy() {
	if (status != IE_IMAGE_STATUS_UNLOADED) {
		_destroy(*this);
	}
}
