/* Include this file's header. */
#include "IEBuffer.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"


void IEBuffer::destroy(bool ignoreDependents) {
    if (hasNoDependents() || ignoreDependents) {
        if (!created) {
            return;
        }
        wait();
        for (std::function<void()> &function: deletionQueue) {
            function();
        }
        removeAllDependents();
        deletionQueue.clear();
        created = false;
    }
}

void IEBuffer::create(IERenderEngine *engineLink, IEBuffer::CreateInfo *createInfo) {
    linkedRenderEngine = engineLink;
    createdWith = *createInfo;
    VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.size = createdWith.size;
    bufferCreateInfo.usage = createdWith.usage;
    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = createdWith.allocationUsage;
    if (createdWith.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocationCreateInfo.preferredFlags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    }
    if (vmaCreateBuffer(linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create IEBuffer!");
    }
    deletionQueue.emplace_back([&] {
        vmaDestroyBuffer(linkedRenderEngine->allocator, buffer, allocation);
    });
    if (createdWith.data != nullptr) {
        if (createdWith.sizeOfData > createdWith.size) {
            throw std::runtime_error("IEBuffer::CreateInfo::sizeOfData must not be greater than IEBuffer::CreateInfo::bufferSize.");
        }
        vmaMapMemory(linkedRenderEngine->allocator, allocation, &data);
        memcpy(data, createdWith.data, createdWith.sizeOfData);
        vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
    }
    if (createdWith.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
        bufferDeviceAddressInfo.buffer = buffer;
        deviceAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device.device, &bufferDeviceAddressInfo);
    }
    created = true;
}

void IEBuffer::uploadData(void *input, uint32_t sizeOfInput) {
    if (!created) { throw std::runtime_error("Calling IEBuffer::uploadData() on a IEBuffer for which IEBuffer::create() has not been called is illegal."); }
    if (sizeOfInput > createdWith.size) { throw std::runtime_error("sizeOfInput must not be greater than IEBuffer::CreateInfo::bufferSize."); }
    vmaMapMemory(linkedRenderEngine->allocator, allocation, &data);
    memcpy(data, input, sizeOfInput);
    vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
}

void IEBuffer::toImage(IEImage *image, uint32_t width, uint32_t height) {
    if (!created) {
        throw std::runtime_error("Calling IEBuffer::toImage() on a IEBuffer for which IEBuffer::create() has not been called is illegal.");
    }
    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = image->imageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || image->imageLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {width, height, 1};
    VkImageLayout oldLayout;
    if (image->imageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        oldLayout = image->imageLayout;
        image->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        vkCmdCopyBufferToImage((linkedRenderEngine->graphicsCommandPool)[0].commandBuffer, buffer, image->image, image->imageLayout, 1, &region);
        image->transitionLayout(oldLayout);
    } else {
        vkCmdCopyBufferToImage((linkedRenderEngine->graphicsCommandPool)[0].commandBuffer, buffer, image->image, image->imageLayout, 1, &region);
    }
}

void IEBuffer::toImage(IEImage *image) {
    if (!created) {
        throw std::runtime_error("Calling IEBuffer::toImage() on a IEBuffer for which IEBuffer::create() has not been called is illegal.");
    }
    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = image->imageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || image->imageLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {image->width, image->height, 1};
    VkImageLayout oldLayout;
    if (image->imageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        oldLayout = image->imageLayout;
        image->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        linkedRenderEngine->graphicsCommandPool[0].recordCopyBufferToImage(this, image, {region});
        image->transitionLayout(oldLayout);
    } else {
        linkedRenderEngine->graphicsCommandPool[0].recordCopyBufferToImage(this, image, {region});
    }
}

IEBuffer::~IEBuffer() {
    destroy(false);
}

IEBuffer::IEBuffer(IERenderEngine *engineLink, IEBuffer::CreateInfo *createInfo) {
    linkedRenderEngine = engineLink;
    createdWith = *createInfo;
    VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.size = createdWith.size;
    bufferCreateInfo.usage = createdWith.usage;
    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = createdWith.allocationUsage;
    if (createdWith.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        allocationCreateInfo.preferredFlags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    }
    if (vmaCreateBuffer(linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("failed to create IEBuffer!");
    }
    deletionQueue.emplace_back([&] {
        vmaDestroyBuffer(linkedRenderEngine->allocator, buffer, allocation);
    });
    if (createdWith.data != nullptr) {
        if (createdWith.sizeOfData > createdWith.size) {
            throw std::runtime_error("IEBuffer::CreateInfo::sizeOfData must not be greater than IEBuffer::CreateInfo::bufferSize.");
        }
        vmaMapMemory(linkedRenderEngine->allocator, allocation, &data);
        memcpy(data, createdWith.data, createdWith.sizeOfData);
        vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
    }
    if (createdWith.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
        VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
        bufferDeviceAddressInfo.buffer = buffer;
        deviceAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device.device, &bufferDeviceAddressInfo);
    }
    created = true;
}

IEBuffer::IEBuffer() = default;