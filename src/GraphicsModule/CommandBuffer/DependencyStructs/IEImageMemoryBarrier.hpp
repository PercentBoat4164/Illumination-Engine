#pragma once

#include <memory>
#include <vulkan/vulkan.h>
#include "Image/IEImage.hpp"

class IEImageMemoryBarrier {
public:
	const void *pNext;
	VkAccessFlags srcAccessMask;
	VkAccessFlags dstAccessMask;
	VkImageLayout newLayout;
	uint32_t srcQueueFamilyIndex;
	uint32_t dstQueueFamilyIndex;
	std::shared_ptr<IEImage> image;
	VkImageSubresourceRange subresourceRange;

	[[nodiscard]] std::vector<std::shared_ptr<IEImage>> getImages() const;

	[[nodiscard]] std::vector<std::shared_ptr<IEDependency>> getDependencies() const;

	explicit operator VkImageMemoryBarrier() const;

	explicit operator VkImageMemoryBarrier2() const;
};