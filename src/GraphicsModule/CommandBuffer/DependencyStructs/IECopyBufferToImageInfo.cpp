#include "IECopyBufferToImageInfo.hpp"
#include "Image/ImageVulkan.hpp"
#include <vector>

std::vector<std::shared_ptr<IEBuffer>> IECopyBufferToImageInfo::getBuffers() const {
    return {srcBuffer};
}

std::vector<std::shared_ptr<IE::Graphics::Image>> IECopyBufferToImageInfo::getImages() const {
    return {dstImage};
}

IECopyBufferToImageInfo::operator VkCopyBufferToImageInfo2() const {
    return {
      .sType       = VK_STRUCTURE_TYPE_COPY_BUFFER_TO_IMAGE_INFO_2,
      .pNext       = pNext,
      .srcBuffer   = srcBuffer->buffer,
      .dstImage    = dynamic_cast<IE::Graphics::detail::ImageVulkan *>(dstImage.get())->m_id,
      .regionCount = static_cast<uint32_t>(regions.size()),
      .pRegions    = regions.data()};
}