#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include "Buffer/IEBuffer.hpp"

class IEBufferMemoryBarrier {
public:
	const void *pNext;
	VkAccessFlags srcAccessMask;
	VkAccessFlags dstAccessMask;
	uint32_t srcQueueFamilyIndex;
	uint32_t dstQueueFamilyIndex;
	std::shared_ptr<IEBuffer> buffer;
	VkDeviceSize offset;
	VkDeviceSize size;

	[[nodiscard]] std::vector<std::shared_ptr<IEBuffer>> getBuffers() const;

	[[nodiscard]] std::vector<std::shared_ptr<IEDependency>> getDependencies() const;

	explicit operator VkBufferMemoryBarrier() const;

	explicit operator VkBufferMemoryBarrier2() const;
};