#pragma once

#include <utility>

#include "AssetLinking.hpp"

class Asset {

public:

private:
    void loadModel(const char *filename) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename)) {throw std::runtime_error(warn + err);}
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.pos = {attrib.vertices[3 * index.vertex_index], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2]};
                vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index], 1.f - attrib.texcoords[2 * index.texcoord_index + 1]};
                vertex.normal = {attrib.normals[3 * index.normal_index], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2]};
                vertex.color = {1.f, 1.f, 1.f};
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = vertices.size();
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    void loadTextures(const std::vector<const char *>& filenames) {
        for (const char *filename : filenames) {
            stbi_uc *pixels = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { throw std::runtime_error("failed to load texture image!"); } else {
                textures.push_back(*pixels);
                stbi_image_free(pixels);
            }
        }
    }

    void uploadAsset(const RenderEngineLink& renderEngineLink) {
        linkedRenderEngine = renderEngineLink;
        VkDeviceSize imageSize = width * height * 4;
        AllocatedBuffer stagingBuffer{};
        stagingBuffer.allocator = renderEngineLink.allocator;
        stagingBuffer.device = renderEngineLink.logicalDevice;
        VkDeviceMemory stagingBufferMemory;
        for (stbi_uc texture : textures) {
//            //Create buffer for pixels
            stagingBuffer.allocate(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, VMA_MEMORY_USAGE_CPU_TO_GPU);
            /* -----------------------------PERHAPS USE THIS IF THE ABOVE FAILS-----------------------------------
            if (vkCreateBuffer(renderEngineLink.logicalDevice, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) { throw std::runtime_error("failed to create image buffer!"); }
            VkMemoryRequirements memoryRequirements{};
            vkGetBufferMemoryRequirements(renderEngineLink.logicalDevice, stagingBuffer, &memoryRequirements);
            VkPhysicalDeviceMemoryProperties memProperties;
            vkGetPhysicalDeviceMemoryProperties(linkedRenderEngine.physicalDevice, &memProperties);
            int memoryTypeIndex{};
            for (int i = 0; i < memProperties.memoryTypeCount; i++) {
                if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                    memoryTypeIndex = i;
                    break;
                }
            }
            if (!memoryTypeIndex) { throw std::runtime_error("failed to find suitable memory type!"); }
            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memoryRequirements.size;
            allocInfo.memoryTypeIndex = memoryTypeIndex;
            if (vkAllocateMemory(linkedRenderEngine.logicalDevice, &allocInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS) { throw std::runtime_error("failed to allocate buffer memory!"); }
            vkBindImageMemory(device, image, memory, 0);
            */
            //Put pixels in buffer
            void *data;
            vkMapMemory(renderEngineLink.logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
            memcpy(data, &texture, imageSize);
            vkUnmapMemory(renderEngineLink.logicalDevice, stagingBufferMemory);
            stbi_image_free(&texture);
            textureImage.allocator = renderEngineLink.allocator;
            textureImage.device = renderEngineLink.logicalDevice;
            //textureImage.allocate(width, height, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_SAMPLE_COUNT_1_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU)

//            VkImageCreateInfo imageInfo{};
//            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//            imageInfo.imageType = VK_IMAGE_TYPE_2D;
//            imageInfo.extent.width = width;
//            imageInfo.extent.height = height;
//            imageInfo.extent.depth = 1;
//            imageInfo.mipLevels = 1;
//            imageInfo.arrayLayers = 1;
//            imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
//            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
//            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//            imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
//            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//            VmaAllocationCreateInfo allocInfo{};
//            allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
//            vmaCreateImage(linkedRenderEngine.allocator, &imageInfo, &allocInfo, &textureImage.image, &textureImage.allocation, nullptr);
        }
    }

    RenderEngineLink linkedRenderEngine{};
    VkImageView textureImageView{};
    AllocatedImage textureImage{};
    VkDeviceMemory textureImageMemory{};
    VkSampler textureSampler{};
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};
    VkCommandBuffer commandBuffer{};
    std::vector<stbi_uc> textures{};
    int width{}, height{}, channels{};
};