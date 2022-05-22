#include "IEImageMemoryBarrier.hpp"

std::vector<std::shared_ptr<IEImage>> IEImageMemoryBarrier::getImages() const {
    return {image};
}

std::vector<std::shared_ptr<IEDependency>> IEImageMemoryBarrier::getDependencies() const {
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
			.image=image->image,
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
            .image=image->image,
            .subresourceRange=subresourceRange
    };
}