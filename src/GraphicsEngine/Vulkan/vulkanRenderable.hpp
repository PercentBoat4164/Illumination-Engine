/**@todo: Allow loading models and textures into RAM without loading them into VRAM.
 * - MEDIUM PRIORITY: Useful and easy to implement feature.
 */

/**@todo: Switch to ASSIMP to support materials and more file types.
 * - HIGH PRIORITY: Should be done before the second alpha release for convenience.
 */

#pragma once

#include "vulkanShader.hpp"
#include "vulkanCamera.hpp"
#include "vulkanVertex.hpp"
#include "vulkanTexture.hpp"
#include "vulkanPipeline.hpp"
#include "vulkanAccelerationStructure.hpp"
#include "vulkanUniformBufferObject.hpp"
#include "vulkanGraphicsEngineLink.hpp"

#include <cstddef>
#include <glm/gtc/quaternion.hpp>

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <../../../deps/tiny_obj_loader.h>
#endif

#include <fstream>
#include <cstring>
#include <vector>

class VulkanRenderable {
public:
    VulkanRenderable(VulkanGraphicsEngineLink *engineLink, const char *modelFileName, const std::vector<const char *> &textureFileNames, const std::vector<const char *> &shaderFileNames, glm::vec3 initialPosition = {0, 0, 0}, glm::vec3 initialRotation = {0, 0, 0}, glm::vec3 initialScale = {1, 1, 1}) {
        linkedRenderEngine = engineLink;
        position = initialPosition;
        rotation = initialRotation;
        scale = initialScale;
        modelName = modelFileName;
        textureNames = textureFileNames;
        if (textureNames.empty()) { textureNames.push_back("NO_FILE"); }
        shaderNames = shaderFileNames;
        loadModel(modelFileName);
        loadTextures(textureFileNames);
        loadShaders(shaderFileNames);
    }

    void reloadRenderable(const char *modelFileName = nullptr, const std::vector<const char *> *textureFileNames = nullptr, const std::vector<const char *> *shaderFileNames = nullptr) {
        if (modelFileName != nullptr) { modelName = modelFileName; }
        if (textureFileNames != nullptr) { textureNames = *textureFileNames; }
        if (shaderFileNames != nullptr) { shaderNames = *shaderFileNames; }
        loadModel(modelName);
        loadTextures(textureNames);
        loadShaders(shaderNames);
        created = true;
    }

    void destroy() {
        if (!created) { return; }
#pragma unroll 1
        for (VulkanImage &textureImage : textureImages) { textureImage.destroy(); }
#pragma unroll 9
        for (const std::function<void(VulkanRenderable *)> &function : deletionQueue) { function(this); }
        deletionQueue.clear();
        created = false;
    }

    void update(const VulkanCamera &camera) {
        glm::quat quaternion = glm::quat(glm::radians(rotation));
        glm::mat4 matrix = glm::translate(glm::rotate(glm::scale(glm::mat4(1.0f), scale), glm::angle(quaternion), glm::axis(quaternion)), position);
        uniformBufferObject = {matrix, camera.view, camera.proj, (float)glfwGetTime()};
        modelBuffer.uploadData(&uniformBufferObject, sizeof(VulkanUniformBufferObject));
        if (linkedRenderEngine->settings->rayTracing) {
            transformationMatrix = {uniformBufferObject.model[0][0], uniformBufferObject.model[0][1], uniformBufferObject.model[0][2], uniformBufferObject.model[3][0], uniformBufferObject.model[1][0], uniformBufferObject.model[1][1], uniformBufferObject.model[1][2], uniformBufferObject.model[3][1], uniformBufferObject.model[2][0], uniformBufferObject.model[2][1], uniformBufferObject.model[2][2], uniformBufferObject.model[3][2]};
            transformationBuffer.uploadData(&transformationMatrix, sizeof(transformationMatrix));
            if (bottomLevelAccelerationStructure.created) { bottomLevelAccelerationStructure.destroy(); }
            VulkanAccelerationStructure::CreateInfo renderableBottomLevelAccelerationStructureCreateInfo{};
            renderableBottomLevelAccelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
            renderableBottomLevelAccelerationStructureCreateInfo.transformationMatrix = &identityTransformMatrix;
            renderableBottomLevelAccelerationStructureCreateInfo.primitiveCount = triangleCount;
            renderableBottomLevelAccelerationStructureCreateInfo.vertexBufferAddress = vertexBuffer.deviceAddress;
            renderableBottomLevelAccelerationStructureCreateInfo.indexBufferAddress = indexBuffer.deviceAddress;
            renderableBottomLevelAccelerationStructureCreateInfo.transformationBufferAddress = transformationBuffer.deviceAddress;
            bottomLevelAccelerationStructure.create(linkedRenderEngine, &renderableBottomLevelAccelerationStructureCreateInfo);
        }
    }

