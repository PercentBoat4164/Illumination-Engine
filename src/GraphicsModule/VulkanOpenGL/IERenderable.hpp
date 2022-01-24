#pragma once

#include "IEShader.hpp"
#include "IECamera.hpp"
#include "IEVertex.hpp"
#include "IETexture.hpp"
#include "IEDescriptorSet.hpp"
#include "IEPipeline.hpp"
#include "IEAccelerationStructure.hpp"
#include "IEUniformBufferObject.hpp"
#include "IEGraphicsLink.hpp"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif
#ifndef VMA_INCLUDED
#define VMA_INCLUDED
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#endif

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/gtc/quaternion.hpp>

#include <vulkan/vulkan.h>

#include <cstddef>
#include <fstream>
#include <cstring>
#include <vector>
#include <filesystem>

/**@todo Refine the asset management process. Abstract this to the Asset Module.*/

class IERenderable {
public:
    const char *modelName{};
    std::vector<uint32_t> indices{};
    std::vector<IEVertex> vertices{};
    VkTransformMatrixKHR transformationMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    uint32_t diffuseTexture{};
    uint32_t emissionTexture{};
    uint32_t heightTexture{};
    uint32_t metallicTexture{};
    uint32_t normalTexture{};
    uint32_t roughnessTexture{};
    uint32_t specularTexture{};
    uint32_t triangleCount{};
    IEBuffer vertexBuffer{};
    IEBuffer indexBuffer{};
    IEBuffer transformationBuffer{};
    IEDescriptorSet descriptorSet{};
    IEPipeline pipeline{};
    IEAccelerationStructure bottomLevelAccelerationStructure{};

    IERenderable(IEGraphicsLink* engineLink, const std::string& filePath) {
        linkedRenderEngine = engineLink;
        textures = linkedRenderEngine->textures;
        modelName = filePath.c_str();
        directory = filePath.substr(0, filePath.find_last_of('/'));
        int channels{};
        IETexture::CreateInfo textureCreateInfo{};
        textureCreateInfo.filename = std::string("res/Models/NoTexture.png");
        textureCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        textureCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        textureCreateInfo.allocationUsage = VMA_MEMORY_USAGE_GPU_ONLY;
        textureCreateInfo.imageType = VULKAN_TEXTURE;
        textureCreateInfo.data = stbi_load(textureCreateInfo.filename.c_str(), &textureCreateInfo.width, &textureCreateInfo.height, &channels, STBI_rgb_alpha);
        if (!textureCreateInfo.data) { throw std::runtime_error("failed to prepare texture image from file: " + textureCreateInfo.filename); }
        (*textures)[0].create(linkedRenderEngine, &textureCreateInfo);
        Assimp::Importer importer{};
        const aiScene *scene = importer.ReadFile(modelName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_GenUVCoords | aiProcess_CalcTangentSpace | aiProcess_GenNormals);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { throw std::runtime_error("failed to prepare texture image from file: " + std::string(filePath)); }
        processNode(scene->mRootNode, scene);
    }

    void destroy() {
        if (!created) {
            return;
        }
        for (const std::function<void()> &function : deletionQueue) {
            function();
        }
        deletionQueue.clear();
        created = false;
    }

    void createShaders(const std::string& shaderDirectory) {
        IEShader shader = IEShader{};
        IEShader::CreateInfo shaderCreateInfo{.filename="shaders/Rasterize/fragmentShader.frag.spv"};
        shader.create(linkedRenderEngine, &shaderCreateInfo);
        shaders.push_back(shader);
        shader = IEShader{};
        shaderCreateInfo.filename="shaders/Rasterize/vertexShader.vert.spv";
        shader.create(linkedRenderEngine, &shaderCreateInfo);
        shaders.push_back(shader);
    }

    void createModelBuffer() {
        IEBuffer::CreateInfo modelBufferCreateInfo{
            .size=sizeof(IEUniformBufferObject),
            .usage=VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU
        };
        modelBuffer.create(linkedRenderEngine, &modelBufferCreateInfo);
        deletionQueue.emplace_back([&] {
            modelBuffer.destroy();
        });
    }

    void createVertexBuffer() {
        IEBuffer::CreateInfo vertexBufferCreateInfo{
            .size=sizeof(vertices[0]) * vertices.size(),
            .usage=VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU
        };
        vertexBuffer.create(linkedRenderEngine, &vertexBufferCreateInfo);
        deletionQueue.emplace_back([&] {
            vertexBuffer.destroy();
        });
    }

    ~IERenderable() {
        destroy();
    }

