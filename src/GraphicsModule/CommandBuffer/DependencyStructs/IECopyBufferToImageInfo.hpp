#pragma once

#include <memory>
#include <vulkan/vulkan.h>
#include "Buffer/IEBuffer.hpp"
#include "Image/Image.hpp"

class IECopyBufferToImageInfo {
public:
	const void *pNext;
	std::shared_ptr<IEBuffer> srcBuffer;
	std::shared_ptr<IE::Graphics::Image> dstImage;
	std::vector<VkBufferImageCopy2> regions;

	[[nodiscard]] std::vector<std::shared_ptr<IEBuffer>> getBuffers() const;

	[[nodiscard]] std::vector<std::shared_ptr<IE::Graphics::Image>> getImages() const;

	explicit operator VkCopyBufferToImageInfo2() const;
};