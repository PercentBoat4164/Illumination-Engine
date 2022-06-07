#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IEImage;

class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IEDependency.hpp"

// External dependencies
#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>

// System dependencies
#include <vector>
#include <cstdint>
#include <functional>


class IEBuffer : public IEDependency, public std::enable_shared_from_this<IEBuffer> {
public:
	struct CreateInfo {
		// Only required for IEBuffer
		VkDeviceSize size{};
		VkBufferUsageFlags usage{};
		VmaMemoryUsage allocationUsage{};
	};

	typedef enum IEBufferStatus {
		IE_BUFFER_STATUS_NONE = 0x0,
		IE_BUFFER_STATUS_CREATED = 0x1,
		IE_BUFFER_STATUS_DATA_IN_RAM = 0x2,
		IE_BUFFER_STATUS_DATA_IN_VRAM = 0x4,
		IE_BUFFER_STATUS_DESTROYED = 0x8,
		IE_BUFFER_STATUS_DATA_IN_RAM_AND_VRAM = IE_BUFFER_STATUS_DATA_IN_RAM | IE_BUFFER_STATUS_DATA_IN_VRAM,
		IE_BUFFER_STATUS_MAX_ENUM = 0xFF
	} IEBufferStatus;

	std::vector<char> data{};
	VkBuffer buffer{};
	VkDeviceAddress deviceAddress{};
	VkDeviceSize size{};
	VkBufferUsageFlags usage{};
	VmaMemoryUsage allocationUsage{};
	std::vector<std::function<void()>> deletionQueue{};
	IEBufferStatus status{IE_BUFFER_STATUS_NONE};

	void destroy(bool= false);

	IEBuffer();

	IEBuffer(IERenderEngine *, IEBuffer::CreateInfo *);

	IEBuffer(IERenderEngine *, VkDeviceSize, VkBufferUsageFlags, VmaMemoryUsage);

	void create(IERenderEngine *, VkDeviceSize, VkBufferUsageFlags, VmaMemoryUsage);

	void create(IERenderEngine *engineLink, IEBuffer::CreateInfo *createInfo);

	void toImage(const std::shared_ptr<IEImage> &image, uint32_t width, uint32_t height);

	void toImage(const std::shared_ptr<IEImage> &image);

	~IEBuffer();
	
	void uploadToRAM();

	void loadFromRAMToVRAM();

	void loadFromDiskToRAM(void *, uint32_t);

	void loadFromDiskToRAM(const std::vector<char> &);

	void unloadFromVRAM();

protected:
	IERenderEngine *linkedRenderEngine{};
	VmaAllocation allocation{};

private:
	void *internalBufferData{};
};
