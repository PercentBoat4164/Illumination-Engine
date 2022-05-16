#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IEImage;

class IERenderEngine;

/* Include classes used as attributes or _function arguments. */
// Internal dependencies
#include "IEDependency.hpp"

// External dependencies
#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>

// System dependencies
#include <vector>
#include <cstdint>
#include <functional>


class IEBuffer : public IEDependency {
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

	void destroy();

	IEBuffer();

	IEBuffer(IERenderEngine *, IEBuffer::CreateInfo *);

	IEBuffer(IERenderEngine *, VkDeviceSize, VkBufferUsageFlags, VmaMemoryUsage);

	void create(IERenderEngine *, IEBuffer::CreateInfo *);

	void create(IERenderEngine *, VkDeviceSize, VkBufferUsageFlags, VmaMemoryUsage);

	void toImage(IEImage *, uint32_t, uint32_t);

	void toImage(IEImage *);

	~IEBuffer() override;

	void loadFromRAMToVRAM();

	void loadFromDiskToRAM(void *, uint32_t);

	void loadFromDiskToRAM(std::vector<char>);

	void unloadFromVRAM();

protected:
	IERenderEngine *linkedRenderEngine{};
	VmaAllocation allocation{};

private:
	void *internalBufferData{};
};
