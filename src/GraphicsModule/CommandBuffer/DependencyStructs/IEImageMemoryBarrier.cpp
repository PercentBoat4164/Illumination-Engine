#include "IEImageMemoryBarrier.hpp"

#include "Image/ImageVulkan.hpp"

std::vector<std::shared_ptr<IE::Graphics::Image>> IEImageMemoryBarrier::getImages() const {
	return {image};
}

IEImageMemoryBarrier::operator VkImageMemoryBarrier() const {
	return {
			.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext=pNext,
			.srcAccessMask=srcAccessMask,
			.dstAccessMask=dstAccessMask,
			.newLayout=newLayout,
			.srcQueueFamilyIndex=srcQueueFamilyIndex,
			.dstQueueFamilyIndex=dstQueueFamilyIndex,
			.image=dynamic_cast<IE::Graphics::detail::ImageVulkan *>(image.get())->m_id,
			.subresourceRange=subresourceRange
	};
}

IEImageMemoryBarrier::operator VkImageMemoryBarrier2() const {
	return {
			.sType=VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.pNext=pNext,
			.srcAccessMask=srcAccessMask,
			.dstAccessMask=dstAccessMask,
			.newLayout=newLayout,
			.srcQueueFamilyIndex=srcQueueFamilyIndex,
			.dstQueueFamilyIndex=dstQueueFamilyIndex,
			.image=dynamic_cast<IE::Graphics::detail::ImageVulkan *>(image.get())->m_id,
			.subresourceRange=subresourceRange
	};
}