    void update(const IECamera &camera, float time) {
        glm::quat quaternion = glm::quat(glm::radians(rotation));
        glm::mat4 modelMatrix = glm::translate(glm::rotate(glm::scale(glm::mat4(1.0f), scale), glm::angle(quaternion), glm::axis(quaternion)), position);
        uniformBufferObject.viewModelMatrix = camera.viewMatrix;
        uniformBufferObject.modelMatrix = modelMatrix;
        uniformBufferObject.projectionMatrix = camera.projectionMatrix;
        uniformBufferObject.normalMatrix = glm::mat4(glm::transpose(glm::inverse(modelMatrix)));
        uniformBufferObject.position = camera.position;
        uniformBufferObject.time = time;
        modelBuffer.uploadData(&uniformBufferObject, sizeof(uniformBufferObject));
        if (linkedRenderEngine->settings.rayTracing) {
            transformationMatrix = {
                    modelMatrix[0][0], modelMatrix[0][1], modelMatrix[0][2], modelMatrix[3][0],
                    modelMatrix[1][0], modelMatrix[1][1], modelMatrix[1][2], modelMatrix[3][1],
                    modelMatrix[2][0], modelMatrix[2][1], modelMatrix[2][2], modelMatrix[3][2]
            };
            transformationBuffer.uploadData(&transformationMatrix, sizeof(transformationMatrix));
            bottomLevelAccelerationStructure.destroy();
            IEAccelerationStructure::CreateInfo renderableBottomLevelAccelerationStructureCreateInfo{};
            renderableBottomLevelAccelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
            renderableBottomLevelAccelerationStructureCreateInfo.transformationMatrix = &identityTransformMatrix;
            renderableBottomLevelAccelerationStructureCreateInfo.primitiveCount = triangleCount;
            renderableBottomLevelAccelerationStructureCreateInfo.vertexBufferAddress = vertexBuffer.deviceAddress;
            renderableBottomLevelAccelerationStructureCreateInfo.indexBufferAddress = indexBuffer.deviceAddress;
            renderableBottomLevelAccelerationStructureCreateInfo.transformationBufferAddress = transformationBuffer.deviceAddress;
            bottomLevelAccelerationStructure.create(linkedRenderEngine, &renderableBottomLevelAccelerationStructureCreateInfo);
        }
    }

    std::vector<std::function<void()>> deletionQueue{};
    IEBuffer modelBuffer{};
    IEGraphicsLink *linkedRenderEngine{};
    IEUniformBufferObject uniformBufferObject{};
    std::vector<IETexture>* textures;
    std::vector<IEShader::CreateInfo> shaderCreateInfos{};
    std::vector<IEShader> shaders{};
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    bool render{true};
    bool created{false};
    std::string directory{};
    VkTransformMatrixKHR identityTransformMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

    void createIndexBuffer() {
        IEBuffer::CreateInfo indexBufferCreateInfo{
                .size=sizeof(vertices[0]) * vertices.size(),
                .usage=VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                .allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU
        };
        indexBuffer.create(linkedRenderEngine, &indexBufferCreateInfo);
        deletionQueue.emplace_back([&] {
            indexBuffer.destroy();
        });
    }

    /**@todo Rewrite the shader abstraction to generate a required descriptor set. Create pipelines based off of descriptor set / shader pairs. Sort and render geometry by pipeline used.*/

    void createDescriptorSet() {
        IEDescriptorSet::CreateInfo descriptorSetCreateInfo{
            .poolSizes={{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}},
            .shaderStages={static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT), VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_FRAGMENT_BIT},
            .data={&modelBuffer, &(*textures)[diffuseTexture], &(*textures)[specularTexture]}
        };
        descriptorSet.create(linkedRenderEngine, &descriptorSetCreateInfo);
        deletionQueue.emplace_back([&] {
            descriptorSet.destroy();
        });
    }

    void uploadTextures() {
        for (IETexture& texture : *textures) {
            texture.upload();
        }
    }

    void createPipeline() {
        IEPipeline::CreateInfo pipelineCreateInfo{
            .shaders=&shaders,
            .descriptorSet=&descriptorSet,
            .renderPass=linkedRenderEngine->renderPass
        };
        pipeline.create(linkedRenderEngine, &pipelineCreateInfo);
        deletionQueue.emplace_back([&] {
            pipeline.destroy();
        });
    }

