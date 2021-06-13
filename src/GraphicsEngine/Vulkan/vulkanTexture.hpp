#pragma once

#include "vulkanImage.hpp"

#include "../../../deps/stb_image.h"

#include <fstream>
#include <cstring>

class Texture : public Image {
public:
    void create(VulkanGraphicsEngineLink *engineLink, CreateInfo *createInfo) override {
        createdWith = *createInfo;
        linkedRenderEngine = engineLink;
        int channels{};
        pixels = stbi_load(((std::string)createdWith.filename).c_str(), &createdWith.width, &createdWith.height, &channels, STBI_rgb_alpha);
        if (!pixels) { throw std::runtime_error(("failed to load texture image from file: " + (std::string)createdWith.filename).c_str()); }
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = createdWith.width;
        imageCreateInfo.extent.height = createdWith.height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = createdWith.mipLevels;
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
        imageViewCreateInfo.subresourceRange.aspectMask = createdWith.imageType == ImageType::DEPTH ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(linkedRenderEngine->device->device, &imageViewCreateInfo, nullptr, &view) != VK_SUCCESS) { throw std::runtime_error("failed to create texture image view!"); }
        deletionQueue.emplace_front([&] { vkDestroyImageView(linkedRenderEngine->device->device, view, nullptr); view = VK_NULL_HANDLE; });
        transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        Buffer scratchBuffer{};
        Buffer::CreateInfo scratchBufferCreateInfo{static_cast<VkDeviceSize>(createdWith.width * createdWith.height * 4), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        memcpy(scratchBuffer.create(linkedRenderEngine, &scratchBufferCreateInfo), pixels, createdWith.width * createdWith.height * 4);
        scratchBuffer.toImage(image, createdWith.width, createdWith.height);
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
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        if (vkCreateSampler(linkedRenderEngine->device->device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) { throw std::runtime_error("failed to create texture sampler!"); }
        deletionQueue.emplace_front([&] { vkDestroySampler(linkedRenderEngine->device->device, sampler, nullptr); sampler = VK_NULL_HANDLE; });
        transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        scratchBuffer.destroy();
    }

private:
    stbi_uc *pixels{};
};