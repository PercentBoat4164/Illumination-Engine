#pragma once

#include <cstdint>
#include <vector>
#include "IEVertex.hpp"
#include "IEMaterial.hpp"
#include "IEDescriptorSet.hpp"
#include "IEPipeline.hpp"
#include "Buffer/IEAccelerationStructure.hpp"
#include "Buffer/IEBuffer.hpp"

class IERenderEngine;

class IEMesh {
private:
	std::vector<IEVertex> vertices{};
	std::vector<uint32_t> indices{};
	uint32_t triangleCount{};
	// Should be moved to render enngine
	IEPipeline pipeline{};  // Should be moved to render engine
	std::vector<IEShader> shaders{};  // Should be moved to render engine
	IEAccelerationStructure accelerationStructure{};  // Should be moved to render engine?
	IEBuffer vertexBuffer{};
	IEBuffer indexBuffer{};
	IEMaterial material{};  // Should be moved to render engine?
	std::vector<std::function<void()>> deletionQueue{};
    IERenderEngine *linkedRenderEngine{};


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
	void create(IERenderEngine *engineLink);

	void loadFromDiskToRAM(const std::string &directory, const aiScene *scene, aiMesh *mesh);

	void loadFromRAMToVRAM();

	void update(uint32_t);

    void destroy();

    ~IEMesh();
};