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
    void create(IEGraphicsLink *engineLink, CreateInfo *createInfo) override {
        createdWith = *createInfo;
        linkedRenderEngine = engineLink;
        mipLevels = std::max((static_cast<uint32_t>(std::floor(std::log2(std::max(createdWith.width, createdWith.height)))) + 1) * createdWith.mipMapping, static_cast<uint32_t>(1));
        imageFormat = createdWith.format;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = createdWith.width;
        imageCreateInfo.extent.height = createdWith.height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = mipLevels;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = createdWith.format;
        imageCreateInfo.tiling = createdWith.tiling;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = createdWith.usage | (linkedRenderEngine->settings.mipMapping ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0);
        imageCreateInfo.samples = createdWith.msaaSamples;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        allocationCreateInfo.usage = createdWith.allocationUsage;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = createdWith.format;
        imageViewCreateInfo.subresourceRange.aspectMask = createdWith.imageType == VulkanImageType::VULKAN_DEPTH ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = mipLevels;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        if (linkedRenderEngine->device.physical_device.features.samplerAnisotropy) {
            samplerInfo.anisotropyEnable = linkedRenderEngine->settings.anisotropicFilterLevel > 0 ? VK_TRUE : VK_FALSE;
            samplerInfo.maxAnisotropy = std::min(linkedRenderEngine->settings.anisotropicFilterLevel, linkedRenderEngine->device.physical_device.properties.limits.maxSamplerAnisotropy);
        }
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = linkedRenderEngine->settings.mipMapLevel;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
    }

    /**@todo Combine as many command IEBuffer submissions as possible together to reduce prepare on GPU and CPU.*/
    /**@todo Allow either dataSource input or bufferData input from the CreateInfo. Currently is only bufferData for texture and only dataSource for other.*/
    void upload() override {
        if (vmaCreateImage(linkedRenderEngine->allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr) != VK_SUCCESS) { throw std::runtime_error("failed to create texture image!"); }
        deletionQueue.emplace_back([&] { vmaDestroyImage(linkedRenderEngine->allocator, image, allocation); });
        imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageViewCreateInfo.image = image;
        if (vkCreateImageView(linkedRenderEngine->device.device, &imageViewCreateInfo, nullptr, &view) != VK_SUCCESS) { throw std::runtime_error("failed to create texture image view!"); }
        deletionQueue.emplace_back([&] { vkDestroyImageView(linkedRenderEngine->device.device, view, nullptr);});
        if (vkCreateSampler(linkedRenderEngine->device.device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) { throw std::runtime_error("failed to create texture sampler!"); }
        deletionQueue.emplace_back([&] { vkDestroySampler(linkedRenderEngine->device.device, sampler, nullptr); sampler = VK_NULL_HANDLE; });
        IEBuffer scratchBuffer{};
        IEBuffer::CreateInfo scratchBufferCreateInfo{};
        scratchBufferCreateInfo.size = static_cast<VkDeviceSize>(createdWith.width * createdWith.height) * 4;
        scratchBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        scratchBufferCreateInfo.allocationUsage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        scratchBuffer.create(linkedRenderEngine, &scratchBufferCreateInfo);
        scratchBuffer.uploadData(createdWith.data, createdWith.width * createdWith.height * 4);
        transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        scratchBuffer.toImage(*this, createdWith.width, createdWith.height);
        /**@todo Add support for more mipmap interpolation methods. Currently only linear interpolation is supported.*/
        if (createdWith.mipMapping) {
            VkFormatProperties formatProperties{};
            vkGetPhysicalDeviceFormatProperties(linkedRenderEngine->device.physical_device.physical_device, imageFormat, &formatProperties);
            if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) { throw std::runtime_error("texture image properties " + std::to_string(imageFormat) + " does not support linear blitting!"); }
            VkImageMemoryBarrier imageMemoryBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
            imageMemoryBarrier.image = image;
            imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            imageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
            imageMemoryBarrier.subresourceRange.layerCount = 1;
            imageMemoryBarrier.subresourceRange.levelCount = 1;
            int32_t mipWidth{createdWith.width}, mipHeight{createdWith.height};
            for (uint32_t i = 1; i < mipLevels; ++i) {
                imageMemoryBarrier.subresourceRange.baseMipLevel = i - 1;
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                vkCmdPipelineBarrier((*linkedRenderEngine->computeCommandPool)[0], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
                VkImageBlit imageBlit{};
                imageBlit.srcOffsets[0] = {0, 0, 0};
                imageBlit.srcOffsets[1] = {mipWidth, mipHeight, 1};
                imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageBlit.srcSubresource.mipLevel = i - 1;
                imageBlit.srcSubresource.baseArrayLayer = 0;
                imageBlit.srcSubresource.layerCount = 1;
                imageBlit.dstOffsets[0] = {0, 0, 0};
                imageBlit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
                imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                imageBlit.dstSubresource.mipLevel = i;
                imageBlit.dstSubresource.baseArrayLayer = 0;
                imageBlit.dstSubresource.layerCount = 1;
                vkCmdBlitImage((*linkedRenderEngine->computeCommandPool)[0], image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                vkCmdPipelineBarrier((*linkedRenderEngine->computeCommandPool)[0], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
                if (mipWidth > 1) { mipWidth /= 2; }
                if (mipHeight > 1) { mipHeight /= 2; }
            }
            imageMemoryBarrier.subresourceRange.baseMipLevel = mipLevels - 1;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier((*linkedRenderEngine->computeCommandPool)[0], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
            imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        scratchBuffer.destroy();
    }
};