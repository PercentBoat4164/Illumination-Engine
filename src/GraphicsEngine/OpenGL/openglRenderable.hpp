/**@todo: Add a graphics engine link
 * - HIGH PRIORITY: Will enable a better code structure
 */


#pragma once

#include "openglVertex.hpp"
#include "openglProgram.hpp"

#ifndef TINYOBJLOADER_IMPLEMENTATION
#define TINYOBJLOADER_IMPLEMENTATION
#include <../../../deps/tiny_obj_loader.h>
#endif

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "../../../deps/stb_image.h"
#endif

#include <vector>
#include <unordered_map>

class OpenGLRenderable {
public:
    int width{}, height{};
    std::vector<const char *> textureFilenames{};
    std::vector<const char *> shaderFilenames{};
    const char *modelFilename{};
    std::vector<unsigned int> textureIDs{};
    OpenGLProgram program{};
    unsigned int vertexBuffer{};
    unsigned int vertexArrayObject{};
    unsigned int indexBuffer{};
    std::vector<uint32_t> indices{};
    std::vector<OpenGLVertex> vertices{};
    uint32_t triangleCount{};
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{};

    explicit OpenGLRenderable(const char *modelPath, const std::vector<const char *>& texturePaths, const std::vector<const char *>& shaderPaths, glm::vec3 initialPosition = {0.0f, 0.0f, 0.0f}, glm::vec3 initialRotation = {0.0f, 0.0f, 0.0f}, glm::vec3 initialScale = {1.0f, 1.0f, 1.0f}) {
        position = initialPosition;
        rotation = initialRotation;
        scale = initialScale;
        textureFilenames = texturePaths;
        modelFilename = modelPath;
        shaderFilenames = shaderPaths;
        std::vector<OpenGLShader> shaders{shaderFilenames.size()};
        for (uint32_t i = 0; i < shaderFilenames.size(); ++i) {
            OpenGLShader::CreateInfo shaderCreateInfo{shaderFilenames[i]};
            shaders[i].create(&shaderCreateInfo);
        }
        OpenGLProgram::CreateInfo programCreateInfo{shaders};
        program.create(&programCreateInfo);
    }

    void loadTextures(std::vector<const char *> filenames) {
        if (filenames.empty()) { filenames = textureFilenames; } else { textureFilenames = filenames; }
        textureIDs.resize(filenames.size());
        glGenTextures((GLsizei)textureIDs.size(), textureIDs.data());
        for (unsigned int i = 0; i < filenames.size(); ++i) {
            glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
            stbi_set_flip_vertically_on_load(true);
            int channels{};
            stbi_uc *pixels = stbi_load(((std::string)filenames[i]).c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { throw std::runtime_error(("failed to load texture image from file: " + (std::string)filenames[i]).c_str()); }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(pixels);
        }
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
                vertex.pos = { attrib.vertices[3 * index.vertex_index], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2] };
                vertex.color = {1.0f, 1.0f, 1.0f, 0.0f};
                vertex.texCoords = {attrib.texcoords[2 * index.texcoord_index], attrib.texcoords[2 * index.texcoord_index + 1] };
                vertex.normal = { attrib.normals[3 * index.normal_index], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2] };
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
        glGenBuffers(1, &vertexBuffer);
        glGenBuffers(1, &indexBuffer);
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
};