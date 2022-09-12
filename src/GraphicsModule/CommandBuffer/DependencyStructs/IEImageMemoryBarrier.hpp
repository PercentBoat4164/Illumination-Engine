#pragma once

#include <memory>
#include <vulkan/vulkan.h>
#include "Image/Image.hpp"

class IEImageMemoryBarrier {
public:
	const void *pNext;
	VkAccessFlags srcAccessMask;
	VkAccessFlags dstAccessMask;
	VkImageLayout newLayout;
	uint32_t srcQueueFamilyIndex;
	uint32_t dstQueueFamilyIndex;
	std::shared_ptr<IE::Graphics::Image> image;
	VkImageSubresourceRange subresourceRange;

	[[nodiscard]] std::vector<std::shared_ptr<IE::Graphics::Image>> getImages() const;
	
	explicit operator VkImageMemoryBarrier() const;

	explicit operator VkImageMemoryBarrier2() const;
};