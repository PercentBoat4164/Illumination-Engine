#pragma once

#include "IEImage.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#include <cstddef>
#include <fstream>
#include <cstring>

#include <vulkan/vulkan.h>

class IETexture : public IEImage {
public:
    struct CreateInfo {
        VkFormat format{VK_FORMAT_R8G8B8A8_SRGB};
        VkImageLayout layout{VK_IMAGE_LAYOUT_UNDEFINED};
        VkImageType type{VK_IMAGE_TYPE_2D};
        VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};
        VkImageCreateFlags flags{};
        VmaMemoryUsage allocationUsage{};
        uint32_t width{}, height{};
        IEBuffer *dataSource{};
        stbi_uc *data{};
        std::string filename{};
    };

    virtual void copyCreateInfo(IETexture::CreateInfo *createInfo) {
        imageFormat = createInfo->format;
        imageLayout = createInfo->layout;
        imageType = createInfo->type;
        imageUsage = createInfo->usage;
        imageFlags = createInfo->flags;
        allocationUsage = createInfo->allocationUsage;
        width = createInfo->width;
        height = createInfo->height;
        dataSource = createInfo->dataSource;
        data = createInfo->data;
        filename = createInfo->filename;
    }

    virtual void create(IEGraphicsLink *engineLink, IETexture::CreateInfo *createInfo) {
        if (engineLink) {  // Assume that this image is being recreated in a new engine, or created for the first time.
            destroy();  // Delete anything that was created in the context of the old engine
            linkedRenderEngine = engineLink;
        }

        copyCreateInfo(createInfo);

        VkImageLayout desiredLayout = imageLayout;
        imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        /**@todo Implement mip-mapping again.*/
        // Determine image data based on settings and input data
//        auto maxMipLevel = static_cast<uint8_t>(std::floor(std::log2(std::max(width, height))) + 1);
//        mipLevels = mipMapping && linkedRenderEngine->settings.mipMapping ? std::min(std::max(maxMipLevel, static_cast<uint8_t>(1)), static_cast<uint8_t>(linkedRenderEngine->settings.mipMapLevel)) : 1;

        // Start with specified format, and an undefined layout.
        imageFormat = imageFormat;
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
                .tiling=VK_IMAGE_TILING_OPTIMAL,
                .usage=imageUsage,
                .sharingMode=VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
        };

        // Set up allocation create info
        VmaAllocationCreateInfo allocationCreateInfo{
                .usage=allocationUsage,
        };

        // Create image
        if (vmaCreateImage(linkedRenderEngine->allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create texture image!");
        }
        deletionQueue.emplace_back([&] {
            vmaDestroyImage(linkedRenderEngine->allocator, image, allocation);
        });

        // Set up image view create info.
        VkImageViewCreateInfo imageViewCreateInfo{
                .sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image=image,
                .viewType=VK_IMAGE_VIEW_TYPE_2D, /**@todo Add support for more than just 2D images.*/
                .format=imageFormat,
                .components=VkComponentMapping{VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY},  // Unused. All components are mapped to default data.
                .subresourceRange=VkImageSubresourceRange{
                        .aspectMask=VK_IMAGE_ASPECT_COLOR_BIT,
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

        // Determine the anisotropy level to use.
//        anisotropyLevel = linkedRenderEngine->settings.anisotropicFilterLevel > 0 * std::min(anisotropyLevel, linkedRenderEngine->device.physical_device.properties.limits.maxSamplerAnisotropy);

        // Set up image sampler create info.
        VkSamplerCreateInfo samplerCreateInfo{
                .sType=VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                .magFilter=VK_FILTER_NEAREST,
                .minFilter=VK_FILTER_NEAREST,
                .mipmapMode=VK_SAMPLER_MIPMAP_MODE_LINEAR,
                .addressModeU=VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeV=VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .addressModeW=VK_SAMPLER_ADDRESS_MODE_REPEAT,
                .mipLodBias=linkedRenderEngine->settings.mipMapLevel,
//                .anisotropyEnable=linkedRenderEngine->settings.anisotropicFilterLevel > 0,
//                .maxAnisotropy=anisotropyLevel,
                .compareEnable=VK_FALSE,
                .compareOp=VK_COMPARE_OP_ALWAYS,
                .minLod=0.0f,
//                .maxLod=static_cast<float>(mipLevels),
                .borderColor=VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
                .unnormalizedCoordinates=VK_FALSE,
        };

        // Create image sampler
        if (vkCreateSampler(linkedRenderEngine->device.device, &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
        deletionQueue.emplace_back([&] {
            if (sampler) {
                vkDestroySampler(linkedRenderEngine->device.device, sampler, nullptr);
                sampler = VK_NULL_HANDLE;
            }
        });

        // Upload data if provided
        if (data != nullptr && dataSource != nullptr) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempt to create image with raw and buffered data!");
        }
        if (data != nullptr) {
            upload(data);
        }
        if (dataSource != nullptr) {
            upload(dataSource);
        }

        // Set transition to requested layout from undefined or dst_optimal.
        if (imageLayout != desiredLayout) {
            transitionLayout(desiredLayout);
        }
    }

    void upload(void *data) {
        IEBuffer scratchBuffer{};
        IEBuffer::CreateInfo scratchBufferCreateInfo{};
        scratchBufferCreateInfo.size = static_cast<VkDeviceSize>(width * height) * 4;
        scratchBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        scratchBufferCreateInfo.allocationUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        scratchBuffer.create(linkedRenderEngine, &scratchBufferCreateInfo);
        scratchBuffer.uploadData(data, width * height * 4);
        transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        scratchBuffer.toImage(this);
    }

    void upload(IEBuffer *data) {
        transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        data->toImage(this);
    }

    void generateMipMaps() {
//        if (mipMapping) {
//            VkFormatProperties formatProperties{};
//            vkGetPhysicalDeviceFormatProperties(linkedRenderEngine->device.physical_device.physical_device, imageFormat, &formatProperties);
//            if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) { throw std::runtime_error("texture image properties " + std::to_string(imageFormat) + " does not support linear blitting!"); }
//            VkImageMemoryBarrier imageMemoryBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
//            imageMemoryBarrier.image = image;
//            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
//            imageMemoryBarrier.subresourceRange.layerCount = 1;
//            imageMemoryBarrier.subresourceRange.levelCount = 1;
//            int32_t mipWidth{static_cast<int32_t>(width)}, mipHeight{static_cast<int32_t>(height)};
//            for (uint32_t i = 1; i < mipLevels; ++i) {
//                imageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
//                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//                vkCmdPipelineBarrier((*linkedRenderEngine->graphicsCommandPool)[0], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
//                VkImageBlit imageBlit{};
//                imageBlit.srcOffsets[0] = {0, 0, 0};
//                imageBlit.srcOffsets[1] = {mipWidth, mipHeight, 1};
//                imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//                imageBlit.srcSubresource.mipLevel = i - 1;
//                imageBlit.srcSubresource.baseArrayLayer = 0;
//                imageBlit.srcSubresource.layerCount = 1;
//                imageBlit.dstOffsets[0] = {0, 0, 0};
//                imageBlit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
//                imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//                imageBlit.dstSubresource.mipLevel = i;
//                imageBlit.dstSubresource.baseArrayLayer = 0;
//                imageBlit.dstSubresource.layerCount = 1;
//                vkCmdBlitImage((*linkedRenderEngine->graphicsCommandPool)[0], image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
//                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
//                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
//                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//                vkCmdPipelineBarrier((*linkedRenderEngine->graphicsCommandPool)[0], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
//                if (mipWidth > 1) {
//                    mipWidth /= 2;
//                }
//                if (mipHeight > 1) {
//                    mipHeight /= 2;
//                }
//            }
//            imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
//            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
//            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//            vkCmdPipelineBarrier((*linkedRenderEngine->graphicsCommandPool)[0], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
//            imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//        }
//        transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//
//        // This may cause problems down the line as other things may be using command buffer 0.
//        linkedRenderEngine->graphicsCommandPool->executeCommandBuffer(0);
//        linkedRenderEngine->graphicsCommandPool->recordCommandBuffer(0);
    }
};