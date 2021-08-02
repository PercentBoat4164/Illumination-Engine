/**@todo: Add support for multiple textures.
 * ie. normal maps, height maps, roughness maps, metallic maps and diffuse maps.
 */

#pragma once

#include "openglVertex.hpp"
#include "openglProgram.hpp"
#include "openglTexture.hpp"

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <../../../deps/tiny_obj_loader.h>
#endif

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "../../../deps/stb_image.h"
#endif

#include <../../../deps/assimp/include/assimp/Importer.hpp>
#include <../../../deps/assimp/include/assimp/scene.h>
#include <../../../deps/assimp/include/assimp/postprocess.h>

#include <cstddef>
#include <vector>
#include <unordered_map>

class OpenGLMesh {
public:
    std::vector<OpenGLVertex> vertices{};
    std::vector<unsigned int> indices{};
    std::vector<OpenGLTexture> textures{};
};

class OpenGLRenderable {
public:
    std::vector<const char *> textureFilenames{};
    std::vector<const char *> shaderFilenames{};
    const char *modelFilename{};
    std::vector<OpenGLTexture> textures{};
    std::vector<OpenGLMesh> meshes{};
    OpenGLProgram program{};
    OpenGLGraphicsEngineLink *linkedRenderEngine{};
    unsigned int vertexBuffer{};
    unsigned int vertexArrayObject{};
    unsigned int indexBuffer{};
    std::vector<uint32_t> indices{};
    std::vector<OpenGLVertex> vertices{};
    uint32_t triangleCount{};
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{};
    glm::mat4 model{};

    explicit OpenGLRenderable(OpenGLGraphicsEngineLink *engineLink, const char *modelPath, const std::vector<const char *>& texturePaths, const std::vector<const char *>& shaderPaths, glm::vec3 initialPosition = {0.0f, 0.0f, 0.0f}, glm::vec3 initialRotation = {0.0f, 0.0f, 0.0f}, glm::vec3 initialScale = {1.0f, 1.0f, 1.0f}) {
        linkedRenderEngine = engineLink;
        position = initialPosition;
        rotation = initialRotation;
        scale = initialScale;
        textureFilenames = texturePaths;
        modelFilename = modelPath;
        shaderFilenames = shaderPaths;
    }

    explicit OpenGLRenderable(OpenGLGraphicsEngineLink *engineLink, const char *filePath) {
        linkedRenderEngine = engineLink;
        Assimp::Importer importer{};
        const aiScene *scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { throw std::runtime_error("failed to import file: " + std::string(filePath)); }
        std::string directory = std::string(filePath).substr(0, std::string(filePath).find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode *node, const aiScene *scene) {
        meshes.resize(node->mNumMeshes + meshes.size());
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes[i].vertices.resize(mesh->mNumVertices + meshes[i].vertices.size());
            for (unsigned int j = 0; j < mesh->mNumVertices; ++j) {
                meshes[i].vertices[j].pos.x = mesh->mVertices[i].x;
                meshes[i].vertices[j].pos.y = mesh->mVertices[i].y;
                meshes[i].vertices[j].pos.z = mesh->mVertices[i].z;
                meshes[i].vertices[j].normal.x = mesh->mNormals[i].x;
                meshes[i].vertices[j].normal.y = mesh->mNormals[i].y;
                meshes[i].vertices[j].normal.z = mesh->mNormals[i].z;
                if (mesh->mTextureCoords[0]) {
                    meshes[i].vertices[j].texCoords.x = mesh->mTextureCoords[0][i].x;
                    meshes[i].vertices[j].texCoords.y = mesh->mTextureCoords[0][i].y;
                } else { meshes[i].vertices[j].texCoords = glm::vec2(0.0f, 0.0f); }
            }
            for (unsigned int j = 0; j < mesh->mNumFaces; ++j) {
                aiFace face = mesh->mFaces[j];
                meshes[i].indices.reserve(face.mNumIndices + meshes[i].indices.size());
                for (unsigned int k = 0; k < face.mNumIndices; ++k) {
                    meshes[i].indices.push_back(face.mIndices[j]);
                }
            }
            if (mesh->mMaterialIndex >= 0) {
                std::vector<std::pair<aiTextureType, OpenGLImageType>> textureTypes{
                    {aiTextureType_DIFFUSE, OPENGL_TEXTURE_DIFFUSE},
                    {aiTextureType_SPECULAR, OPENGL_TEXTURE_SPECULAR}
                };
                aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
                for (std::pair<aiTextureType, OpenGLImageType> textureType : textureTypes) {
                    for (unsigned int j = 0; j < material->GetTextureCount(textureType.first); ++j) {
                        aiString path;
                        material->GetTexture(textureType.first, j, &path);
                        OpenGLTexture::CreateInfo textureCreateInfo{};
                        textureCreateInfo.filename = path.C_Str();
                        textureCreateInfo.format = textureType.second;
                        meshes[i].textures[j].create(&textureCreateInfo);
                    }
                }
            }
        }
        for (unsigned int i = 0; i < node->mNumChildren; ++i) { processNode(node->mChildren[i], scene); }
    }

