#pragma once

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <cstring>
#include <valarray>

#if defined(_WIN32)
#define GLSLC "glslc.exe "
#else
#define GLSLC "glslc "
#endif


class Asset {

public:
    Asset(const char *modelName, const std::vector<const char *>& textureFileNames, const std::vector<const char *>& shaderFileNames, Settings *EngineSettings, glm::vec3 initialPosition, glm::vec3 initialRotation, glm::vec3 initialScale) {
        //TODO: Create an option to re-load an asset from disk.
        // This will enable developers to edit shaders and upload them to see how they look without restarting the engine.
        position = initialPosition;
        rotation = initialRotation;
        scale = initialScale;
        settings = EngineSettings;
        loadModel(modelName);
        loadTextures(textureFileNames);
        loadShaders(shaderFileNames);
    }

    void destroy() {
        uniformBuffer.destroy();
        for (AllocatedImage textureImage : textureImages) { textureImage.destroy(); }
        for (stbi_uc *pixels : textures) { stbi_image_free(pixels); }
        for (const std::function<void(Asset)>& function : deletionQueue) { function(*this); }
        deletionQueue.clear();
    }

    void update() {
        //TODO: Make this take a Camera object that determines the other values of the UBO
        uniformBufferObject = {glm::mat4(1.0f), glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), glm::perspective(glm::radians(45.0f), settings->resolution[0] / (float) settings->resolution[1], 0.1f, 1000.0f) };
        uniformBufferObject.model = glm::translate(glm::rotate(glm::rotate(glm::rotate(glm::scale(glm::mat4(1.0f), glm::abs(scale)), rotation[0], glm::vec3(1.f, 0.f, 0.f)), rotation[1], glm::vec3(0.f, 1.f, 0.f)), rotation[2], glm::vec3(0.f, 0.f, 1.f)), position);
        uniformBufferObject.proj[1][1] *= -1;
        memcpy(uniformBuffer.data, &uniformBufferObject, sizeof(UniformBufferObject));
    }

    std::deque<std::function<void(Asset asset)>> deletionQueue;
    std::vector<uint32_t> indices{};
    std::vector<Vertex> vertices{};
    AllocatedBuffer uniformBuffer{};
    UniformBufferObject uniformBufferObject{};
    std::vector<AllocatedImage> textureImages{};
    std::vector<stbi_uc *> textures{};
    std::vector<std::vector<char>> shaderData{};
    int width{};
    int height{};
    VkPipeline graphicsPipeline{};
    VkDescriptorSet descriptorSet{};
    VkDescriptorPool descriptorPool{};
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{};

    unsigned long vertexOffset{};
private:
    void loadModel(const char *filename) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (settings->absolutePath + (std::string)filename).c_str())) { throw std::runtime_error(warn + err); }
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.pos = {attrib.vertices[3 * index.vertex_index], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2]};
                vertex.texCoord = {attrib.texcoords[2 * index.texcoord_index], 1.f - attrib.texcoords[2 * index.texcoord_index + 1]};
                //vertex.normal = {attrib.normals[3 * index.normal_index], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2]};
                vertex.color = {1.f, 1.f, 1.f};
                if (uniqueVertices.count(vertex) == 0) {
                    uniqueVertices[vertex] = vertices.size();
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    void loadTextures(const std::vector<const char *>& filenames) {
        for (const char *filename : filenames) {
            int channels{};
            stbi_uc *pixels = stbi_load((settings->absolutePath + (std::string)filename).c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { throw std::runtime_error(("failed to load texture image from file: " + settings->absolutePath + (std::string)filename).c_str()); }
            textures.push_back(pixels);
        }
    }

    void loadShaders(const std::vector<const char *>& filenames, bool compile = true) {
        std::copy(filenames.begin(), filenames.end(), back_inserter(shaderNames));
        for (const char *shaderName : shaderNames) {
            std::string compiledFileName = ((std::string) shaderName).substr(0, sizeof(shaderName) - 4) + "spv";
            if (compile) { system((GLSLC + settings->absolutePath + shaderName + " -o " + settings->absolutePath + compiledFileName).c_str()); }
            std::ifstream file(settings->absolutePath + compiledFileName, std::ios::ate | std::ios::binary);
            if (!file.is_open()) { throw std::runtime_error("failed to open file: " + compiledFileName.append("\n as file: " + settings->absolutePath + compiledFileName)); }
            size_t fileSize = (size_t) file.tellg();
            std::vector<char> buffer(fileSize);
            file.seekg(0);
            file.read(buffer.data(), fileSize);
            file.close();
            shaderData.push_back(buffer);
        }
    }

    Settings *settings{};
    std::vector<const char *> shaderNames{};
};