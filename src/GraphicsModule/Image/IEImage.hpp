#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

class IEBuffer;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "GraphicsModule/CommandBuffer/IEDependency.hpp"
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
	IE_IMAGE_STATUS_LOADED = 0x2,
	IE_IMAGE_STATUS_QUEUED_RAM = 0x4,
	IE_IMAGE_STATUS_IN_RAM = 0x8,
	IE_IMAGE_STATUS_QUEUED_VRAM = 0x10,
	IE_IMAGE_STATUS_IN_VRAM = 0x11,
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
		uint32_t width{}, height{}, channels{};
		std::vector<char> data{};
	};

	VkImage image{};
	VkImageView view{};
	VkSampler sampler{};
	/**@todo Add proper format checks.*/
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
	static std::function<void(IEImage &, const std::vector<char> &)> _uploadToRAM_vector;
	static std::function<void(IEImage &, void *, std::size_t)> _uploadToRAM_void;
	
	static std::function<void(IEImage &)> _uploadToVRAM;
	static std::function<void(IEImage &, const std::vector<char> &)> _uploadToVRAM_vector;
	static std::function<void(IEImage &, void *, std::size_t)> _uploadToVRAM_void;

	static std::function<void(IEImage &, const std::vector<char> &)> _update_vector;
	static std::function<void(IEImage &, void *, std::size_t)> _update_void;

	static std::function<void(IEImage &)> _unloadFromVRAM;

	static std::function<void(IEImage &)> _destroy;

protected:
	virtual void _openglUploadToRAM();
	
	virtual void _vulkanUploadToRAM();
	
	virtual void _openglUploadToRAM_vector(const std::vector<char> &);
	
	virtual void _vulkanUploadToRAM_vector(const std::vector<char> &);
	
	virtual void _openglUploadToRAM_void(void *, std::size_t);
	
	virtual void _vulkanUploadToRAM_void(void *, std::size_t);
	
	
	virtual void _openglUploadToVRAM();
	
	virtual void _vulkanUploadToVRAM();
	
	virtual void _openglUploadToVRAM_vector(const std::vector<char> &);
	
	virtual void _vulkanUploadToVRAM_vector(const std::vector<char> &);
	
	virtual void _openglUploadToVRAM_void(void *, std::size_t);
	
	virtual void _vulkanUploadToVRAM_void(void *, std::size_t);
	
	
	virtual void _openglUpdate_vector(const std::vector<char> &);
	
	virtual void _vulkanUpdate_vector(const std::vector<char> &);
	
	virtual void _openglUpdate_voidPtr(void *, std::size_t);
	
	virtual void _vulkanUpdate_voidPtr(void *, std::size_t);
	
	
	virtual void _openglUnloadFromVRAM();
	
	virtual void _vulkanUnloadFromVRAM();
	
	
//	virtual void _openglUnloadFromRAM();
//
//	virtual void _vulkanUnloadFromRAM();
	
	
	virtual void _openglDestroy();
	
	virtual void _vulkanDestroy();
	
	
	virtual void _vulkanCreateImage();
	
	virtual void _vulkanCreateImageView();

public:
	void uploadToRAM();
	
	void uploadToRAM(const std::vector<char> &);
	
	void uploadToRAM(void *, uint64_t);
	
	
	virtual void uploadToVRAM();
	
	void uploadToVRAM(const std::vector<char> &);
	
	void uploadToVRAM(void *, uint64_t);
	

	void update(const std::vector<char> &);

	void update(void *, uint64_t);


	void unloadFromVRAM();


	void destroy();


	void toBuffer(const std::shared_ptr<IEBuffer> &, uint32_t) const;
	
	
	// VULKAN ONLY FUNCTIONS
	
	
	uint8_t getBytesInFormat() const;
	
	void transitionLayout(VkImageLayout);
};