    void loadTextures(std::vector<const char *> filenames) {
        stbi_set_flip_vertically_on_load(true);
        int channels{0};
        textures.resize(filenames.size());
        for (uint32_t i = 0; i < filenames.size(); ++i) {
            OpenGLTexture::CreateInfo textureCreateInfo{};
            textureCreateInfo.filename = filenames[i];
            textureCreateInfo.format = OPENGL_TEXTURE_DIFFUSE;
            textureCreateInfo.data = stbi_load(textureCreateInfo.filename, &textureCreateInfo.width, &textureCreateInfo.height, &channels, STBI_rgb_alpha);
            textures[i].create(&textureCreateInfo);
            textures[i].upload();
        }
        deletionQueue.emplace_front([&] { for (OpenGLTexture& texture : textures) { texture.destroy(); } });
    }

    void loadModel(const char *filename = nullptr) {
        if (!filename) { filename = modelFilename; }
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
        std::unordered_map<OpenGLVertex, uint32_t> uniqueVertices{};
        uniqueVertices.reserve(reserveCount * (2 / 3)); // Also allocates too much space, but it will be deleted at the end of the function, so we don't care
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                OpenGLVertex vertex{};
                vertex.pos = { attrib.vertices[static_cast<std::vector<int>::size_type>(3) * index.vertex_index], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] };
                vertex.color = {1.0f, 1.0f, 1.0f, 0.0f};
                vertex.texCoords = {attrib.texcoords[static_cast<std::vector<int>::size_type>(2) * index.texcoord_index], attrib.texcoords[2 * index.texcoord_index + 1] };
                vertex.normal = { attrib.normals[static_cast<std::vector<int>::size_type>(3) * index.normal_index], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2] };
                if (uniqueVertices.find(vertex) == uniqueVertices.end()) {
                    uniqueVertices.insert({vertex, static_cast<uint32_t>(vertices.size())});
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        // Remove unneeded space at end of vertices at the last minute
        std::vector<OpenGLVertex> tmp = vertices;
        vertices.swap(tmp);
        triangleCount = static_cast<uint32_t>(indices.size()) / 3;
        glGenVertexArrays(1, &vertexArrayObject);
        deletionQueue.emplace_front([&] { glDeleteVertexArrays(1, &vertexArrayObject); });
        glGenBuffers(1, &vertexBuffer);
        deletionQueue.emplace_front([&] { glDeleteBuffers(1, &vertexBuffer); });
        glGenBuffers(1, &indexBuffer);
        deletionQueue.emplace_front([&] { glDeleteBuffers(1, &indexBuffer); });
        glBindVertexArray(vertexArrayObject);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(vertices.size() * sizeof(vertices[0])), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizei>(indices.size() * sizeof(indices[0])), indices.data(), GL_STATIC_DRAW);
        //Position data
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(OpenGLVertex), (void *)offsetof(OpenGLVertex, pos));
        glEnableVertexAttribArray(0);
        //Color data
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(OpenGLVertex), (void *)offsetof(OpenGLVertex, color));
        glEnableVertexAttribArray(1);
        //Texture Coordinates data
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(OpenGLVertex), (void *)offsetof(OpenGLVertex, texCoords));
        glEnableVertexAttribArray(2);
        //Normal Data
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(OpenGLVertex), (void *)offsetof(OpenGLVertex, normal));
        glEnableVertexAttribArray(3);
    }

    void loadShaders(std::vector<const char *> filenames) {
        std::vector<OpenGLShader> shaders{filenames.size()};
        for (uint32_t i = 0; i < filenames.size(); ++i) {
            OpenGLShader::CreateInfo shaderCreateInfo{filenames[i]};
            shaders[i].create(&shaderCreateInfo);
        }
        OpenGLProgram::CreateInfo programCreateInfo{shaders};
        program.create(&programCreateInfo);
        deletionQueue.emplace_front([&] { program.destroy(); });
    }

    void update() {
        glm::quat quaternion = glm::quat(glm::radians(rotation));
        model = glm::translate(glm::rotate(glm::scale(glm::mat4(1.0f), scale), glm::angle(quaternion), glm::axis(quaternion)), position);
    }

    void destroy() {
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};