#pragma once

#include "GraphicsModule/VulkanOpenGL/IEGraphicsLink.hpp"
#include "GraphicsModule/VulkanOpenGL/IEBuffer.hpp"
#include "GraphicsModule/VulkanOpenGL/IECommandPool.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif
#ifndef VMA_INCLUDED
#define VMA_INCLUDED
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#endif

#include <cmath>
#include <vulkan/vulkan.h>

enum IEMipLevels {
    IE_MIP_LEVEL_NONE = 0x0,
    IE_MIP_LEVEL_50_PERCENT = 0x50
};

class IEImage {
public:
    struct CreateInfo {
    public:
        //Required
        VkFormat format{};
        VkImageTiling tiling{};
        VkImageUsageFlags usage{};
        VmaMemoryUsage allocationUsage{};

        //Optional
        uint32_t width = 0, height = 0;

        //Optional; Only use for non-texture images
        /**@todo Remove this item.*/
        VkSampleCountFlagBits msaaSamples{VK_SAMPLE_COUNT_1_BIT};
        VkImageLayout imageLayout{VK_IMAGE_LAYOUT_UNDEFINED};
        VkImageType imageType{VK_IMAGE_TYPE_2D};
        IEBuffer *dataSource{};

        //Only use for texture images
        std::string filename{};
        float anisotropyLevel{};
        /**@todo Remove this item.*/
        bool mipMapping{false};
        stbi_uc *data{};
    };

    VkImage image{};
    VkImageView view{};
    VkSampler sampler{};
    VkFormat imageFormat{};
    VkImageLayout imageLayout{};
    uint32_t mipLevels{};
    CreateInfo createdWith{};

    void destroy() {
        for (std::function<void()> &function: deletionQueue) {
            function();
        }
        deletionQueue.clear();
    }

