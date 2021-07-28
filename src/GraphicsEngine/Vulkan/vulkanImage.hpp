#pragma once

#include "vulkanBuffer.hpp"

enum VulkanImageType {
    VULKAN_DEPTH = 0x00000000,
    VULKAN_COLOR = 0x00000001,
    VULKAN_TEXTURE = 0x00000002
};

class VulkanImage {
public:
    struct CreateInfo {
        //Required
        VkFormat format{};
        VkImageTiling tiling{};
        VkImageUsageFlags usage{};
        VmaMemoryUsage allocationUsage{};

        //Optional
        int width = 0, height = 0;                                  //OPTIONAL - will default to swapchain size for non-texture images

        //Optional; Only use for non-texture images
        VkSampleCountFlagBits msaaSamples{VK_SAMPLE_COUNT_1_BIT};
        VkImageLayout imageLayout{VK_IMAGE_LAYOUT_UNDEFINED};
        VulkanImageType imageType{};
        VulkanBuffer *dataSource{};

        //Only use for texture images
        const char *filename{};
        bool mipMapping{false};
    };

    VkImage image{};
    VkImageView view{};
    VkSampler sampler{};
    VkFormat imageFormat{};
    VkImageLayout imageLayout{};
    uint32_t mipLevels{};
    CreateInfo createdWith{};

    void destroy() {
        for (std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

    virtual void create(VulkanGraphicsEngineLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        mipLevels = std::max((static_cast<uint32_t>(std::floor(std::log2(std::max(createdWith.width, createdWith.height)))) + 1) * createdWith.mipMapping, static_cast<uint32_t>(1));
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = createdWith.width == 0 ? linkedRenderEngine->swapchain->extent.width : createdWith.width;
        imageCreateInfo.extent.height = createdWith.height == 0 ? linkedRenderEngine->swapchain->extent.height : createdWith.height;
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
        deletionQueue.emplace_front([&] { vmaDestroyImage(*linkedRenderEngine->allocator, image, allocation); });
        imageFormat = createdWith.format;
        imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = image;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = createdWith.format;
        imageViewCreateInfo.subresourceRange.aspectMask = createdWith.imageType == VulkanImageType::VULKAN_DEPTH ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(linkedRenderEngine->device->device, &imageViewCreateInfo, nullptr, &view) != VK_SUCCESS) { throw std::runtime_error("failed to create texture image view!"); }
        deletionQueue.emplace_front([&] { vkDestroyImageView(linkedRenderEngine->device->device, view, nullptr);});
        if (createdWith.dataSource != nullptr) {
            transitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
            createdWith.dataSource->toImage(*this, createdWith.width, createdWith.height);
        }
        if (createdWith.imageLayout != imageLayout) { transitionLayout(createdWith.imageLayout); }
    }

    void toBuffer(const VulkanBuffer &buffer, uint32_t width, uint32_t height, VkCommandBuffer commandBuffer = nullptr) const {
        bool noCommandBuffer{false};
        if (commandBuffer == nullptr) { noCommandBuffer = true; }
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = createdWith.imageType == VULKAN_DEPTH ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};
        if (noCommandBuffer) { commandBuffer = linkedRenderEngine->beginSingleTimeCommands(); }
        vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer.buffer, 1, &region);
        if (noCommandBuffer) { linkedRenderEngine->endSingleTimeCommands(commandBuffer); }
    }

    void transitionLayout(VkImageLayout newLayout, VkCommandBuffer commandBuffer = nullptr) {
        if (imageLayout == newLayout) { return; }
        bool noCommandBuffer{false};
        if (commandBuffer == nullptr) { noCommandBuffer = true; }
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
        VkPipelineStageFlags sourceStage{};
        VkPipelineStageFlags destinationStage{};
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
        } else if (imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else if (imageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) {
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
        } else { throw std::runtime_error("Unknown transfer parameters!"); }
        if (noCommandBuffer) { commandBuffer = linkedRenderEngine->beginSingleTimeCommands(); }
        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
        if (noCommandBuffer) { linkedRenderEngine->endSingleTimeCommands(commandBuffer); }
        imageLayout = newLayout;
    }

protected:
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    VmaAllocation allocation{};
    std::deque<std::function<void()>> deletionQueue{};
};

void VulkanBuffer::toImage(const VulkanImage &image, uint32_t width, uint32_t height, VkCommandBuffer commandBuffer) const {
    if (!created) { throw std::runtime_error("Calling VulkanBuffer::toImage() on a buffer for which VulkanBuffer::create() has not been called is illegal."); }
    bool noCommandBuffer{commandBuffer == VK_NULL_HANDLE};
    VkBufferImageCopy region{};
    region.imageSubresource.aspectMask = image.imageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || image.imageLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;;
    region.imageSubresource.layerCount = 1;
    region.imageExtent = {width, height, 1};
    if (noCommandBuffer) { commandBuffer = linkedRenderEngine->beginSingleTimeCommands(); }
    vkCmdCopyBufferToImage(commandBuffer, buffer, image.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    if (noCommandBuffer) { linkedRenderEngine->endSingleTimeCommands(commandBuffer); }
}