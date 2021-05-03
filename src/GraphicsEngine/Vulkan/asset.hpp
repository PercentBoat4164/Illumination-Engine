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
#include <glm/gtc/quaternion.hpp>

#include "imageManager.hpp"
#include "bufferManager.hpp"
#include "camera.hpp"
#include "gpuData.hpp"
#include "rasterizationPipelineManager.hpp"
#include "vertex.hpp"

/** This class holds the methods to manage the window that was created.*/
class Asset {
public:
    /** This method sets the variables used in this class.
     * @param initialPosition This variable holds the initial placement of the object.
     * @param initialRotation This variable holds the initial rotation of the object.
     * @param initialScale This variable holds the initial scale of the object.
     * @param modelFileName This is the name of the model file.
     * @param textureFileNames This is the name of the texture files.
     * @param shaderFileNames This is the name of the shader files.*/
    Asset(const char *modelFileName, const std::vector<const char *>& textureFileNames, const std::vector<const char *>& shaderFileNames, glm::vec3 initialPosition = {0, 0, 0}, glm::vec3 initialRotation = {0, 0, 0}, glm::vec3 initialScale = {1, 1, 1}) {
        position = initialPosition;
        rotation = initialRotation;
        scale = initialScale;
        modelName = modelFileName;
        textureNames = textureFileNames;
        shaderNames = shaderFileNames;
        loadModel(modelFileName);
        loadTextures(textureFileNames);
        loadShaders(shaderFileNames);
    }

    /** This method reloads the model, textures, and shaders.
     * @param modelFileName This is the name of the model file.
     * @param textureFileNames This is the name of the texture files.
     * @param shaderFileNames This is the name of the shader files.*/
    void reloadAsset(const char *modelFileName = nullptr, const std::vector<const char *> *textureFileNames = nullptr, const std::vector<const char *> *shaderFileNames = nullptr) {
        if (modelFileName != nullptr) { modelName = modelFileName; }
        if (textureFileNames != nullptr) { textureNames = *textureFileNames; }
        if (shaderFileNames != nullptr) { shaderNames = *shaderFileNames; }
        loadModel(modelName);
        loadTextures(textureNames);
        loadShaders(shaderNames);
    }

    /** This method destroys the program and loaded textures.*/
    void destroy() {
        for (ImageManager &textureImage : textureImages) { textureImage.destroy(); }
        for (const std::function<void(Asset)>& function : deletionQueue) { function(*this); }
        deletionQueue.clear();
    }

    /** This method updates/renders the program.
     * @param camera This is the camera that the user is looking through in the program.*/
    void update(Camera camera) {
        uniformBufferObject = {glm::mat4(1.0f), camera.view, camera.proj};
        glm::quat quaternion = glm::quat(glm::radians(rotation));
        uniformBufferObject.model = glm::translate(glm::rotate(glm::scale(glm::mat4(1.0f), scale), glm::angle(quaternion), glm::axis(quaternion)), position);
        memcpy(uniformBuffer.data, &uniformBufferObject, sizeof(UniformBufferObject));
    }

