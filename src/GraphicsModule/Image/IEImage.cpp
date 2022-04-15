/* Include this file's header. */
#include "IEImage.hpp"

/* Include dependencies within this module. */
#include "Buffer/IEBuffer.hpp"
#include "IERenderEngine.hpp"

/* Include dependencies from Core. */
#include "Core/LogModule/IELogger.hpp"


[[maybe_unused]] [[nodiscard]] uint8_t IEImage::getHighestMSAASampleCount(uint8_t requested) const {
    uint8_t count = 1;
    VkImageFormatProperties properties{};
    vkGetPhysicalDeviceImageFormatProperties(linkedRenderEngine->device.physical_device, imageFormat, imageType, imageMemoryArrangement, imageUsage, imageFlags, &properties);
    return std::max(count, std::min(static_cast<uint8_t>(properties.sampleCounts), std::min(requested, linkedRenderEngine->settings->msaaSamples)));
}

[[maybe_unused]] [[nodiscard]] float IEImage::getHighestAnisotropyLevel(float requested) const {
    float anisotropyLevel = 1.0F;
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(linkedRenderEngine->device.physical_device, &properties);
    return std::max(anisotropyLevel, std::min(properties.limits.maxSamplerAnisotropy, requested));
}

void IEImage::destroy() {
    if (hasNoDependents()) {
        removeAllDependents();
        for (std::function<void()> &function: deletionQueue) {
            function();
        }
        deletionQueue.clear();
    }
}

void IEImage::copyCreateInfo(IEImage::CreateInfo *createInfo) {
    imageFormat = createInfo->format;
    imageLayout = createInfo->layout;
    imageType = createInfo->type;
    imageUsage = createInfo->usage;
    imageFlags = createInfo->flags;
    imageAspect = createInfo->aspect;
    allocationUsage = createInfo->allocationUsage;
    width = createInfo->width;
    height = createInfo->height;
    dataSource = createInfo->dataSource;
    data = createInfo->data;
}