private:
    /**@todo Write this better.*/
    void processNode(aiNode *node, const aiScene *scene) {
        for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
            vertices.reserve(scene->mMeshes[node->mMeshes[i]]->mNumVertices);
            for (uint32_t j = 0; j < scene->mMeshes[node->mMeshes[i]]->mNumVertices; ++j) {
                IEVertex temporaryVertex{};
                temporaryVertex.position.x = scene->mMeshes[node->mMeshes[i]]->mVertices[j].x;
                temporaryVertex.position.y = scene->mMeshes[node->mMeshes[i]]->mVertices[j].y;
                temporaryVertex.position.z = scene->mMeshes[node->mMeshes[i]]->mVertices[j].z;
                if (scene->mMeshes[node->mMeshes[i]]->HasNormals()) {
                    temporaryVertex.normal.x = scene->mMeshes[node->mMeshes[i]]->mNormals[j].x;
                    temporaryVertex.normal.y = scene->mMeshes[node->mMeshes[i]]->mNormals[j].y;
                    temporaryVertex.normal.z = scene->mMeshes[node->mMeshes[i]]->mNormals[j].z;
                }
                if (scene->mMeshes[node->mMeshes[i]]->mNumUVComponents[0] > 0) {
                    temporaryVertex.textureCoordinates.x = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[0][j].x;
                    temporaryVertex.textureCoordinates.y = scene->mMeshes[node->mMeshes[i]]->mTextureCoords[0][j].y;
                }
                vertices.push_back(temporaryVertex);
            }
            indices.reserve(static_cast<std::vector<uint32_t>::size_type>(scene->mMeshes[node->mMeshes[i]]->mNumFaces) * 3);
            for (; triangleCount < scene->mMeshes[node->mMeshes[i]]->mNumFaces; ++triangleCount) {
                for (uint32_t k = 0; k < scene->mMeshes[node->mMeshes[i]]->mFaces[triangleCount].mNumIndices; ++k) {
                    indices.push_back(scene->mMeshes[node->mMeshes[i]]->mFaces[triangleCount].mIndices[k]);
                }
            }
            if (scene->mMeshes[node->mMeshes[i]]->mMaterialIndex >= 0) {
                std::vector<std::pair<uint32_t *, aiTextureType>> textureTypes{
                    {&diffuseTexture, aiTextureType_DIFFUSE},
                    {&emissionTexture, aiTextureType_EMISSIVE},
                    {&heightTexture, aiTextureType_HEIGHT},
                    {&metallicTexture, aiTextureType_METALNESS},
                    {&normalTexture, aiTextureType_NORMALS},
                    {&roughnessTexture, aiTextureType_DIFFUSE_ROUGHNESS},
                    {&specularTexture, aiTextureType_SPECULAR}
                };
                for (std::pair<uint32_t *, aiTextureType> textureType : textureTypes) {
                    if (scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTextureCount(textureType.second) > 0) {
                        textures->reserve(scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTextureCount(textureType.second) + textures->size());
                        IETexture temporaryTexture{};
                        IETexture::CreateInfo textureCreateInfo{};
                        bool textureAlreadyLoaded{false};
                        aiString filename;
                        scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTexture(textureType.second, 0, &filename);
                        if (filename.length == 0) {
                            continue;
                        }
                        std::string texturePath{directory + '/' + std::string(filename.C_Str())};
                        for (uint32_t k = 0; k < textures->size(); ++k) {
                            if (std::strcmp((*textures)[k].createdWith.filename.c_str(), texturePath.c_str()) == 0) {
                                *textureType.first = k;
                                textureAlreadyLoaded = true;
                                break;
                            }
                        }
                        if (!textureAlreadyLoaded) {
                            int channels{};
                            textureCreateInfo.filename = texturePath;
                            textureCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
                            textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                            textureCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                            textureCreateInfo.allocationUsage = VMA_MEMORY_USAGE_GPU_ONLY;
                            textureCreateInfo.mipMapping = linkedRenderEngine->settings.mipMapping;
                            textureCreateInfo.imageType = VULKAN_TEXTURE;
                            textureCreateInfo.data = stbi_load(textureCreateInfo.filename.c_str(), &textureCreateInfo.height, &textureCreateInfo.width, &channels, STBI_rgb_alpha);
                            if (!textureCreateInfo.data) { throw std::runtime_error("failed to prepare texture image from file: " + textureCreateInfo.filename); }
                            temporaryTexture.create(linkedRenderEngine, &textureCreateInfo);
                            textures->push_back(temporaryTexture);
                            *textureType.first = textures->size() - 1;
                        }
                    }
                }
            }
        }
        /**@todo DON'T DO THIS! THIS IS TERRIBLE! ... I think.*/
        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            processNode(node->mChildren[i], scene);
        }
    }

    /**@todo Make this auto-detect shader stage.*/
    void loadShaders(const std::vector<const char *> &filenames) {
        shaderCreateInfos.clear();
        shaderCreateInfos.reserve(filenames.size());
        for (const char *filename : filenames) {
            IEShader::CreateInfo shaderCreateInfo {filename};
            shaderCreateInfos.push_back(shaderCreateInfo);
        }
    }
};