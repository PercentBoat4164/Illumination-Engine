#pragma once

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <fstream>
#include <cstring>

#if defined(_WIN32)
#define GLSLC "glslc.exe "
#else
#define GLSLC "glslc "
#endif


class Asset {

public:
    Asset(const char *modelName, const std::vector<const char *>& textureFileNames, const std::vector<const char *>& shaderFileNames, Settings *EngineSettings) {
        settings = EngineSettings;
        loadModel(modelName);
        loadTextures(textureFileNames);
        loadShaders(shaderFileNames);
        ubo = { glm::rotate(glm::mat4(1.0f), 0 * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f)), glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), glm::perspective(glm::radians(45.0f), settings->resolution[0] / (float) settings->resolution[1], 0.1f, 10.0f) };
        ubo.proj[1][1] *= -1;
        deletionQueue.emplace_front([&](const Asset& thisAsset){ vertexBuffer.destroy(); });
        deletionQueue.emplace_front([&](const Asset& thisAsset){ indexBuffer.destroy(); });
        deletionQueue.emplace_front([&](const Asset& thisAsset){ uniformBuffer.destroy(); });
        deletionQueue.emplace_front([&](const Asset& thisAsset){ for (AllocatedImage textureImage : textureImages) { textureImage.destroy(); } });
    }

    ~Asset() {
        if (autoCleanUp) {
            destroy();
        }
    }

    void destroy() {
        for (std::function<void(Asset)> &function : deletionQueue) { function(*this); }
        deletionQueue.clear();
    }

    void updateUbo() {
        memcpy(uniformBuffer.data, &ubo, sizeof(UniformBufferObject));
    }

    std::vector<uint32_t> indices{};
    std::vector<Vertex> vertices{};
    AllocatedBuffer vertexBuffer{};
    AllocatedBuffer indexBuffer{};
    VkPipeline graphicsPipeline{};
    VkDescriptorSet descriptorSet{};
    std::vector<stbi_uc *> textures{};
    int width{};
    int height{};
    std::vector<AllocatedImage> textureImages{};
    AllocatedBuffer uniformBuffer{};
    std::deque<std::function<void(Asset asset)>> deletionQueue;
    std::vector<std::vector<char>> shaderData{};
    VkDescriptorPool descriptorPool{};
    VkRenderPass renderPass{};
    std::vector<VkFramebuffer> framebuffers{};
    UniformBufferObject ubo{};

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
            deletionQueue.emplace_front([&](const Asset& thisAsset){ for (stbi_uc *pixels : textures) { stbi_image_free(pixels); } });
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
    bool autoCleanUp{false};
};