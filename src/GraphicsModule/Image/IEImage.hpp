#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

class IEBuffer;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IEDependency.hpp"
#include "IEAPI.hpp"

// External dependencies
#include <vk_mem_alloc.h>

#include <vulkan/vulkan.h>

#include <stb_image.h>

// System dependencies
#include <cstdint>
#include <string>
#include <vector>
#include <functional>


enum IEImageStatus {
	IE_IMAGE_STATUS_UNKNOWN = 0x0,
	IE_IMAGE_STATUS_UNLOADED = 0x1,
	IE_IMAGE_STATUS_IN_RAM = 0x2,
	IE_IMAGE_STATUS_IN_VRAM = 0x4,
};


class IEImage : public IEDependency, public std::enable_shared_from_this<IEImage> {
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
	IERenderEngine *linkedRenderEngine{};
	std::vector<char> data{};
	IEImageStatus status{IE_IMAGE_STATUS_UNKNOWN};

	IEImage() = default;

	IEImage(IERenderEngine *, IEImage::CreateInfo *);

	~IEImage() override;

	static void setAPI(const IEAPI &API);


	void create(IERenderEngine *, IEImage::CreateInfo *);

private:
	static std::function<void(IEImage &)> _uploadToRAM;
	
	static std::function<void(IEImage &)> _uploadToVRAM;

	static std::function<void(IEImage &, const std::vector<char> &)> _update_vector;
	static std::function<void(IEImage &, void *, uint64_t)> _update_voidPtr;

	static std::function<void(IEImage &)> _unloadFromVRAM;

	static std::function<void(IEImage &)> _destroy;

protected:
	virtual void _openglUploadToRAM();
	
	virtual void _vulkanUploadToRAM();
	
	
	virtual void _openglUploadToVRAM();

	virtual void _vulkanUploadToVRAM();


	virtual void _openglUpdate_vector(const std::vector<char> &);

	virtual void _vulkanUpdate_vector(const std::vector<char> &);

	virtual void _openglUpdate_voidPtr(void *, uint64_t);

	virtual void _vulkanUpdate_voidPtr(void *, uint64_t);


	virtual void _openglUnloadFromVRAM();

	virtual void _vulkanUnloadFromVRAM();


	virtual void _openglDestroy();

	virtual void _vulkanDestroy();

public:
	virtual void uploadToRAM();
	
	
	virtual void uploadToVRAM();


	virtual void update(const std::vector<char> &);

	virtual void update(void *, uint64_t);


	virtual void unloadFromVRAM();


	virtual void destroy();


	void toBuffer(const std::shared_ptr<IEBuffer> &, uint32_t) const;

	// VULKAN ONLY FUNCTIONS
	void transitionLayout(VkImageLayout);
};
