#pragma once

#if defined(_WIN32)
#define GLSLC "glslc.exe "
#else
#define GLSLC "glslc "
#endif

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <cstring>
#include <valarray>

#include <glm/glm.hpp>

#include "ImageManager.hpp"
#include "BufferManager.hpp"
#include "Camera.hpp"

#include "GPUData.hpp"
#include "PipelineManager.hpp"
#include "Vertex.hpp"

class Asset {
public:
    Asset(const char *modelFileName, const std::vector<const char *>& textureFileNames, const std::vector<const char *>& shaderFileNames, Settings *EngineSettings = new Settings{}, glm::vec3 initialPosition = {0, 0, 0}, glm::vec3 initialRotation = {0, 0, 0}, glm::vec3 initialScale = {1, 1, 1}) {
        position = initialPosition;
        rotation = initialRotation;
        absolutePath = EngineSettings->absolutePath;
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
        uniformBuffer.destroy();
        vertexBuffer.destroy();
        indexBuffer.destroy();
        for (ImageManager &textureImage : textureImages) { textureImage.destroy(); }
        for (const std::function<void(Asset)>& function : deletionQueue) { function(*this); }
        deletionQueue.clear();
    }

    void update(Camera camera) {
        uniformBufferObject = {glm::mat4(1.0f), camera.view, camera.proj};
        uniformBufferObject.model = glm::translate(glm::rotate(glm::rotate(glm::rotate(glm::scale(glm::mat4(1.0f), glm::abs(scale)), rotation[0], glm::vec3(1.f, 0.f, 0.f)), rotation[1], glm::vec3(0.f, 1.f, 0.f)), rotation[2], glm::vec3(0.f, 0.f, 1.f)), position);
        memcpy(uniformBuffer.data, &uniformBufferObject, sizeof(UniformBufferObject));
    }

    std::deque<std::function<void(Asset asset)>> deletionQueue{};
    std::vector<uint32_t> indices{};
    std::vector<Vertex> vertices{};
    BufferManager uniformBuffer{};
    BufferManager vertexBuffer{};
    BufferManager indexBuffer{};
    std::vector<PipelineManager> pipelineManagers{};
    UniformBufferObject uniformBufferObject{};
    std::vector<ImageManager> textureImages{};
    std::vector<stbi_uc *> textures{};
    std::vector<std::vector<char>> shaderData{};
    int width{};
    int height{};
    VkDescriptorSet descriptorSet{};
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{};
    bool render{true};
    uint32_t triangleCount{};

private:
    void loadModel(const char *filename) {
        vertices.clear();
        indices.clear();
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (absolutePath + (std::string)filename).c_str())) { throw std::runtime_error(warn + err); }
        uint32_t reserveCount{};
        for (const auto& shape : shapes) { reserveCount += shape.mesh.indices.size(); }
        indices.reserve(reserveCount);
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.pos = { attrib.vertices[3 * index.vertex_index], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] };
                vertex.texCoord = { attrib.texcoords[2 * index.texcoord_index], 1.f - attrib.texcoords[2 * index.texcoord_index + 1] };
                vertex.normal = { attrib.normals[3 * index.normal_index], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2] };
                vertex.color = {1.f, 1.f, 1.f};
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = vertices.size();
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        triangleCount = indices.size() / 3;
    }

    void loadTextures(const std::vector<const char *>& filenames) {
        textures.clear();
        textures.reserve(filenames.size());
        for (const char *filename : filenames) {
            int channels{};
            stbi_uc *pixels = stbi_load((absolutePath + (std::string)filename).c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { throw std::runtime_error(("failed to load texture image from file: " + absolutePath + (std::string)filename).c_str()); }
            textures.push_back(pixels);
        }
    }

    void loadShaders(const std::vector<const char *>& filenames, bool compile = true) {
        shaderData.clear();
        shaderData.reserve(filenames.size());
        for (const char *shaderName : shaderNames) {
            std::string compiledFileName = ((std::string) shaderName).substr(0, sizeof(shaderName) - 4) + "spv";
            if (compile) { if (system((GLSLC + absolutePath + shaderName + " -o " + absolutePath + compiledFileName).c_str()) != 0) { throw std::runtime_error("failed to compile shaders!"); } }
            std::ifstream file(absolutePath + compiledFileName, std::ios::ate | std::ios::binary);
            if (!file.is_open()) { throw std::runtime_error("failed to open file: " + compiledFileName.append("\n as file: " + absolutePath + compiledFileName)); }
            size_t fileSize = (size_t) file.tellg();
            std::vector<char> buffer(fileSize);
            file.seekg(0);
            file.read(buffer.data(), (std::streamsize)fileSize);
            file.close();
            shaderData.push_back(buffer);
        }
    }

    std::vector<const char *> shaderNames{};
    std::vector<const char *> textureNames{};
    std::string absolutePath{};
    const char *modelName{};
};