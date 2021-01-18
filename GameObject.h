#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Settings.h"

#include <iostream>


struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

namespace std {template<> struct hash<Vertex> {size_t operator()(Vertex const& vertex) const {return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);}};}

class GameObject{
public:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    int texWidth{}, texHeight{}, texChannels{};
    stbi_uc* albedo;
    stbi_uc* normal{};
    stbi_uc* herm{};
    std::string modelFilename;
    std::string albedoFilename;
    std::string normalFilename;
    std::string hermFilename;

    GameObject(const std::string& filename, const std::string& modelExtension, const std::string& textureExtension) {
        modelFilename = filename + modelExtension;
        albedoFilename = filename + textureExtension;
        normalFilename = filename + "_normal" + textureExtension;
        hermFilename = filename + "_herm" + textureExtension;
        //Import model
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,modelFilename.c_str())) {throw std::runtime_error(warn + err);}
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
        albedo = stbi_load(albedoFilename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!albedo) {std::cerr << "failed to load texture image: " + albedoFilename << std::endl;}
        normal = stbi_load(normalFilename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!normal) {std::cerr << "failed to load texture image: " + normalFilename << std::endl;}
        herm = stbi_load(hermFilename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!herm) {std::cerr << "failed to load texture image: " + hermFilename << std::endl;}
    }


};

#endif
