#pragma once

#include "Image.hpp"

namespace IE { class ImageVulkan; }

class IE::ImageVulkan : public IE::Image {
	VkFormat format;
	VkImageLayout layout;
	VkImageType type;
	VkImageTiling tiling;
	VkImageUsageFlags usage;
	VkImageCreateFlags flags;
	VkImageAspectFlags aspect;
	VmaAllocation allocation;
	
	ImageVulkan() {}
	
	~ImageVulkan() {}
};
