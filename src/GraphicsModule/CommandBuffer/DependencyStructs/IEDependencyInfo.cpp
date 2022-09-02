#include "IEDependencyInfo.hpp"

std::vector<std::shared_ptr<IEBuffer>> IEDependencyInfo::getBuffers() const {
	std::vector<std::shared_ptr<IEBuffer>> buffers{bufferMemoryBarriers.size()};
	for (const IEBufferMemoryBarrier &bufferBarrier: bufferMemoryBarriers) {
		buffers.push_back(bufferBarrier.buffer);
	}
	return buffers;
}

std::vector<std::shared_ptr<IEImage>> IEDependencyInfo::getImages() const {
	std::vector<std::shared_ptr<IEImage>> images{imageMemoryBarriers.size()};
	for (const IEImageMemoryBarrier &imageBarrier: imageMemoryBarriers) {
		images.push_back(imageBarrier.image);
	}
	return images;
}

std::vector<std::shared_ptr<IEDependency>> IEDependencyInfo::getDependencies() const {
	std::vector<std::shared_ptr<IEDependency>> dependencies{};
	for (const IEBufferMemoryBarrier &bufferBarrier: bufferMemoryBarriers) {
		dependencies.push_back(bufferBarrier.buffer);
	}
	for (const IEImageMemoryBarrier &imageBarrier: imageMemoryBarriers) {
		dependencies.push_back(imageBarrier.image);
	}
	return dependencies;
}

IEDependencyInfo::operator VkDependencyInfo() {
	bufferBarriers.resize(bufferMemoryBarriers.size());
	for (const IEBufferMemoryBarrier &bufferBarrier: bufferMemoryBarriers) {
		bufferBarriers.emplace_back((VkBufferMemoryBarrier2) bufferBarrier);
	}
	imageBarriers.resize(imageMemoryBarriers.size());
	for (const IEImageMemoryBarrier &imageBarrier: imageMemoryBarriers) {
		imageBarriers.emplace_back((VkImageMemoryBarrier2) imageBarrier);
	}
	return {
			.sType=VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
			.pNext=pNext,
			.dependencyFlags=dependencyFlags,
			.memoryBarrierCount=static_cast<uint32_t>(memoryBarriers.size()),
			.pMemoryBarriers=memoryBarriers.data(),
			.bufferMemoryBarrierCount=static_cast<uint32_t>(bufferMemoryBarriers.size()),
			.pBufferMemoryBarriers=bufferBarriers.data(),
			.imageMemoryBarrierCount=static_cast<uint32_t>(imageMemoryBarriers.size()),
			.pImageMemoryBarriers=imageBarriers.data(),
	};
}
