#pragma once

#include <glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <sstream>

class IERenderable {
public:
    struct IeMesh {
    public:
        struct IeVertex {
            glm::vec3 position{};
            glm::vec2 textureCoordinates{};
            glm::vec3 normal{};

            #ifdef ILLUMINATION_ENGINE_VULKAN
            static VkVertexInputBindingDescription getBindingDescription() {
                VkVertexInputBindingDescription bindingDescription{};
                bindingDescription.binding = 0;
                bindingDescription.stride = sizeof(IeVertex);
                bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
                return bindingDescription;
            }

            static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
                std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
                attributeDescriptions[0].binding = 0;
                attributeDescriptions[0].location = 0;
                attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[0].offset = offsetof(IeVertex, position);
                attributeDescriptions[1].binding = 0;
                attributeDescriptions[1].location = 2;
                attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
                attributeDescriptions[1].offset = offsetof(IeVertex, textureCoordinates);
                attributeDescriptions[2].binding = 0;
                attributeDescriptions[2].location = 3;
                attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
                attributeDescriptions[2].offset = offsetof(IeVertex, normal);
                return attributeDescriptions;
            }
            #endif
        };

        std::vector<uint32_t> indicies{};
        std::vector<IeVertex> vertices{};
        uint32_t diffuseTextureIndex{};
        uint32_t emissionTextureIndex{};
        uint32_t heightTextureIndex{};
        uint32_t metallicTextureIndex{};
        uint32_t normalTextureIndex{};
        uint32_t roughnessTextureIndex{};
        uint32_t specularTextureIndex{};
        uint32_t triangleCount{};
        IEBuffer vertexBuffer{};
        IEBuffer indexBuffer{};

        void destroy() {
            vertexBuffer.destroy();
            indexBuffer.destroy();
        }

        IeMesh load(const std::string& filePath) {
            Assimp::Importer importer{};
            const aiScene *scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_GenUVCoords | aiProcess_CalcTangentSpace | aiProcess_GenNormals);
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { throw std::runtime_error("failed to prepare texture image from file: " + std::string(filePath)); }
            std::string directory = std::string(filePath).substr(0, std::string(filePath).find_last_of('/'));
            return *this;
        }

        IeMesh threadedLoad(const std::string& filePath) {
            Assimp::Importer importer{};
            const aiScene *scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_GenUVCoords | aiProcess_CalcTangentSpace | aiProcess_GenNormals);
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) { throw std::runtime_error("failed to prepare texture image from file: " + std::string(filePath)); }
            std::string directory = std::string(filePath).substr(0, std::string(filePath).find_last_of('/'));
            return *this;
        }
    };

    std::string filePath{};
    IERenderEngineLink& renderEngineLink;
    std::vector<std::string> data{};

    void create(const IERenderEngineLink& engineLink, const std::string& file) {
        renderEngineLink = engineLink;
        filePath = file;
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
};
