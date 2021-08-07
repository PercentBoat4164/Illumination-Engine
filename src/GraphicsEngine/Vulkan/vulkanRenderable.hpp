#pragma once

#include "vulkanShader.hpp"
#include "vulkanCamera.hpp"
#include "vulkanVertex.hpp"
#include "vulkanTexture.hpp"
#include "vulkanDescriptorSet.hpp"
#include "vulkanPipeline.hpp"
#include "vulkanAccelerationStructure.hpp"
#include "vulkanUniformBufferObject.hpp"
#include "vulkanGraphicsEngineLink.hpp"

#include <../../../deps/assimp/include/assimp/Importer.hpp>
#include <../../../deps/assimp/include/assimp/scene.h>
#include <../../../deps/assimp/include/assimp/postprocess.h>

#include <glm/gtc/quaternion.hpp>

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <../../../deps/tiny_obj_loader.h>
#endif

#include <cstddef>
#include <fstream>
#include <cstring>
#include <vector>

class VulkanRenderable {
public:
    struct VulkanMesh {
        std::vector<unsigned int> indices{};
        std::vector<VulkanVertex> vertices{};
        VkTransformMatrixKHR transformationMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
        unsigned int diffuseTexture{};
        unsigned int emissionTexture{};
        unsigned int heightTexture{};
        unsigned int metallicTexture{};
        unsigned int normalTexture{};
        unsigned int roughnessTexture{};
        unsigned int specularTexture{};
        VulkanBuffer vertexBuffer{};
        VulkanBuffer indexBuffer{};
        VulkanBuffer transformationBuffer{};
        VulkanDescriptorSet descriptorSet{};
        VulkanPipeline pipeline{};
        VulkanAccelerationStructure bottomLevelAccelerationStructure{};
        std::deque<std::function<void(VulkanMesh *mesh)>> deletionQueue{};

        void destroy() {
            for (const std::function<void(VulkanMesh *)> &function : deletionQueue) { function(this); }
            deletionQueue.clear();
        }
    };

    std::vector<const char *> shaderNames{"res/Shaders/ThisLocationGetsSwitchedWithRayTracingAndRasterization/vertexShader.vert", "res/Shaders/ThisLocationGetsSwitchedWithRayTracingAndRasterization/fragmentShader.frag"};
    const char *modelName{};

    explicit VulkanRenderable(VulkanGraphicsEngineLink *engineLink, const char *filePath) {
        linkedRenderEngine = engineLink;
        modelName = filePath;
        int channels{};
        VulkanTexture::CreateInfo textureCreateInfo{};
        textureCreateInfo.filename = std::string("res/Models/NoTexture.png");
        textureCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        textureCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        textureCreateInfo.allocationUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        textureCreateInfo.imageType = VULKAN_TEXTURE;
        textureCreateInfo.data = stbi_load(textureCreateInfo.filename.c_str(), &textureCreateInfo.width, &textureCreateInfo.height, &channels, STBI_rgb_alpha);
        if (!textureCreateInfo.data) { throw std::runtime_error("failed to load texture image from file: " + textureCreateInfo.filename); }
        textures[0].create(linkedRenderEngine, &textureCreateInfo);
        Assimp::Importer importer{};
        const aiScene *scene = importer.ReadFile(modelName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_GenUVCoords | aiProcess_CalcTangentSpace | aiProcess_GenNormals);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { throw std::runtime_error("failed to load texture image from file: " + std::string(filePath)); }
        std::string directory = std::string(filePath).substr(0, std::string(filePath).find_last_of('/'));
        meshes.reserve(scene->mNumMeshes);
        processNode(scene->mRootNode, scene, directory);
    }

    void reloadRenderable(const std::vector<const char *> &shaderFileNames) {
        loadShaders(shaderFileNames);
        created = true;
    }

    void destroy() {
        if (!created) { return; }
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
            for (VulkanMesh mesh : meshes) {
                mesh.transformationMatrix = {
                        uniformBufferObject.model[0][0], uniformBufferObject.model[0][1], uniformBufferObject.model[0][2], uniformBufferObject.model[3][0],
                        uniformBufferObject.model[1][0], uniformBufferObject.model[1][1], uniformBufferObject.model[1][2], uniformBufferObject.model[3][1],
                        uniformBufferObject.model[2][0], uniformBufferObject.model[2][1], uniformBufferObject.model[2][2], uniformBufferObject.model[3][2]
                };
                mesh.transformationBuffer.uploadData(&mesh.transformationMatrix, sizeof(mesh.transformationMatrix));
                if (mesh.bottomLevelAccelerationStructure.created) { mesh.bottomLevelAccelerationStructure.unload(); }
                VulkanAccelerationStructure::CreateInfo renderableBottomLevelAccelerationStructureCreateInfo{};
                renderableBottomLevelAccelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
                renderableBottomLevelAccelerationStructureCreateInfo.transformationMatrix = &identityTransformMatrix;
                renderableBottomLevelAccelerationStructureCreateInfo.primitiveCount = triangleCount;
                renderableBottomLevelAccelerationStructureCreateInfo.vertexBufferAddress = mesh.vertexBuffer.deviceAddress;
                renderableBottomLevelAccelerationStructureCreateInfo.indexBufferAddress = mesh.indexBuffer.deviceAddress;
                renderableBottomLevelAccelerationStructureCreateInfo.transformationBufferAddress = mesh.transformationBuffer.deviceAddress;
                mesh.bottomLevelAccelerationStructure.create(linkedRenderEngine, &renderableBottomLevelAccelerationStructureCreateInfo);
            }
        }
    }

    std::deque<std::function<void(VulkanRenderable *renderable)>> deletionQueue{};
    VulkanBuffer modelBuffer{};
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    VulkanUniformBufferObject uniformBufferObject{};
    std::vector<VulkanMesh> meshes{};
    std::vector<VulkanTexture> textures{1};
    std::vector<VulkanShader::CreateInfo> shaderCreateInfos{};
    std::vector<VulkanShader> shaders{};
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{};
    bool render{true};
    bool created{false};
    uint32_t triangleCount{};
    VkTransformMatrixKHR identityTransformMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};