    /** This variable holds the deletion queue for the destroy() method.*/
    std::deque<std::function<void(Asset asset)>> deletionQueue{};
    /** This variable holds the indices.*/
    std::vector<uint32_t> indices{};
    /** This variable holds the vertices*/
    std::vector<Vertex> vertices{};
    /** This is a buffer manager named uniformBuffer{}.*/
    BufferManager uniformBuffer{};
    /** This is a buffer manager named vertexBuffer{}.*/
    BufferManager vertexBuffer{};
    /** This is a buffer manager named indexBuffer{}.*/
    BufferManager indexBuffer{};
    /** This is a buffer manager named transformationBuffer{}.*/
    BufferManager transformationBuffer{};
    /** This variable holds the pipeline managers.*/
    std::vector<RasterizationPipelineManager> pipelineManagers{};
    /** This is a uniform buffer object.*/
    UniformBufferObject uniformBufferObject{};
    /** This variable holds the textures images.*/
    std::vector<ImageManager> textureImages{};
    /** This variable holds the textures.*/
    std::vector<stbi_uc *> textures{};
    /** This variable holds the shader data.*/
    std::vector<std::vector<char>> shaderData{};
    /** This variable is used to set the width of the texture.*/
    int width{};
    /** This variable is used to set the height of the texture.*/
    int height{};
    /** This is a Vulkan descriptor set.*/
    VkDescriptorSet descriptorSet{};
    /** This is a vector3 called position.*/
    glm::vec3 position{};
    /** This is a vector3 called rotation.*/
    glm::vec3 rotation{};
    /** This is a vector3 called scale.*/
    glm::vec3 scale{};
    /** This variable tells the program whether or not to render.*/
    bool render{true};
    /** This is the number of triangles.*/
    uint32_t triangleCount{};
    /** This is a Vulkan transformation matrix.*/
    VkTransformMatrixKHR transformationMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

private:
    /** This method loads the model that is inputted.
     * @param filename This is the filename of the model.*/
    void loadModel(const char *filename) {
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
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};
        uniqueVertices.reserve(reserveCount * (2 / 3)); // Also allocates too much space, but it will be deleted at the end of the function, so we don't care
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.pos = { attrib.vertices[3 * index.vertex_index], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] };
                vertex.texCoord = { attrib.texcoords[2 * index.texcoord_index], 1.f - attrib.texcoords[2 * index.texcoord_index + 1] };
                vertex.normal = { attrib.normals[3 * index.normal_index], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2] };
                vertex.color = {1.f, 1.f, 1.f};
                if (uniqueVertices.find(vertex) == uniqueVertices.end()) {
                    uniqueVertices.insert({vertex, static_cast<uint32_t>(vertices.size())});
                    vertices.push_back(vertex);
                }
                indices.push_back(uniqueVertices[vertex]);
            }
        }
        // Remove unneeded space at end of vertices at the last minute
        std::vector<Vertex> tmp = vertices;
        vertices.swap(tmp);
        triangleCount = static_cast<uint32_t>(indices.size()) / 3;
    }

    /** This method loads the textures that are inputted into the program.
     * @param filenames These are the filenames of the textures that are being loaded.*/
    void loadTextures(const std::vector<const char *>& filenames) {
        textures.clear();
        textures.reserve(filenames.size());
        for (const char *filename : filenames) {
            int channels{};
            stbi_uc *pixels = stbi_load(((std::string)filename).c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { throw std::runtime_error(("failed to load texture image from file: " + (std::string)filename).c_str()); }
            textures.push_back(pixels);
        }
    }

    /** This method loads the shaders that are inputted into the program
     * @param filenames These are the filenames of the shaders that are being loaded.
     * @param compile This variable tells the method whether or not to compile the shaders.*/
    void loadShaders(const std::vector<const char *>& filenames, bool compile = true) {
        shaderData.clear();
        shaderData.reserve(filenames.size());
        for (const char *shaderName : shaderNames) {
            std::string compiledFileName = ((std::string) shaderName).substr(0, sizeof(shaderName) - 4) + "spv";
            if (compile) { if (system((GLSLC + (std::string)shaderName + " -o " + compiledFileName).c_str()) != 0) { throw std::runtime_error("failed to compile Shaders!"); } }
            std::ifstream file(compiledFileName, std::ios::ate | std::ios::binary);
            if (!file.is_open()) { throw std::runtime_error("failed to open file: " + compiledFileName.append("\n as file: " + compiledFileName)); }
            size_t fileSize = (size_t) file.tellg();
            std::vector<char> buffer(fileSize);
            file.seekg(0);
            file.read(buffer.data(), (std::streamsize)fileSize);
            file.close();
            shaderData.push_back(buffer);
        }
    }

    /** This variable holds the shader names.*/
    std::vector<const char *> shaderNames{};
    /** This variable holds the texture names.*/
    std::vector<const char *> textureNames{};
    /** This variable holds the model name.*/
    const char *modelName{};
};