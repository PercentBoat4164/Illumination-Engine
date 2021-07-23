#pragma once

#include "vulkanImage.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "../../../deps/stb_image.h"
#endif

#include <fstream>
#include <cstring>

class VulkanTexture : public VulkanImage {
public:
    /**@todo: Combine as many command buffer submissions as possible together to reduce load on GPU.*/
    void create(VulkanGraphicsEngineLink *engineLink, CreateInfo *createInfo) override {
        createdWith = *createInfo;
        linkedRenderEngine = engineLink;
        int channels{};
        pixels = stbi_load(((std::string)createdWith.filename).c_str(), &createdWith.width, &createdWith.height, &channels, STBI_rgb_alpha);
        if (!pixels) { throw std::runtime_error(("failed to load texture image from file: " + (std::string)createdWith.filename).c_str()); }
        mipLevels = std::max((static_cast<uint32_t>(std::floor(std::log2(std::max(createdWith.width, createdWith.height)))) + 1) * createdWith.mipMapping, static_cast<uint32_t>(1));
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = createdWith.width;
        imageCreateInfo.extent.height = createdWith.height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = mipLevels;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = createdWith.format;
        imageCreateInfo.tiling = createdWith.tiling;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = createdWith.usage;
        imageCreateInfo.samples = createdWith.msaaSamples;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = createdWith.allocationUsage;
        vmaCreateImage(*linkedRenderEngine->allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr);
        deletionQueue.emplace_front([&]{ if(image != VK_NULL_HANDLE) { vmaDestroyImage(*linkedRenderEngine->allocator, image, allocation); image = VK_NULL_HANDLE; } });
        imageFormat = createdWith.format;
        imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = image;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = createdWith.format;
        imageViewCreateInfo.subresourceRange.aspectMask = createdWith.imageType == VulkanImageType::VULKAN_DEPTH ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = mipLevels;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(linkedRenderEngine->device->device, &imageViewCreateInfo, nullptr, &view) != VK_SUCCESS) { throw std::runtime_error("failed to create texture image view!"); }
        deletionQueue.emplace_front([&] { vkDestroyImageView(linkedRenderEngine->device->device, view, nullptr); view = VK_NULL_HANDLE; });
        transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        VulkanBuffer scratchBuffer{};
        VulkanBuffer::CreateInfo scratchBufferCreateInfo{static_cast<VkDeviceSize>(createdWith.width * createdWith.height * 4), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        memcpy(scratchBuffer.create(linkedRenderEngine, &scratchBufferCreateInfo), pixels, createdWith.width * createdWith.height * 4);
        scratchBuffer.toImage(image, createdWith.width, createdWith.height);
        /**@todo: Add support for more mipmap interpolation methods. Currently only linear interpolation is supported.*/
        if (createdWith.mipMapping) {
            VkFormatProperties formatProperties{};
            vkGetPhysicalDeviceFormatProperties(linkedRenderEngine->device->physical_device.physical_device, imageFormat, &formatProperties);
            if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) { throw std::runtime_error("texture image format " + std::to_string(imageFormat) + " does not support linear blitting!"); }
            VkCommandBuffer commandBuffer = linkedRenderEngine->beginSingleTimeCommands();
            VkImageMemoryBarrier imageMemoryBarrier{};
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
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
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
                vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);
                imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
                if (mipWidth > 1) { mipWidth /= 2; }
                if (mipHeight > 1) { mipHeight /= 2; }
            }
            linkedRenderEngine->endSingleTimeCommands(commandBuffer);
            transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = linkedRenderEngine->settings->anisotropicFilterLevel > 0 ? VK_TRUE : VK_FALSE;
        samplerInfo.maxAnisotropy = linkedRenderEngine->settings->anisotropicFilterLevel;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = -linkedRenderEngine->settings->mipMapLevel * 10;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        if (vkCreateSampler(linkedRenderEngine->device->device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) { throw std::runtime_error("failed to create texture sampler!"); }
        deletionQueue.emplace_front([&] { vkDestroySampler(linkedRenderEngine->device->device, sampler, nullptr); sampler = VK_NULL_HANDLE; });
        transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        scratchBuffer.destroy();
        stbi_image_free(pixels);
    }

private:
    stbi_uc *pixels{};
};