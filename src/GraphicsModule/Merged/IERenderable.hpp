#pragma once

#include "IEVertex.hpp"
#include "IEShader.hpp"
#include "IEMesh.hpp"

#include <glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <sstream>

class IERenderable {
public:
    std::string filePath{};
    IERenderEngineLink* linkedRenderEngine{};
    std::vector<std::string> data{};
    std::vector<IETexture> textures{};
    std::vector<IEShader*> shaders{};
    std::vector<IEMesh> meshes{};

    explicit IERenderable(const std::string& file) {
        filePath = file;
    }

    void create(IERenderEngineLink* engineLink, const std::string& file = "") {
        linkedRenderEngine = engineLink;
        if (!file.empty()) {
            filePath = file;
        }
        std::ifstream fileObject{filePath, std::ios::in};
        std::stringstream contents{};
        contents << fileObject.rdbuf();
        fileObject.close();
        std::string contentsString{};
        contentsString = contents.str();
        while(!contentsString.empty()) {
            std::size_t pointer{contentsString.find_first_of('\n')};
            data.push_back(contentsString.substr(0, pointer));
            contentsString = contentsString.substr(pointer, contentsString.size() - pointer);
        }
    }

    void setShader(IEShader& shader) {
        if (shader.created.all()) {
            shaders[shader.language] = &shader;
        }
    }

    void processNode(aiNode *node, const aiScene *scene, const std::string& directory) {
        for (uint32_t i = 0; i < node->mNumMeshes; ++i) {
            IEMesh temporaryMesh{};
            temporaryMesh.vertices.reserve(scene->mMeshes[node->mMeshes[i]]->mNumVertices);
            for (uint32_t j = 0; j < scene->mMeshes[node->mMeshes[i]]->mNumVertices; ++j) {
                IEVertex temporaryVertex{};
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
            temporaryMesh.indices.reserve(static_cast<std::vector<uint32_t>::size_type>(scene->mMeshes[node->mMeshes[i]]->mNumFaces) * 3);
            for (; temporaryMesh.triangleCount < scene->mMeshes[node->mMeshes[i]]->mNumFaces; ++temporaryMesh.triangleCount) {
                for (uint32_t k = 0; k < scene->mMeshes[node->mMeshes[i]]->mFaces[temporaryMesh.triangleCount].mNumIndices; ++k) { temporaryMesh.indices.push_back(scene->mMeshes[node->mMeshes[i]]->mFaces[temporaryMesh.triangleCount].mIndices[k]); } }
            if (scene->mMeshes[node->mMeshes[i]]->mMaterialIndex >= 0) {
                std::vector<std::pair<uint32_t*, aiTextureType>> textureTypes{
                        {&temporaryMesh.diffuseTextureIndex, aiTextureType_DIFFUSE},
                        {&temporaryMesh.emissionTextureIndex, aiTextureType_EMISSIVE},
                        {&temporaryMesh.heightTextureIndex, aiTextureType_HEIGHT},
                        {&temporaryMesh.metallicTextureIndex, aiTextureType_METALNESS},
                        {&temporaryMesh.normalTextureIndex, aiTextureType_NORMALS},
                        {&temporaryMesh.roughnessTextureIndex, aiTextureType_DIFFUSE_ROUGHNESS},
                        {&temporaryMesh.specularTextureIndex, aiTextureType_SPECULAR}
                };
                for (std::pair<uint32_t*, aiTextureType> textureType : textureTypes) {
                    if (scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTextureCount(textureType.second) > 0) {
                        textures.reserve(scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTextureCount(textureType.second) + textures.size());
                        IETexture temporaryTexture{};
                        IETexture::CreateInfo textureCreateInfo{};
                        bool textureAlreadyLoaded{false};
                        aiString filename;
                        scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTexture(textureType.second, 0, &filename);
                        if (filename.length == 0) { continue; }
                        std::string texturePath{directory + '/' + std::string(filename.C_Str())};
                        for (uint32_t k = 0; k < textures.size(); ++k) {
                            if (std::strcmp(textures[k].createdWith.filename.c_str(), texturePath.c_str()) == 0) {
                                *textureType.first = k;
                                textureAlreadyLoaded = true;
                                break;
                            }
                        }
                        if (!textureAlreadyLoaded) {
                            uint32_t channels{};
                            textureCreateInfo.filename = texturePath;
                            textureCreateInfo.data = reinterpret_cast<char *>(stbi_load(textureCreateInfo.filename.c_str(), reinterpret_cast<int *>(&textureCreateInfo.height), reinterpret_cast<int *>(&textureCreateInfo.width), reinterpret_cast<int *>(&channels), STBI_rgb_alpha));
                            if (textureCreateInfo.data.empty()) { throw std::runtime_error("failed to prepare texture image from file: " + textureCreateInfo.filename); }
                            temporaryTexture.create(linkedRenderEngine, &textureCreateInfo);
                            textures.push_back(temporaryTexture);
                            *textureType.first = textures.size() - 1;
                        }
                    }
                }
            }
            meshes.push_back(temporaryMesh);
        }
        for (uint32_t i = 0; i < node->mNumChildren; ++i) { processNode(node->mChildren[i], scene, directory); }
    }
};