    virtual void create(CreateInfo *createInfo) {
        if (!linkedRenderEngine) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to create an image without a render engine!");
        }
    }

    virtual void create(IEGraphicsLink *engineLink, CreateInfo *createInfo) {
        if (engineLink) {  // Assume that this image is being recreated in a new engine, or created for the first time.
            destroy();  // Delete anything that was created in the context of the old engine
            linkedRenderEngine = engineLink;
        }
        createdWith = *createInfo;

        // Determine image data based on settings and input data
        auto maxMipLevel = static_cast<uint8_t>(std::floor(std::log2(std::max(createdWith.width, createdWith.height))) + 1);
        mipLevels = createdWith.mipMapping && linkedRenderEngine->settings.mipMapping ? std::min(std::max(maxMipLevel, static_cast<uint8_t>(1)), static_cast<uint8_t>(linkedRenderEngine->settings.mipMapLevel)) : 1;
        createdWith.width = createdWith.width == 0 ? static_cast<uint16_t>(linkedRenderEngine->swapchain.extent.width) : createdWith.width;
        createdWith.height = createdWith.height == 0 ? static_cast<uint16_t>(linkedRenderEngine->swapchain.extent.height) : createdWith.height;

        // Determine the number of MSAA samples to use
        uint8_t msaaSamples = 1;

        // Start with specified format, and an undefined layout.
        imageFormat = createdWith.format;
        imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        // Set up image create info.
        VkImageCreateInfo imageCreateInfo{
                .sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType=createdWith.imageType,
                .format=createdWith.format,
                .extent=VkExtent3D{
                        .width=createdWith.width,
                        .height=createdWith.height,
                        .depth=1
                },
                .mipLevels=1, // mipLevels, Unused due to no implementation of mip-mapping support yet.
                .arrayLayers=1,
                .samples=static_cast<VkSampleCountFlagBits>(msaaSamples),
                .tiling=createdWith.tiling,
                .usage=createdWith.usage,
                .sharingMode=VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
        };

        // Set up allocation create info
        VmaAllocationCreateInfo allocationCreateInfo{
                .usage=createdWith.allocationUsage,
        };

        // Create image
        if (vmaCreateImage(linkedRenderEngine->allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image!");
        }
        deletionQueue.emplace_back([&] {
            vmaDestroyImage(linkedRenderEngine->allocator, image, allocation);
        });

        // Set up image view create info.
        VkImageViewCreateInfo imageViewCreateInfo{
                .sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image=image,
                .viewType=VK_IMAGE_VIEW_TYPE_2D, /**@todo Add support for more than just 2D images.*/
                .format=createdWith.format,
                .components=VkComponentMapping{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},  // Unused. All components are mapped to default data.
                .subresourceRange=VkImageSubresourceRange{
                        .aspectMask=createdWith.format == linkedRenderEngine->swapchain.image_format ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT,
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
        if (createdWith.dataSource != nullptr) {
            transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            createdWith.dataSource->toImage(this, createdWith.width, createdWith.height);
        }

        // Set transition to requested layout from undefined or dst_optimal.
        if (createdWith.imageLayout != imageLayout) {
            transitionLayout(createdWith.imageLayout);
        }
    }

    [[maybe_unused]] void toBuffer(const IEBuffer &buffer) const {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = createdWith.format == linkedRenderEngine->swapchain.image_format ? VK_IMAGE_ASPECT_COLOR_BIT : VK_IMAGE_ASPECT_DEPTH_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {static_cast<uint32_t>(createdWith.width), static_cast<uint32_t>(createdWith.height), 1};
        vkCmdCopyImageToBuffer((*linkedRenderEngine->graphicsCommandPool)[0], image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer.buffer, 1, &region);
    }

    void transitionLayout(VkImageLayout newLayout) {
        if (imageLayout == newLayout) {
            return;
        }
        VkImageMemoryBarrier imageMemoryBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        imageMemoryBarrier.oldLayout = imageLayout;
        imageMemoryBarrier.newLayout = newLayout;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange.aspectMask = newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageMemoryBarrier.subresourceRange.levelCount = mipLevels;
        imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
        imageMemoryBarrier.subresourceRange.layerCount = 1;
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
        } else { throw std::runtime_error("Unknown transition parameters!"); }
        vkCmdPipelineBarrier((*linkedRenderEngine->graphicsCommandPool)[0], sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        imageLayout = newLayout;
    }

    virtual ~IEImage() {
        destroy();
    }

protected:
    IEGraphicsLink *linkedRenderEngine{};
    std::vector<std::function<void()>> deletionQueue{};
    VmaAllocation allocation{};
};

//@todo: Input image should have a layout of VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL!
void IEBuffer::toImage(IEImage *image, uint32_t width, uint32_t height) {
    if (!created) { throw std::runtime_error("Calling IEBuffer::toImage() on a IEBuffer for which IEBuffer::create() has not been called is illegal."); }
    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = image->imageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || image->imageLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {width, height, 1};
    VkImageLayout oldLayout;
    if (image->imageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        oldLayout = image->imageLayout;
        image->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        vkCmdCopyBufferToImage((*linkedRenderEngine->graphicsCommandPool)[0], buffer, image->image, image->imageLayout, 1, &region);
        image->transitionLayout(oldLayout);
    } else { vkCmdCopyBufferToImage((*linkedRenderEngine->graphicsCommandPool)[0], buffer, image->image, image->imageLayout, 1, &region); }
}

void IEBuffer::toImage(IEImage *image) {
    if (!created) { throw std::runtime_error("Calling IEBuffer::toImage() on a IEBuffer for which IEBuffer::create() has not been called is illegal."); }
    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = image->imageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || image->imageLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {image->createdWith.width, image->createdWith.height, 1};
    VkImageLayout oldLayout;
    if (image->imageLayout != VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        oldLayout = image->imageLayout;
        image->transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        vkCmdCopyBufferToImage((*linkedRenderEngine->graphicsCommandPool)[0], buffer, image->image, image->imageLayout, 1, &region);
        image->transitionLayout(oldLayout);
    } else { vkCmdCopyBufferToImage((*linkedRenderEngine->graphicsCommandPool)[0], buffer, image->image, image->imageLayout, 1, &region); }
}
