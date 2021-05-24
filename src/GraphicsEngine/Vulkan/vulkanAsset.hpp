#pragma once

#include "vulkanShader.hpp"
#include "vulkanCamera.hpp"
#include "vulkanVertex.hpp"
#include "vulkanTexture.hpp"
#include "vulkanRasterizationPipeline.hpp"
#include "vulkanRayTracingPipeline.hpp"
#include "vulkanAccelerationStructure.hpp"
#include "vulkanUniformBufferObject.hpp"
#include "vulkanShaderBindingTable.hpp"
#include "vulkanGraphicsEngineLink.hpp"

#include <glm/gtc/quaternion.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <../../../deps/tiny_obj_loader.h>
#define STB_IMAGE_IMPLEMENTATION
#include <../../../deps/stb_image.h>

#include <fstream>
#include <cstring>

//TODO: Add instancing
class Asset {
public:
    Asset(const char *modelFileName, const std::vector<const char *>& textureFileNames, const std::vector<const char *>& shaderFileNames, glm::vec3 initialPosition = {0, 0, 0}, glm::vec3 initialRotation = {0, 0, 0}, glm::vec3 initialScale = {1, 1, 1}) {
        position = initialPosition;
        rotation = initialRotation;
        scale = initialScale;
        modelName = modelFileName;
        textureNames = textureFileNames;
        shaderNames = shaderFileNames;
        loadModel(modelFileName);
        loadTextures(textureFileNames);
        loadShaders(shaderFileNames);
    }

    void reloadAsset(const char *modelFileName = nullptr, const std::vector<const char *> *textureFileNames = nullptr, const std::vector<const char *> *shaderFileNames = nullptr) {
        if (modelFileName != nullptr) { modelName = modelFileName; }
        if (textureFileNames != nullptr) { textureNames = *textureFileNames; }
        if (shaderFileNames != nullptr) { shaderNames = *shaderFileNames; }
        loadModel(modelName);
        loadTextures(textureNames);
        loadShaders(shaderNames);
    }

    void destroy() {
        for (Image &textureImage : textureImages) { textureImage.destroy(); }
        for (const std::function<void(Asset)>& function : deletionQueue) { function(*this); }
        deletionQueue.clear();
    }

    void update(VulkanCamera camera) {
        uniformBufferObject = {glm::mat4(1.0f), camera.view, camera.proj};
        glm::quat quaternion = glm::quat(glm::radians(rotation));
        uniformBufferObject.model = glm::translate(glm::rotate(glm::scale(glm::mat4(1.0f), scale), glm::angle(quaternion), glm::axis(quaternion)), position);
        memcpy(uniformBuffer.data, &uniformBufferObject, sizeof(UniformBufferObject));
    }

    std::deque<std::function<void(Asset asset)>> deletionQueue{};
    std::vector<uint32_t> indices{};
    std::vector<Vertex> vertices{};
    Buffer uniformBuffer{};
    Buffer vertexBuffer{};
    Buffer indexBuffer{};
    Buffer transformationBuffer{};
    RasterizationPipelineManager pipelineManager{};
    DescriptorSetManager descriptorSetManager{};
    AccelerationStructure bottomLevelAccelerationStructure{};
    UniformBufferObject uniformBufferObject{};
    std::vector<Texture> textureImages{};
    std::vector<const char *> textures{};
    std::vector<Shader::CreateInfo> shaderCreateInfos{};
    std::vector<Shader> shaders{};
    int width{};
    int height{};
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{};
    bool render{true};
    uint32_t triangleCount{};
    VkTransformMatrixKHR transformationMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

private:
    void loadModel(const char *filename) {
        vertices.clear();
        indices.clear();
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename)) { throw std::runtime_error(warn + err); }
        size_t reserveCount{};
        for (const auto& shape : shapes) { reserveCount += shape.mesh.indices.size(); }
        indices.reserve(reserveCount);
        vertices.reserve(reserveCount * (2 / 3)); // Allocates too much space! Let's procrastinate cutting it down.
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        uniqueVertices.reserve(reserveCount * (2 / 3)); // Also allocates too much space, but it will be deleted at the end of the function, so we don't care
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.pos = { attrib.vertices[3 * index.vertex_index], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] };
                vertex.texCoord = { attrib.texcoords[2 * index.texcoord_index], 1.f - attrib.texcoords[2 * index.texcoord_index + 1] };
                vertex.normal = { attrib.normals[3 * index.normal_index], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2] };
                vertex.color = {1.f, 1.f, 1.f};
                if (uniqueVertices.find(vertex) == uniqueVertices.end()) {
                    uniqueVertices.insert({vertex, static_cast<uint32_t>(vertices.size())});
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        // Remove unneeded space at end of vertices at the last minute
        std::vector<Vertex> tmp = vertices;
        vertices.swap(tmp);
        triangleCount = static_cast<uint32_t>(indices.size()) / 3;
    }

    void loadTextures(const std::vector<const char *>& filenames) {
        textures.clear();
        textures = filenames;
    }

    //First shader is vertex rest are fragment
    void loadShaders(const std::vector<const char *>& filenames, bool compile = true) {
        shaderCreateInfos.clear();
        shaderCreateInfos.reserve(filenames.size());
        for (const char *filename : filenames) {
            Shader::CreateInfo shaderCreateInfo {filename, filename == filenames[0] ? VK_SHADER_STAGE_VERTEX_BIT : VK_SHADER_STAGE_FRAGMENT_BIT};
            shaderCreateInfos.push_back(shaderCreateInfo);
        }
    }

    std::vector<const char *> shaderNames{};
    std::vector<const char *> textureNames{};
    const char *modelName{};
};