#pragma once

#include "openglVertex.hpp"

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
    std::vector<unsigned int> textures{};
    GLuint programID{};
    GLuint modelMatrixID{};
    unsigned int vertexBuffer{};
    unsigned int vertexArrayObject{};
    unsigned int indexBuffer{};
    std::vector<uint32_t> indices{};
    std::vector<OpenGLVertex> vertices{};
    uint32_t triangleCount{};

    explicit OpenGLRenderable(const char *modelPath, const std::vector<const char *>& texturePaths, const std::vector<const char *>& shaderPaths) {
        textureFilenames = texturePaths;
        modelFilename = modelPath;
        shaderFilenames = shaderPaths;
    }

    void loadTextures(std::vector<const char *> filenames) {
        if (filenames.empty()) { filenames = textureFilenames; } else { textureFilenames = filenames; };
        textures.resize(filenames.size());
        for (unsigned int i = 0; i < filenames.size(); ++i) {
            glGenTextures(1, &textures[i]);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            stbi_set_flip_vertically_on_load(true);
            int channels{};
            stbi_uc *pixels = stbi_load(((std::string)filenames[i]).c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (!pixels) { throw std::runtime_error(("failed to load texture image from file: " + (std::string)filenames[i]).c_str()); }
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(pixels);
            glUseProgram(programID);
            glUniform1i(glGetUniformLocation(programID, "texture"), (GLint)i);
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
                vertex.texCoord = { attrib.texcoords[2 * index.texcoord_index], 1.f - attrib.texcoords[2 * index.texcoord_index + 1] };
                vertex.normal = { attrib.normals[3 * index.normal_index], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2] };
                vertex.color = {1.0f, 1.0f, 1.0f, 0.0f};
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    GLuint loadShaders(const std::vector<const char *>& filenames) {
        assert(!filenames.empty());
        GLuint vertexShaderID{glCreateShader(GL_VERTEX_SHADER)};
        GLuint fragmentShaderID{glCreateShader(GL_FRAGMENT_SHADER)};
        std::array<GLuint, 2> shaderIDs{vertexShaderID, fragmentShaderID};
        GLint Result{GL_FALSE};
        int InfoLogLength{0};
        for (unsigned int i = 0; i < filenames.size(); i++) {
            std::ifstream file(filenames[i], std::ios::in);
            if (!file.is_open()) { throw std::runtime_error("failed to load shader: " + (std::string)filenames[i]); }
            std::stringstream stringStream;
            stringStream << file.rdbuf();
            std::string shaderCode = stringStream.str();
            file.close();
            char const *sourcePointer = shaderCode.c_str();
            glShaderSource(shaderIDs[i], 1, &sourcePointer, nullptr);
            glCompileShader(shaderIDs[i]);
            glGetShaderiv(shaderIDs[i], GL_COMPILE_STATUS, &Result);
            glGetShaderiv(shaderIDs[i], GL_INFO_LOG_LENGTH, &InfoLogLength);
            if (InfoLogLength > 1) { throw std::runtime_error("failed to compile shader: " + (std::string)filenames[i]); }
        }
        programID = glCreateProgram();
        glAttachShader(programID, vertexShaderID);
        glAttachShader(programID, fragmentShaderID);
        glLinkProgram(programID);
        glGetProgramiv(programID, GL_LINK_STATUS, &Result);
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0){
            std::vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(programID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
            printf("%s\n", &ProgramErrorMessage[0]);
        }
        glDetachShader(programID, vertexShaderID);
        glDetachShader(programID, fragmentShaderID);
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        modelMatrixID = glGetUniformLocation(programID, "MVP");
    }
};