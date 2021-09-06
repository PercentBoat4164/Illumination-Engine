#pragma once

#include "openglProgram.hpp"
#include "openglTexture.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/detail/type_quat.hpp>

#include <cstddef>
#include <vector>
#include <unordered_map>

class OpenGLRenderable {
public:
    struct OpenGLMesh {
        struct OpenGLVertex {
            glm::vec3 position{0.0f, 0.0f, 0.0f};
            glm::vec2 textureCoordinates{0.0f, 0.0f};
            glm::vec3 normal{0.5f, 0.5f, 0.5f};
        };

        std::vector<unsigned int> indices{};
        std::vector<OpenGLVertex> vertices{};
        unsigned int diffuseTexture{};
        unsigned int emissionTexture{};
        unsigned int heightTexture{};
        unsigned int metallicTexture{};
        unsigned int normalTexture{};
        unsigned int roughnessTexture{};
        unsigned int specularTexture{};
        unsigned int vertexArrayObject;
        unsigned int vertexBuffer;
        unsigned int indexBuffer;
    };

    std::vector<const char *> shaderFilenames{"res/Shaders/OpenGLShaders/vertexShader.vert", "res/Shaders/OpenGLShaders/fragmentShader.frag"};
    std::vector<OpenGLTexture> textures{1};
    std::vector<OpenGLMesh> meshes{};
    std::vector<OpenGLShader> shaders{};
    OpenGLProgram program{};
    OpenGLGraphicsEngineLink *linkedRenderEngine{};
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::mat4 model{1.0f};
    const char *path{};
    bool render{true};
    bool loaded{false};
    bool uploaded{false};

    explicit OpenGLRenderable(OpenGLGraphicsEngineLink *engineLink, const char *filePath) {
        linkedRenderEngine = engineLink;
        path = filePath;
    }

    void create() {
        int channels{};
        OpenGLTexture::CreateInfo textureCreateInfo{};
        textureCreateInfo.filename = std::string("res/Models/NoTexture.png");
        textureCreateInfo.format = OPENGL_TEXTURE;
        textureCreateInfo.mipMapping = linkedRenderEngine->settings->mipMapping;
        textureCreateInfo.data = stbi_load(textureCreateInfo.filename.c_str(), &textureCreateInfo.height, &textureCreateInfo.width, &channels, STBI_rgb_alpha);
        textures[0].create(&textureCreateInfo);
    }

