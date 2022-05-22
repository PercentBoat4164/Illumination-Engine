#include "IECopyBufferToImageInfo.hpp"
#include <vector>

std::vector<std::shared_ptr<IEBuffer>> IECopyBufferToImageInfo::getBuffers() const {
	return {srcBuffer};
}

std::vector<std::shared_ptr<IEImage>> IECopyBufferToImageInfo::getImages() const {
	return {dstImage};
}

std::vector<std::shared_ptr<IEDependency>> IECopyBufferToImageInfo::getDependencies() const {
	return {srcBuffer, dstImage};
}

IECopyBufferToImageInfo::operator VkCopyBufferToImageInfo2() const {
	return {
			.sType=VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
			.pNext=pNext,
			.srcBuffer=srcBuffer->buffer,
			.dstImage=dstImage->image,
			.regionCount=static_cast<uint32_t>(regions.size()),
			.pRegions=regions.data()
	};
}