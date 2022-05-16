/* Include this file's header. */
#include "IEBuffer.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"


void IEBuffer::destroy() {
	if (hasNoDependents()) {
		if ((status & IE_BUFFER_STATUS_CREATED) == 0) {
			return;
		}
		wait();
		for (std::function<void()> &function: deletionQueue) {
			function();
		}
		removeAllDependents();
		deletionQueue.clear();
	}
}

void IEBuffer::loadFromDiskToRAM(void *pData, uint32_t dataSize) {
	data = std::vector<char>{(char *) pData, (char *) ((uint64_t) pData + dataSize)};
	if (!data.empty()) {
		status = static_cast<IEBufferStatus>(IE_BUFFER_STATUS_DATA_IN_RAM | status);
	}
}

void IEBuffer::loadFromDiskToRAM(std::vector<char> dataVector) {
	data = dataVector;
	status = static_cast<IEBufferStatus>(IE_BUFFER_STATUS_DATA_IN_RAM | status);
}

void IEBuffer::loadFromRAMToVRAM() {
	if ((status & IE_BUFFER_STATUS_DATA_IN_RAM) == 0) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to load buffer with no contents in RAM to VRAM.");
	}
	if ((status & IE_BUFFER_STATUS_DATA_IN_VRAM) == 0) {
		// Create the VkBuffer because it does not yet exist.
		VmaAllocationCreateInfo allocationCreateInfo{
				.usage=allocationUsage,
		};
		VkBufferCreateInfo bufferCreateInfo{
				.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
				.size=size,
				.usage=usage,
		};

		// If usage request device address, then prepare allocation for using device address
		if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0U) {
			allocationCreateInfo.requiredFlags |= VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
		}

		// Create buffer and update status
		vmaCreateBuffer(linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr);
		status = static_cast<IEBufferStatus>(IE_BUFFER_STATUS_DATA_IN_VRAM | status);

		// Get the buffer device address if requested
		if ((usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) != 0U) {
			VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
			bufferDeviceAddressInfo.buffer = buffer;
			deviceAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device.device, &bufferDeviceAddressInfo);
		}
	}

	// Upload data in RAM to VRAM
	vmaMapMemory(linkedRenderEngine->allocator, allocation, &internalBufferData);
	memcpy(internalBufferData, data.data(), size);
	vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
}

void IEBuffer::unloadFromVRAM() {
	if ((status & IE_BUFFER_STATUS_DATA_IN_VRAM) == 0) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to unload buffer from VRAM when buffer not in VRAM.");
	}
	vmaDestroyBuffer(linkedRenderEngine->allocator, buffer, allocation);
	status = static_cast<IEBufferStatus>(~IE_BUFFER_STATUS_DATA_IN_VRAM & status);
}

void IEBuffer::toImage(IEImage *image, uint32_t width, uint32_t height) {
	VkBufferImageCopy region{};
	region.imageSubresource.aspectMask =
			image->layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || image->layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ?
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageExtent = {width, height, 1};
	VkImageLayout oldLayout;
	if (image->layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		oldLayout = image->layout;
		image->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		linkedRenderEngine->graphicsCommandPool[0].recordCopyBufferToImage(this, image, {region});
		image->transitionLayout(oldLayout);
	} else {
		linkedRenderEngine->graphicsCommandPool[0].recordCopyBufferToImage(this, image, {region});
	}
	linkedRenderEngine->graphicsCommandPool[0].execute();
}

void IEBuffer::toImage(IEImage *image) {
	VkBufferImageCopy region{};
	region.imageSubresource.aspectMask =
			image->layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || image->layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ?
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	region.imageExtent = {image->width, image->height, 1};
	VkImageLayout oldLayout;
	if (image->layout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		oldLayout = image->layout;
		image->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		linkedRenderEngine->graphicsCommandPool[0].recordCopyBufferToImage(this, image, {region});
		image->transitionLayout(oldLayout);
	} else {
		linkedRenderEngine->graphicsCommandPool[0].recordCopyBufferToImage(this, image, {region});
	}
}

IEBuffer::IEBuffer(IERenderEngine *engineLink, IEBuffer::CreateInfo *createInfo) {
	create(engineLink, createInfo);
}

void IEBuffer::create(IERenderEngine *engineLink, VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage) {
	if ((status & IE_BUFFER_STATUS_CREATED) != 0) {}
	linkedRenderEngine = engineLink;
	size = bufferSize;
	usage = usageFlags;
	allocationUsage = memoryUsage;
	status = IE_BUFFER_STATUS_CREATED;
}

void IEBuffer::create(IERenderEngine *engineLink, IEBuffer::CreateInfo *createInfo) {
	linkedRenderEngine = engineLink;
	size = createInfo->size;
	usage = createInfo->usage;
	allocationUsage = createInfo->allocationUsage;
	status = IE_BUFFER_STATUS_CREATED;
}

IEBuffer::~IEBuffer() {
	destroy();
}

IEBuffer::IEBuffer(IERenderEngine *engineLink, VkDeviceSize bufferSize, VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage) {
	create(engineLink, bufferSize, usageFlags, memoryUsage);
}

IEBuffer::IEBuffer() = default;
