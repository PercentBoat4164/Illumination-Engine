#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

class IEBuffer;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IEDependency.hpp"

// External dependencies
#include <vk_mem_alloc.h>

#include <vulkan/vulkan.h>

#include <stb_image.h>

// System dependencies
#include <cstdint>
#include <string>
#include <vector>
#include <functional>

class IEImage : public IEDependency {
private:
	static std::function<void(IEImage &)> _create;
	static std::function<void(IEImage &)> _loadFromDiskToRAM;

protected:
	[[nodiscard]] uint8_t getHighestMSAASampleCount(uint8_t requested) const;

	[[nodiscard]] float getHighestAnisotropyLevel(float requested) const;

public:
	struct CreateInfo {
		VkFormat format{VK_FORMAT_R8G8B8A8_SRGB};
		VkImageLayout layout{VK_IMAGE_LAYOUT_UNDEFINED};
		VkImageType type{VK_IMAGE_TYPE_2D};
		VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};
		VkImageCreateFlags flags{};
		VkImageAspectFlags aspect{VK_IMAGE_ASPECT_COLOR_BIT};
		VmaMemoryUsage allocationUsage{};
		uint32_t width{}, height{};
		IEBuffer *dataSource{};
		std::vector<char> data{};
	};

	VkImage image{};
	VkImageView view{};
	VkSampler sampler{};
	VkFormat format{VK_FORMAT_R8G8B8A8_SRGB};  // Image format as interpreted by the Vulkan API
	VkImageLayout layout{VK_IMAGE_LAYOUT_UNDEFINED};  // How the GPU sees the image
	VkImageType type{VK_IMAGE_TYPE_2D};  // Image shape (1D, 2D, 3D)
	VkImageTiling tiling{VK_IMAGE_TILING_OPTIMAL};  // How the image is stored in GPU memory
	VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};  // How the program is going to use the image
	VkImageCreateFlags flags{};  // How should / was the image be created
	VkImageAspectFlags aspect{VK_IMAGE_ASPECT_COLOR_BIT};
	VmaMemoryUsage allocationUsage{};  // How is the allocation going to be used between the CPU and GPU
	VmaAllocation allocation{};
	uint32_t width{};
	uint32_t height{};
	uint32_t channels{};
	std::vector<char> data{};
	std::string filename{};
	IEBuffer *dataSource{};
	IERenderEngine *linkedRenderEngine{};
	std::vector<std::function<void()>> deletionQueue{};

	IEImage() = default;

	IEImage(IERenderEngine *, IEImage::CreateInfo *);

	virtual void copyCreateInfo(IEImage::CreateInfo *) final;

	virtual void create(IERenderEngine *, IEImage::CreateInfo *);

	virtual void loadFromDiskToRAM();

	void toBuffer(const IEBuffer &, uint32_t) const;

	void transitionLayout(VkImageLayout);

    ~IEImage();

	virtual void destroy(bool=true);
};
