#pragma once

#include "AssetLinking.hpp"

class Asset {

public:
    RenderEngineLink *linkedRenderEngine{};
    VkImageView textureImageView{};
    VkImage textureImage{};
    VkSampler textureSampler{};
    AllocatedBuffer vertexBuffer{};
    AllocatedBuffer indexBuffer{};
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices;
    std::vector<VkBuffer> vertexBuffers{};
    std::vector<VkBuffer> indexBuffers{};


    explicit Asset(RenderEngineLink *linkRenderEngine) {
        linkedRenderEngine = linkRenderEngine;
    }

    [[nodiscard]] std::vector<VkCommandBuffer> beginSingleTimeCommands(size_t count) const {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = *linkedRenderEngine->commandPool;
        allocInfo.commandBufferCount = count;
        std::vector<VkCommandBuffer> singleTimeCommandBuffers{count};
        vkAllocateCommandBuffers(*linkedRenderEngine->logicalDevice, &allocInfo, singleTimeCommandBuffers.data());
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        for (VkCommandBuffer singleTimeCommandBuffer : singleTimeCommandBuffers) {vkBeginCommandBuffer(singleTimeCommandBuffer, &beginInfo);}
        return singleTimeCommandBuffers;
    }
    void endSingleTimeCommands(std::vector<VkCommandBuffer> singleTimeCommandBuffers) const {
        for (VkCommandBuffer singleTimeCommandBuffer : singleTimeCommandBuffers) {vkEndCommandBuffer(singleTimeCommandBuffer);}
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = singleTimeCommandBuffers.size();
        submitInfo.pCommandBuffers = singleTimeCommandBuffers.data();
        vkQueueSubmit(*linkedRenderEngine->singleTimeQueue, singleTimeCommandBuffers.size(), &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(*linkedRenderEngine->singleTimeQueue);
        vkFreeCommandBuffers(*linkedRenderEngine->logicalDevice, *linkedRenderEngine->commandPool, singleTimeCommandBuffers.size(), singleTimeCommandBuffers.data());
    }

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        VkImageView imageView;
        if (vkCreateImageView(*linkedRenderEngine->logicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {throw std::runtime_error("failed to create image view!");}
        (*linkedRenderEngine->deletionQueue).emplace_back([&]{vkDestroyImageView(*linkedRenderEngine->logicalDevice, imageView, nullptr);});
        return imageView;
    }


    Asset load(const std::string& filename, const std::string& modelExtension, const std::string& textureExtension) {
        int texWidth{}, texHeight{}, texChannels{};
        stbi_uc* image{};
        stbi_uc* normal{};
        stbi_uc* herm{};
        std::string modelFilename = filename + modelExtension;
        std::vector<std::string> filenames {filename + textureExtension};//, filename + "_normal" + textureExtension, filename + "_herm" + textureExtension};
        //Import model
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelFilename.c_str())) {throw std::runtime_error(warn + err);}
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto &shape : shapes) {
            for (const auto &index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.pos = {attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1],attrib.vertices[3 * index.vertex_index + 2]};
                vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index + 0],1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};
                vertex.color = {1.0f, 1.0f, 1.0f};
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        //Import textures
        for (const std::string& thisFilename : filenames) {
            image = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            if (!image) {
                std::cerr << "failed to load texture image: " + thisFilename << std::endl;
                continue;
            }
            void *imagePtr = image;
            VkDeviceSize imageSize = texWidth * texHeight * 4;
            VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
            //Put image data into staging buffer
            AllocatedBuffer stagingBuffer{};
            VkBufferCreateInfo stagingBufferCreateInfo{};
            stagingBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            stagingBufferCreateInfo.size = imageSize;
            stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
            VmaAllocationCreateInfo vmaAllocInfo{};
            vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
            vmaCreateBuffer(*linkedRenderEngine->allocator, &stagingBufferCreateInfo, &vmaAllocInfo, &stagingBuffer.buffer, &stagingBuffer.allocation, nullptr);
            void *data;
            vmaMapMemory(*linkedRenderEngine->allocator, stagingBuffer.allocation, &data);
            memcpy(data, imagePtr, imageSize);
            vmaUnmapMemory(*linkedRenderEngine->allocator, stagingBuffer.allocation);
            //free stbi image
            stbi_image_free(image);
            //Allocate space for image on GPU
            VkExtent3D imageExtent;
            imageExtent.width = texWidth;
            imageExtent.height = texHeight;
            imageExtent.depth = 1;
            VkImageCreateInfo imageCreateInfo{};
            AllocatedImage texture{};
            vmaAllocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            vmaCreateImage(*linkedRenderEngine->allocator, &imageCreateInfo, &vmaAllocInfo, &texture.image, &texture.allocation, nullptr);
            //Transition image to new layout
            int mipLevels = (int)std::floor(std::log2(std::max(texWidth, texHeight))) + 1;
            std::vector<VkCommandBuffer> imageCommandBuffers = beginSingleTimeCommands(3);
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = textureImage;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = mipLevels;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            vkCmdPipelineBarrier(imageCommandBuffers[0], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            //Copy to space allocated on GPU
            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageExtent = imageExtent;
            vkCmdCopyBufferToImage(imageCommandBuffers[1], stagingBuffer.buffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
//            vmaDestroyBuffer(allocator, stagingBuffer.buffer, stagingBuffer.allocation);
            //Create mipmaps
            VkFormatProperties physicalDeviceFormatProperties;
            vkGetPhysicalDeviceFormatProperties(*linkedRenderEngine->physicalDevice, VK_FORMAT_R8G8B8A8_SRGB, &physicalDeviceFormatProperties);
            if (!(physicalDeviceFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {throw std::runtime_error("texture image format does not support linear blitting!");}
            barrier.subresourceRange.levelCount = 0;
            int mipWidth = texWidth;
            int mipHeight = texHeight;
            for (uint32_t i = 1; i < (*linkedRenderEngine->settings).mipLevels; i++) {
                barrier.subresourceRange.baseMipLevel = i - 1;
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                vkCmdPipelineBarrier(imageCommandBuffers[2], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
                VkImageBlit blit{};
                blit.srcOffsets[0] = {0, 0, 0};
                blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
                blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.srcSubresource.mipLevel = i - 1;
                blit.srcSubresource.baseArrayLayer = 0;
                blit.srcSubresource.layerCount = 1;
                blit.dstOffsets[0] = {0, 0, 0};
                blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
                blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                blit.dstSubresource.mipLevel = i;
                blit.dstSubresource.baseArrayLayer = 0;
                blit.dstSubresource.layerCount = 1;
                vkCmdBlitImage(imageCommandBuffers[2], textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);
                barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                vkCmdPipelineBarrier(imageCommandBuffers[2], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
                if (mipWidth > 1) {mipWidth /= 2;}
                if (mipHeight > 1) {mipHeight /= 2;}
            }
            barrier.subresourceRange.baseMipLevel = (*linkedRenderEngine->settings).mipLevels - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            vkCmdPipelineBarrier(imageCommandBuffers[2], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
            endSingleTimeCommands(imageCommandBuffers);
            //Create image view
            textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, (*linkedRenderEngine->settings).mipLevels);
            //Create image sampler
            VkSamplerCreateInfo samplerInfo{};
            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            if ((*linkedRenderEngine->settings).anisotropicFilterLevel > 1) {samplerInfo.anisotropyEnable = VK_TRUE;}
            else {samplerInfo.anisotropyEnable = VK_FALSE;}
            VkPhysicalDeviceProperties physicalDeviceProperties{};
            vkGetPhysicalDeviceProperties(*linkedRenderEngine->physicalDevice, &physicalDeviceProperties);
            samplerInfo.maxAnisotropy = physicalDeviceProperties.limits.maxSamplerAnisotropy;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.maxLod = (float)mipLevels;
            if (vkCreateSampler(*linkedRenderEngine->logicalDevice, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {throw std::runtime_error("failed to create texture sampler!");}
            (*linkedRenderEngine->deletionQueue).emplace_back([&]{vkDestroySampler(*linkedRenderEngine->logicalDevice, textureSampler, nullptr);});
        }
        //Create index buffer
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.size = sizeof(indices[0]) * indices.size();
        bufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        VmaAllocationCreateInfo vmaAllocInfo{};
        vmaAllocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
        vmaCreateBuffer(*linkedRenderEngine->allocator, &bufferCreateInfo, &vmaAllocInfo, &indexBuffer.buffer, &indexBuffer.allocation, nullptr);
        indexBuffers.push_back(indexBuffer.buffer);
        //Create vertex buffer
        bufferCreateInfo.size = sizeof(vertices[0]) * vertices.size();
        bufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        vmaCreateBuffer(*linkedRenderEngine->allocator, &bufferCreateInfo, &vmaAllocInfo, &vertexBuffer.buffer, &vertexBuffer.allocation, nullptr);
        vertexBuffers.push_back(vertexBuffer.buffer);
        //Create descriptor sets
        std::vector<VkDescriptorSetLayout> layouts((*linkedRenderEngine->swapChainImages).size(), *linkedRenderEngine->descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = *linkedRenderEngine->descriptorPool;
        allocInfo.descriptorSetCount = (*linkedRenderEngine->swapChainImages).size();
        allocInfo.pSetLayouts = layouts.data();
        (*linkedRenderEngine->descriptorSets).resize((*linkedRenderEngine->swapChainImages).size());
        if (vkAllocateDescriptorSets(*linkedRenderEngine->logicalDevice, &allocInfo, (*linkedRenderEngine->descriptorSets).data()) != VK_SUCCESS) {throw std::runtime_error("failed to allocate descriptor sets!");}
        for (size_t i = 0; i < (*linkedRenderEngine->swapChainImages).size(); i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = (*linkedRenderEngine->uniformBuffers)[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;
            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = (*linkedRenderEngine->descriptorSets)[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = (*linkedRenderEngine->descriptorSets)[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;
            vkUpdateDescriptorSets(*linkedRenderEngine->logicalDevice, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
        }
        return *this;
    }
};