#pragma once

#include <utility>

#include "AssetLinking.hpp"

class Asset {

public:
    Asset(const char *modelName, const std::vector<const char *>& textureNames, const RenderEngineLink& renderEngineLink) {
        setEngineLink(renderEngineLink);
        loadModel(modelName);
        loadTextures(textureNames);
        uploadAsset();
    }

    Asset setEngineLink(const RenderEngineLink& renderEngineLink) {
        linkedRenderEngine = renderEngineLink;
        return *this;
    }

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
            if (!pixels) {throw std::runtime_error("failed to load texture image!");} else {
                int imageSize = width * height * 4;
                std::vector<int, std::allocator<int>> texture;
                for (int i = 0; i < imageSize; i++) {texture.push_back(pixels[i]);}
                textures.push_back(texture);
                stbi_image_free(pixels);
            }
        }
    }

    void uploadAsset() {
        VkDeviceSize imageSize = width * height * 4;
        stagingBuffer.setEngineLink(linkedRenderEngine);
        memcpy(stagingBuffer.create(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), textures[0].data(), imageSize);
        textureImage.setEngineLink(linkedRenderEngine);
        textureImage.create(width, height, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SAMPLE_COUNT_1_BIT);
        textureImage.transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        stagingBuffer.toImage(textureImage.image, width, height);
        textureImage.transition(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    RenderEngineLink linkedRenderEngine{};
    AllocatedImage textureImage{};
    AllocatedBuffer stagingBuffer{};
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};
    VkCommandBuffer commandBuffer{};
    std::vector<std::vector<int>> textures{};
    int width{}, height{}, channels{};
};