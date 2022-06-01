#pragma once

#include <cstdint>
#include <vector>
#include "IEVertex.hpp"
#include "IEMaterial.hpp"
#include "IEDescriptorSet.hpp"
#include "IEPipeline.hpp"
#include "Buffer/IEBuffer.hpp"

class IERenderEngine;

class IEMesh {
private:
	std::vector<IEVertex> vertices{};
	std::vector<uint32_t> indices{};
	uint32_t triangleCount{};
	std::shared_ptr<IEPipeline> pipeline{};
	std::vector<std::shared_ptr<IEShader>> shaders{};  // Should be moved to material
	std::shared_ptr<IEBuffer> vertexBuffer{};
	std::shared_ptr<IEBuffer> indexBuffer{};
	std::shared_ptr<IEMaterial> material{};
	std::vector<std::function<void()>> deletionQueue{};

public:
	IEMesh() = default;

	IEMesh(IERenderEngine *);

	static void setAPI(const IEAPI &API);


	static std::function<void(IEMesh &)> _create;

	void create(IERenderEngine *);

	void _openglCreate();

	void _vulkanCreate();


	static std::function<void(IEMesh &, const std::string &, const aiScene *, aiMesh *)> _loadFromDiskToRAM;

	void loadFromDiskToRAM(const std::string &, const aiScene *, aiMesh *);

	void _openglLoadFromDiskToRAM(const std::string &, const aiScene *, aiMesh *);

	void _vulkanLoadFromDiskToRAM(const std::string &, const aiScene *, aiMesh *);


	static std::function<void(IEMesh &)> _loadFromRAMToVRAM;

	void loadFromRAMToVRAM();

	void _openglLoadFromRAMToVRAM();

	void _vulkanLoadFromRAMToVRAM();


	static std::function<void(IEMesh &, uint32_t)> _update;

	void update(uint32_t);

	void _openglUpdate(uint32_t);

	void _vulkanUpdate(uint32_t);


	static std::function<void(IEMesh &)> _unloadFromVRAM;

	void unloadFromVRAM();

	void _openglUnloadFromVRAM();

	void _vulkanUnloadFromVRAM();


	static std::function<void(IEMesh &)> _unloadFromRAM;

	void unloadFromRAM();

	void _openglUnloadFromRAM();

	void _vulkanUnloadFromRAM();


	IERenderEngine *linkedRenderEngine{};
	std::shared_ptr<IEDescriptorSet> descriptorSet{};
};