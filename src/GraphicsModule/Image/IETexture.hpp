#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IEBuffer;

/* Include classes used as attributes or _function arguments. */
// Internal dependencies
#include "IEImage.hpp"
#include "assimp/material.h"

// External dependencies
#include <vulkan/vulkan.h>

#include <stb_image.h>

// System dependencies
#include <cstdint>
#include <string>

class aiTexture;

class IETexture : public IEImage {
public:
	struct CreateInfo {
		VkFormat format{VK_FORMAT_R8G8B8A8_SRGB};
		VkImageLayout layout{VK_IMAGE_LAYOUT_UNDEFINED};
		VkImageType type{VK_IMAGE_TYPE_2D};
		VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};
		VkImageCreateFlags flags{};
		VmaMemoryUsage allocationUsage{};
	};

	IETexture() = default;

	IETexture(IERenderEngine *, IETexture::CreateInfo *);

	void create(IERenderEngine *, IETexture::CreateInfo *);

	void loadFromDiskToRAM(aiTexture *);

	void upload(void *);

	void upload(IEBuffer *);

	virtual void loadFromRAMToVRAM();
};