private:
    void processNode(aiNode *node, const aiScene *scene, const std::string& directory) {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            VulkanMesh temporaryMesh{};
            temporaryMesh.vertices.reserve(scene->mMeshes[node->mMeshes[i]]->mNumVertices);
            for (unsigned int j = 0; j < scene->mMeshes[node->mMeshes[i]]->mNumVertices; ++j) {
                VulkanVertex temporaryVertex{};
                temporaryVertex.position.x = scene->mMeshes[node->mMeshes[i]]->mVertices[j].x;
                temporaryVertex.position.y = scene->mMeshes[node->mMeshes[i]]->mVertices[j].y;
                temporaryVertex.position.z = scene->mMeshes[node->mMeshes[i]]->mVertices[j].z;
                temporaryVertex.normal.x = scene->mMeshes[node->mMeshes[i]]->mNormals[j].x;
                temporaryVertex.normal.y = scene->mMeshes[node->mMeshes[i]]->mNormals[j].y;
                temporaryVertex.normal.z = scene->mMeshes[node->mMeshes[i]]->mNormals[j].z;
                if (scene->mMeshes[node->mMeshes[i]]->mNumUVComponents[0] > 0) {
                    temporaryVertex.textureCoordinates.x = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[0][j].x;
                    temporaryVertex.textureCoordinates.y = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[0][j].y;
                }
                temporaryMesh.vertices.push_back(temporaryVertex);
            }
            temporaryMesh.indices.reserve(static_cast<std::vector<unsigned int>::size_type>(scene->mMeshes[node->mMeshes[i]]->mNumFaces) * 3);
            for (unsigned int j = 0; j < scene->mMeshes[node->mMeshes[i]]->mNumFaces; ++j) { for (unsigned int k = 0; k < scene->mMeshes[node->mMeshes[i]]->mFaces[j].mNumIndices; ++k) { temporaryMesh.indices.push_back(scene->mMeshes[node->mMeshes[i]]->mFaces[j].mIndices[k]); } }
            if (scene->mMeshes[node->mMeshes[i]]->mMaterialIndex >= 0) {
                std::vector<std::pair<unsigned int *, aiTextureType>> textureTypes{
                    {&temporaryMesh.diffuseTexture, aiTextureType_DIFFUSE},
                    {&temporaryMesh.emissionTexture, aiTextureType_EMISSIVE},
                    {&temporaryMesh.heightTexture, aiTextureType_HEIGHT},
                    {&temporaryMesh.metallicTexture, aiTextureType_METALNESS},
                    {&temporaryMesh.normalTexture, aiTextureType_NORMALS},
                    {&temporaryMesh.roughnessTexture, aiTextureType_DIFFUSE_ROUGHNESS},
                    {&temporaryMesh.specularTexture, aiTextureType_SPECULAR}
                };
                for (std::pair<unsigned int *, aiTextureType> textureType : textureTypes) {
                    VulkanTexture temporaryTexture{};
                    bool textureAlreadyLoaded{false};
                    VulkanTexture::CreateInfo textureCreateInfo{};
                    aiString filename;
                    scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTexture(textureType.second, 0, &filename);
                    if (filename.length == 0) { continue; }
                    std::string path {directory + '/' + std::string(filename.C_Str())};
                    for (unsigned int k = 0; k < textures.size(); ++k) {
                        if (std::strcmp(textures[k].createdWith.filename.c_str(), path.c_str()) == 0) {
                            *textureType.first = k;
                            textureAlreadyLoaded = true;
                            break;
                        }
                    }
                    if (!textureAlreadyLoaded) {
                        int channels{};
                        textureCreateInfo.filename = path;
                        textureCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
                        textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                        textureCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                        textureCreateInfo.allocationUsage = VMA_MEMORY_USAGE_GPU_ONLY;
                        textureCreateInfo.mipMapping = linkedRenderEngine->settings->mipMapping;
                        textureCreateInfo.imageType = VULKAN_TEXTURE;
                        textureCreateInfo.data = stbi_load(textureCreateInfo.filename.c_str(), &textureCreateInfo.height, &textureCreateInfo.width, &channels, STBI_rgb_alpha);
                        if (!textureCreateInfo.data) { throw std::runtime_error("failed to load texture image from file: " + textureCreateInfo.filename); }
                        temporaryTexture.create(linkedRenderEngine, &textureCreateInfo);
                        textures.push_back(temporaryTexture);
                        *textureType.first = textures.size() - 1;
                    }
                }
            }
            meshes.push_back(temporaryMesh);
        }
        for (unsigned int i = 0; i < node->mNumChildren; ++i) { processNode(node->mChildren[i], scene, directory); }
    }

    /**@todo: Make this auto-detect shader stage.*/
    void loadShaders(const std::vector<const char *> &filenames) {
        shaderCreateInfos.clear();
        shaderCreateInfos.reserve(filenames.size());
        for (const char *filename : filenames) {
            VulkanShader::CreateInfo shaderCreateInfo {filename};
            shaderCreateInfos.push_back(shaderCreateInfo);
        }
    }
};