    void prepare() {
        Assimp::Importer importer{};
        const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_GenUVCoords | aiProcess_CalcTangentSpace | aiProcess_GenNormals);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { throw std::runtime_error("failed to prepare scene from file: " + std::string(path)); }
        std::string directory = std::string(path).substr(0, std::string(path
        ).find_last_of('/'));
        meshes.clear();
        meshes.reserve(scene->mNumMeshes);
        processNode(scene->mRootNode, scene, directory);
        deletionQueue.emplace_front([&] { for (OpenGLTexture texture : textures) { texture.destroy(); } });
        shaders.resize(shaderFilenames.size());
        for (uint32_t i = 0; i < shaderFilenames.size(); ++i) {
            OpenGLShader::CreateInfo shaderCreateInfo{shaderFilenames[i]};
            shaders[i].create(&shaderCreateInfo);
            shaders[i].compile();
        }
        OpenGLProgram::CreateInfo programCreateInfo{shaders};
        program.create(&programCreateInfo);
        loaded = true;
    }

    void upload() {
        for (OpenGLMesh &mesh : meshes) {
            glGenVertexArrays(1, &mesh.vertexArrayObject);
            unloadQueue.emplace_front([&] { glDeleteVertexArrays(1, &mesh.vertexArrayObject); });
            glGenBuffers(1, &mesh.vertexBuffer);
            unloadQueue.emplace_front([&] { glDeleteBuffers(1, &mesh.vertexBuffer); });
            glGenBuffers(1, &mesh.indexBuffer);
            unloadQueue.emplace_front([&] { glDeleteBuffers(1, &mesh.indexBuffer); });
            glBindVertexArray(mesh.vertexArrayObject);
            glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBuffer);
            glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(mesh.vertices.size() * sizeof(mesh.vertices[0])), mesh.vertices.data(), GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.indexBuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizei>(mesh.indices.size() * sizeof(mesh.indices[0])), mesh.indices.data(), GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(OpenGLMesh::OpenGLVertex), (void *)offsetof(OpenGLMesh::OpenGLVertex, position));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(OpenGLMesh::OpenGLVertex), (void *)offsetof(OpenGLMesh::OpenGLVertex, textureCoordinates));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(OpenGLMesh::OpenGLVertex), (void *)offsetof(OpenGLMesh::OpenGLVertex, normal));
        }
        for (OpenGLTexture &texture : textures) { texture.upload(); }
        uploaded = true;
    }

    void unload() {
        for (const std::function<void()> &function : unloadQueue) { function(); }
        unloadQueue.clear();
        textures.resize(1);
        uploaded = false;
    }

    void reprepare() {
        bool toUpload{false};
        if (uploaded) { toUpload = true; }
        if (loaded) {
            destroy();
            prepare();
            if (toUpload) { upload(); }
        } else {
            prepare();
            if (toUpload) { upload(); }
        }
    }

    void reupload() {
        if (uploaded) {
            unload();
            upload();
        } else { upload(); }
    }

    void update() {
        glm::quat quaternion = glm::quat(glm::radians(rotation));
        model = glm::translate(glm::rotate(glm::scale(glm::mat4(1.0f), scale), glm::angle(quaternion), glm::axis(quaternion)), position);
    }

    void destroy() {
        unload();
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
        loaded = false;
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
    std::deque<std::function<void()>> unloadQueue{};

    void processNode(aiNode *node, const aiScene *scene, const std::string& directory) {
        for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
            OpenGLMesh temporaryMesh{};
            temporaryMesh.vertices.reserve(scene->mMeshes[node->mMeshes[i]]->mNumVertices);
            for (unsigned int j = 0; j < scene->mMeshes[node->mMeshes[i]]->mNumVertices; ++j) {
                OpenGLMesh::OpenGLVertex temporaryVertex{};
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
                    if (scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTextureCount(textureType.second) > 0) {
                        textures.reserve(scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTextureCount(textureType.second) + textures.size());
                        OpenGLTexture temporaryTexture{};
                        int channels{};
                        bool textureAlreadyLoaded{false};
                        aiString filename;
                        scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTexture(textureType.second, 0, &filename);
                        if (filename.length == 0) { continue; }
                        std::string texturePath {directory + '/' + std::string(filename.C_Str())};
                        for (unsigned int k = 0; k < textures.size(); ++k) {
                            if (std::strcmp(textures[k].createdWith.filename.c_str(), texturePath.c_str()) == 0) {
                                *textureType.first = k;
                                textureAlreadyLoaded = true;
                                break;
                            }
                        }
                        if (!textureAlreadyLoaded) {
                            OpenGLTexture::CreateInfo textureCreateInfo{};
                            textureCreateInfo.filename = texturePath;
                            textureCreateInfo.format = OPENGL_TEXTURE;
                            textureCreateInfo.mipMapping = linkedRenderEngine->settings->mipMapping;
                            textureCreateInfo.data = stbi_load(textureCreateInfo.filename.c_str(), &textureCreateInfo.height, &textureCreateInfo.width, &channels, STBI_rgb_alpha);
                            if (!textureCreateInfo.data) { throw std::runtime_error("failed to prepare texture image from file: " + textureCreateInfo.filename); }
                            temporaryTexture.create(&textureCreateInfo);
                            textures.push_back(temporaryTexture);
                            *textureType.first = textures.size() - 1;
                        }
                    }
                }
            }
            meshes.push_back(temporaryMesh);
        }
        for (unsigned int i = 0; i < node->mNumChildren; ++i) { processNode(node->mChildren[i], scene, directory); }
    }
};