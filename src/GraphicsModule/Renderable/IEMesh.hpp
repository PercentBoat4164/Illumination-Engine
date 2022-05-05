#pragma once

#include <cstdint>
#include <vector>
#include "IEVertex.hpp"
#include "IEMaterial.hpp"

class IERenderEngine;

class IEMesh {
private:
    std::vector<IEVertex> vertices{};
    std::vector<uint32_t> indices{};
    uint32_t triangleCount{};
    IEDescriptorSet descriptorSet{};
    IEAccelerationStructure accelerationStructure{};
    IEBuffer vertexBuffer{};
    IEBuffer indexBuffer{};
    IEMaterial material{};
    IERenderEngine *linkedRenderEngine{};
    std::vector<std::function<void()>> deletionQueue{};

    static std::function<void(IEMesh &)> _create;
    void _vulkanCreate() {}
    void _openglCreate() {}

    static std::function<void(IEMesh &, const std::string &, const aiScene *, aiMesh *)> _import;
    void _vulkanImport(const std::string &, const aiScene *, aiMesh *);
    void _openglImport(const std::string &, const aiScene *, aiMesh *);

    static std::function<void(IEMesh &)> _createVertexBuffer;
    void _openglCreateVertexBuffer() {}
    void _vulkanCreateVertexBuffer();

    static std::function<void(IEMesh &)> _createIndexBuffer;
    void _openglCreateIndexBuffer() {}
    void _vulkanCreateIndexBuffer();

public:
    void create(IERenderEngine *engineLink) {
        linkedRenderEngine = engineLink;
        return _create(*this);
    }

    void import(const std::string &directory, const aiScene *scene, aiMesh *mesh) {
//        return _import(*this, directory, scene, mesh);  // This is the line causing compilation errors.
    }

    void createVertexBuffer() {

    }

    void createIndexBuffer() {

    }

    void destroy() {
        for (const std::function<void()> &function: deletionQueue) {
            function();
        }
        deletionQueue.clear();
    }

    ~IEMesh() {
        destroy();
    }
};