    std::deque<std::function<void(VulkanRenderable *asset)>> deletionQueue{};
    std::vector<uint32_t> indices{};
    std::vector<VulkanVertex> vertices{};
    VulkanBuffer modelBuffer{};
    VulkanBuffer vertexBuffer{};
    VulkanBuffer indexBuffer{};
    VulkanBuffer transformationBuffer{};
    VulkanPipeline pipeline{};
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    DescriptorSet descriptorSet{};
    VulkanAccelerationStructure bottomLevelAccelerationStructure{};
    VulkanUniformBufferObject uniformBufferObject{};
    std::vector<VulkanTexture> textureImages{};
    std::vector<const char *> textures{};
    std::vector<VulkanShader::CreateInfo> shaderCreateInfos{};
    std::vector<VulkanShader> shaders{};
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{};
    bool render{true};
    bool created{false};
    uint32_t triangleCount{};
    VkTransformMatrixKHR transformationMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    VkTransformMatrixKHR identityTransformMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};


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
#pragma unroll 1
        for (const auto &shape : shapes) { reserveCount += shape.mesh.indices.size(); }
        indices.reserve(reserveCount);
        vertices.reserve(reserveCount * (2 / 3)); // Allocates too much space! Let's procrastinate cutting it down.
        std::unordered_map<VulkanVertex, uint32_t> uniqueVertices{};
        uniqueVertices.reserve(reserveCount * (2 / 3)); // Also allocates too much space, but it will be deleted at the end of the function, so we don't care
        for (const auto &shape : shapes) {
#pragma unroll 1
            for (const auto &index : shape.mesh.indices) {
                VulkanVertex vertex{};
                vertex.pos = { attrib.vertices[static_cast<std::vector<int>::size_type>(3) * index.vertex_index], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] };
                vertex.texCoord = { attrib.texcoords[static_cast<std::vector<int>::size_type>(2) * index.texcoord_index], 1.f - attrib.texcoords[2 * index.texcoord_index + 1] };
                vertex.normal = { attrib.normals[static_cast<std::vector<int>::size_type>(3) * index.normal_index], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2] };
                vertex.color = {1.0f, 1.0f, 1.0f, 1.0f};
                if (uniqueVertices.find(vertex) == uniqueVertices.end()) {
                    uniqueVertices.insert({vertex, static_cast<uint32_t>(vertices.size())});
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        // Remove unneeded space at end of vertices at the last minute
        std::vector<VulkanVertex> tmp = vertices;
        vertices.swap(tmp);
        triangleCount = static_cast<uint32_t>(indices.size()) / 3;
    }

    void loadTextures(const std::vector<const char *> &filenames) {
        textures = filenames;
    }

    //First shader is vertex rest are fragment
    void loadShaders(const std::vector<const char *> &filenames) {
        shaderCreateInfos.clear();
        shaderCreateInfos.reserve(filenames.size());
#pragma unroll 1
        for (const char *filename : filenames) {
            VulkanShader::CreateInfo shaderCreateInfo {filename};
            shaderCreateInfos.push_back(shaderCreateInfo);
        }
    }

    std::vector<const char *> shaderNames{};
    std::vector<const char *> textureNames{};
    const char *modelName{};
};