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
	// Make all shared pointers
	std::shared_ptr<IEPipeline> pipeline{};  // Should be moved to render engine
	std::vector<IEShader> shaders{};  // Should be moved to render engine
	IEAccelerationStructure accelerationStructure{};  // Should be moved to render engine?
	IEBuffer vertexBuffer{};
	IEBuffer indexBuffer{};
	IEMaterial material{};  // Should be moved to render engine?
	std::vector<std::function<void()>> deletionQueue{};


    static std::function<void(IEMesh &)> _create;

	void _vulkanCreate() {}

	void _openglCreate() {}

public:
	void create(IERenderEngine *engineLink);

	void loadFromDiskToRAM(const std::string &directory, const aiScene *scene, aiMesh *mesh);

	void loadFromRAMToVRAM();

	void update(uint32_t);

    void destroy();

    ~IEMesh();

    IERenderEngine *linkedRenderEngine{};

    IEDescriptorSet descriptorSet;
};