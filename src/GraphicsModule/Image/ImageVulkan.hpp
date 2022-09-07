#pragma once

#include "Image.hpp"

#include <vulkan/vulkan.h>

namespace IE { class ImageVulkan; }

class IE::ImageVulkan : public IE::Image {
public:
	VkFormat format;  // Pixel format of the image. Includes channel count, location, and depth.
	VkImageLayout layout;  // How the image is to be laid out in memory.
	VkImageType type;  // Defines the image's dimensionality.
	VkImageTiling tiling;  // Boilerplate. Leave at VK_IMAGE_TILING_OPTIMAL.
	VkImageUsageFlags usage;  // How the program will be using the image.
	VkImageCreateFlags flags;  // A record of the flags used to create the image.
	VkImageAspectFlags aspect;  // What aspects should the image have? (e.g. Depth, Color)
	VmaMemoryUsage allocationUsage{};  // A record of what this allocation is optimized for.
	VmaAllocation allocation{};  // The allocation used to create this image.
	
	
	ImageVulkan();
	
	ImageVulkan(ImageVulkan &&other) noexcept;
	
	ImageVulkan(const ImageVulkan &other);
	
	ImageVulkan &operator=(const ImageVulkan &other) {
		if (&other != this) {
			width = other.width;
			height = other.height;
			channels = other.channels;
			if (m_linkedRenderEngine.lock() != other.m_linkedRenderEngine.lock()) {
				/**@todo Add the parts of this function that would destroy the image and rebuild it under the new engine. This may have to be implemented in the classes that inherit this one. */
				m_linkedRenderEngine = other.m_linkedRenderEngine;
			}
		}
		return *this;
	}
	
	ImageVulkan &operator=(ImageVulkan &&other) noexcept {
		if (&other != this) {
			width = std::exchange(other.width, 0);
			height = std::exchange(other.height, 0);
			channels = std::exchange(other.channels, 0);
			if (m_linkedRenderEngine.lock() != other.m_linkedRenderEngine.lock()) {
				/**@todo Add the parts of this function that would destroy the image and rebuild it under the new engine. This may have to be implemented in the classes that inherit this one. */
				m_linkedRenderEngine = other.m_linkedRenderEngine;
			}
		}
		return *this;
	}
	
	~ImageVulkan() = default;
	
	void makePolymorphic() override {};
};
