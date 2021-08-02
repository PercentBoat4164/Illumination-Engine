/**@todo: Add support for multiple textures.
 * ie. normal maps, height maps, roughness maps, metallic maps and diffuse maps.
 */

#pragma once

#include "openglProgram.hpp"
#include "openglTexture.hpp"

#include <../../../deps/assimp/include/assimp/Importer.hpp>
#include <../../../deps/assimp/include/assimp/scene.h>
#include <../../../deps/assimp/include/assimp/postprocess.h>

#include <cstddef>
#include <vector>
#include <unordered_map>

class OpenGLRenderable {
public:
    struct OpenGLMesh {
    public:
        struct OpenGLVertex {
            glm::vec3 position{0.0f, 0.0f, 0.0f};
            glm::vec2 textureCoordinates{0.0f, 0.0f};
            glm::vec3 normal{0.5f, 0.5f, 0.5f};
        };

        std::vector<unsigned int> indices{};
        std::vector<OpenGLVertex> vertices{};
        std::vector<unsigned int> textureIndices{};
        unsigned int vertexArrayObject;
        unsigned int vertexBuffer;
        unsigned int indexBuffer;
    };

    std::vector<const char *> shaderFilenames{"res/Shaders/OpenGLShaders/vertexShader.vert", "res/Shaders/OpenGLShaders/fragmentShader.frag"};
    std::vector<OpenGLTexture> textures{};
    std::vector<OpenGLMesh> meshes{};
    OpenGLProgram program{};
    glm::vec3 position{0.0f, 0.0f, 0.0f};
    glm::vec3 rotation{0.0f, 0.0f, 0.0f};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::mat4 model{1.0f};
    bool render{true};

    explicit OpenGLRenderable(const char *filePath) {
        Assimp::Importer importer{};
        const aiScene *scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_GenUVCoords | aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { throw std::runtime_error("failed to import file: " + std::string(filePath)); }
        std::string directory = std::string(filePath).substr(0, std::string(filePath).find_last_of('/'));
        meshes.reserve(scene->mNumMeshes);
        processNode(scene->mRootNode, scene, directory);
    }

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
                std::vector<std::pair<aiTextureType, OpenGLImageType>> textureTypes{
                    {aiTextureType_AMBIENT, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_AMBIENT_OCCLUSION, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_BASE_COLOR, OPENGL_TEXTURE_DIFFUSE},
                    {aiTextureType_DIFFUSE, OPENGL_TEXTURE_DIFFUSE},
                    {aiTextureType_DIFFUSE_ROUGHNESS, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_DISPLACEMENT, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_EMISSION_COLOR, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_EMISSIVE, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_HEIGHT, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_LIGHTMAP, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_METALNESS, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_NONE, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_NORMALS, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_NORMAL_CAMERA, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_OPACITY, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_REFLECTION, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_SHININESS, OPENGL_TEXTURE_UNKNOWN},
                    {aiTextureType_SPECULAR, OPENGL_TEXTURE_SPECULAR},
                    {aiTextureType_UNKNOWN, OPENGL_TEXTURE_UNKNOWN}
                };
                for (std::pair<aiTextureType, OpenGLImageType> textureType : textureTypes) {
                    textures.reserve(scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTextureCount(textureType.first) + textures.size());
                    temporaryMesh.textureIndices.reserve(scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTextureCount(textureType.first) + temporaryMesh.textureIndices.size());
                    OpenGLTexture temporaryTexture{};
                    for (unsigned int j = 0; j < scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTextureCount(textureType.first); ++j) {
                        int channels{};
                        bool skip{false};
                        OpenGLTexture::CreateInfo textureCreateInfo{};
                        aiString filename;
                        scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]->GetTexture(textureType.first, j, &filename);
                        std::string path {directory + '/' + std::string(filename.C_Str())};
                        textureCreateInfo.filename = std::string(path);
                        for (unsigned int k = 0; k < textures.size(); ++k) {
                            if (std::strcmp(textures[k].createdWith.filename.c_str(), path.c_str()) == 0) {
                                temporaryMesh.textureIndices.push_back(k);
                                skip = true;
                                break;
                            }
                        }
                        if (!skip) {
                            textureCreateInfo.format = textureType.second;
                            textureCreateInfo.data = stbi_load(textureCreateInfo.filename.c_str(), &textureCreateInfo.height, &textureCreateInfo.width, &channels, STBI_rgb_alpha);
                            if (!textureCreateInfo.data) { throw std::runtime_error("failed to open file: " + std::string(textureCreateInfo.filename)); }
                            temporaryTexture.create(&textureCreateInfo);
                            textures.push_back(temporaryTexture);
                            temporaryMesh.textureIndices.push_back(textures.size() - 1);
                        }
                    }
                }
            }
            if (temporaryMesh.textureIndices.empty()) {
                bool skip{false};
                for (unsigned int k = 0; k < textures.size(); ++k) {
                    if (std::strcmp(textures[k].createdWith.filename.c_str(), "res/Models/NoTexture.png") == 0) {
                        temporaryMesh.textureIndices.push_back(k);
                        skip = true;
                        break;
                    }
                }
                if (!skip) {
                    int channels{};
                    OpenGLTexture temporaryTexture{};
                    OpenGLTexture::CreateInfo textureCreateInfo{};
                    textureCreateInfo.filename = std::string("res/Models/NoTexture.png");
                    textureCreateInfo.format = OPENGL_TEXTURE_UNKNOWN;
                    textureCreateInfo.data = stbi_load(textureCreateInfo.filename.c_str(), &textureCreateInfo.height, &textureCreateInfo.width, &channels, STBI_rgb_alpha);
                    temporaryTexture.create(&textureCreateInfo);
                    textures.push_back(temporaryTexture);
                    temporaryMesh.textureIndices.push_back(textures.size() - 1);
                }
            }
            meshes.push_back(temporaryMesh);
        }
        for (unsigned int i = 0; i < node->mNumChildren; ++i) { processNode(node->mChildren[i], scene, directory); }
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

    void upload() {
        for (OpenGLMesh &mesh : meshes) {
            glGenVertexArrays(1, &mesh.vertexArrayObject);
            deletionQueue.emplace_front([&] { glDeleteVertexArrays(1, &mesh.vertexArrayObject); });
            glGenBuffers(1, &mesh.vertexBuffer);
            deletionQueue.emplace_front([&] { glDeleteBuffers(1, &mesh.vertexBuffer); });
            glGenBuffers(1, &mesh.indexBuffer);
            deletionQueue.emplace_front([&] { glDeleteBuffers(1, &mesh.indexBuffer); });
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