void IEImage::create(IERenderEngine *engineLink, IEImage::CreateInfo *createInfo) {
    linkedRenderEngine = engineLink;

    // Copy createInfo data into this image
    copyCreateInfo(createInfo);

    VkImageLayout desiredLayout = imageLayout;
    imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Set up image create info.
    VkImageCreateInfo imageCreateInfo{
            .sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType=imageType,
            .format=imageFormat,
            .extent=VkExtent3D{
                    .width=width,
                    .height=height,
                    .depth=1
            },
            .mipLevels=1, // mipLevels, Unused due to no implementation of mip-mapping support yet.
            .arrayLayers=1,
            .samples=static_cast<VkSampleCountFlagBits>(1),
            .tiling=imageMemoryArrangement,
            .usage=imageUsage,
            .sharingMode=VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
    };

    // Set up allocation create info
    VmaAllocationCreateInfo allocationCreateInfo{
            .usage=allocationUsage,
    };

    if (height == 0) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Image height is zero! This may cause Vulkan to fail to create an image.");
    }
    if (width == 0) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Image width is zero! This may cause Vulkan to fail to create an image.");
    }

    // Create image
    if (vmaCreateImage(linkedRenderEngine->allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create image!");
    }
    deletionQueue.emplace_back([&] {
        vmaDestroyImage(linkedRenderEngine->allocator, image, allocation);
    });

    // Set up image view create info
    VkImageViewCreateInfo imageViewCreateInfo{
            .sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image=image,
            .viewType=VK_IMAGE_VIEW_TYPE_2D, /**@todo Add support for more than just 2D images.*/
            .format=imageFormat,
            .components=VkComponentMapping{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},  // Unused. All components are mapped to default data.
            .subresourceRange=VkImageSubresourceRange{
                    .aspectMask=imageAspect,
                    .baseMipLevel=0,
                    .levelCount=1,  // Unused. Mip-mapping is not yet implemented.
                    .baseArrayLayer=0,
                    .layerCount=1,
            },
    };

    // Create image view
    if (vkCreateImageView(linkedRenderEngine->device.device, &imageViewCreateInfo, nullptr, &view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    deletionQueue.emplace_back([&] {
        vkDestroyImageView(linkedRenderEngine->device.device, view, nullptr);
    });

    // Upload data if provided
    if (dataSource != nullptr) {
        transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        dataSource->toImage(this, width, height);
    }

    // Set transition to requested layout from undefined or dst_optimal.
    if (imageLayout != desiredLayout) {
        transitionLayout(desiredLayout);
    }
}

void IEImage::toBuffer(const IEBuffer &buffer) const {
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = imageFormat == linkedRenderEngine->swapchain.image_format ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    vkCmdCopyImageToBuffer((linkedRenderEngine->graphicsCommandPool)[0].commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer.buffer, 1, &region);
}

void IEImage::transitionLayout(VkImageLayout newLayout) {
    if (imageLayout == newLayout) {
        return;
    }
    if (newLayout == VK_IMAGE_LAYOUT_UNDEFINED) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to transition to an undefined layout (VK_IMAGE_LAYOUT_UNDEFINED)!");
        return;
    }
    IEImageMemoryBarrier imageMemoryBarrier{
            .newLayout=newLayout,
            .srcQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex=VK_QUEUE_FAMILY_IGNORED,
            .image=this,
            .subresourceRange={
                    .aspectMask=static_cast<VkImageAspectFlags>(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT),
                    .baseMipLevel=0,
                    .levelCount=1,  // Will be used for mip mapping in the future
                    .layerCount=1
            }
    };
    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;
    if (imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL | newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else if (imageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (imageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (imageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        imageMemoryBarrier.srcAccessMask = 0;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to transition with unknown parameters!");
        return;
    }
    linkedRenderEngine->graphicsCommandPool[0].recordPipelineBarrier(sourceStage, destinationStage, 0, {}, {}, {imageMemoryBarrier});
    imageLayout = newLayout;
}

IEImage::~IEImage() {
    destroy();
}

void IEImage::addDependent(void *dependent) {
    dependents.push_back(dependent);
}

void IEImage::removeDependent(void *dependent) {
    for (int i = 0; i < dependents.size(); ++i) {
        if (dependents[i] == dependent) {
            dependents.erase(dependents.begin() + i);
            return;
        }
    }
}

IEImage::IEImage(IERenderEngine *engineLink, IEImage::CreateInfo *createInfo) {
    linkedRenderEngine = engineLink;

    // Copy createInfo data into this image
    copyCreateInfo(createInfo);
    VkImageLayout desiredLayout = imageLayout;
    imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Set up image create info.
    VkImageCreateInfo imageCreateInfo{
            .sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType=imageType,
            .format=imageFormat,
            .extent=VkExtent3D{
                    .width=width,
                    .height=height,
                    .depth=1
            },
            .mipLevels=1, // mipLevels, Unused due to no implementation of mip-mapping support yet.
            .arrayLayers=1,
            .samples=static_cast<VkSampleCountFlagBits>(1),
            .tiling=imageMemoryArrangement,
            .usage=imageUsage,
            .sharingMode=VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
    };

    // Set up allocation create info
    VmaAllocationCreateInfo allocationCreateInfo{
            .usage=allocationUsage,
    };

    if (height == 0) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Image height is zero! This may cause Vulkan to fail to create an image.");
    }
    if (width == 0) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Image width is zero! This may cause Vulkan to fail to create an image.");
    }

    // Create image
    if (vmaCreateImage(linkedRenderEngine->allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create image!");
    }
    deletionQueue.emplace_back([&] {
        vmaDestroyImage(linkedRenderEngine->allocator, image, allocation);
    });

    // Set up image view create info
    VkImageViewCreateInfo imageViewCreateInfo{
            .sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image=image,
            .viewType=VK_IMAGE_VIEW_TYPE_2D, /**@todo Add support for more than just 2D images.*/
            .format=imageFormat,
            .components=VkComponentMapping{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},  // Unused. All components are mapped to default data.
            .subresourceRange=VkImageSubresourceRange{
                    .aspectMask=imageAspect,
                    .baseMipLevel=0,
                    .levelCount=1,  // Unused. Mip-mapping is not yet implemented.
                    .baseArrayLayer=0,
                    .layerCount=1,
            },
    };

    // Create image view
    if (vkCreateImageView(linkedRenderEngine->device.device, &imageViewCreateInfo, nullptr, &view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    deletionQueue.emplace_back([&] {
        vkDestroyImageView(linkedRenderEngine->device.device, view, nullptr);
    });

    // Upload data if provided
    if (dataSource != nullptr) {
        transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        dataSource->toImage(this, width, height);
    }

    // Set transition to requested layout from undefined or dst_optimal.
    if (imageLayout != desiredLayout) {
        transitionLayout(desiredLayout);
    }
}

IEImage::IEImage() = default;