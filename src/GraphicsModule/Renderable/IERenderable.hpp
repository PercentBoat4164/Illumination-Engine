#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class aiScene;

class IECamera;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "Buffer/IEBuffer.hpp"
#include "Image/IETexture.hpp"
#include "IEDescriptorSet.hpp"
#include "IEMaterial.hpp"
#include "IEPipeline.hpp"
#include "IEShader.hpp"
#include "IEUniformBufferObject.hpp"
#include "IEVertex.hpp"

// Modular dependencies
#include "Core/AssetModule/IEAspect.hpp"
#include "IEMesh.hpp"

// External dependencies
#include <assimp/Importer.hpp>

#include <vulkan/vulkan.h>

// System dependencies
#include <string>
#include <functional>


typedef enum IERenderableStatus {
	IE_RENDERABLE_STATE_UNKNOWN = 0x0,
	IE_RENDERABLE_STATE_UNLOADED = 0x1,
	IE_RENDERABLE_STATE_IN_RAM = 0x2,
	IE_RENDERABLE_STATE_IN_VRAM = 0x4
} IERenderableStatus;

class IERenderable : public IEAspect {
public:
	std::string modelName{};
	std::vector<IEMesh> meshes{};
	std::vector<std::function<void()>> deletionQueue{};
	IEBuffer modelBuffer{};
	IERenderEngine *linkedRenderEngine{};
	IEUniformBufferObject uniformBufferObject{};
	std::vector<IEShader> shaders{};
	Assimp::Importer importer{};
	bool render{true};
	uint32_t commandBufferIndex{};
	std::string directory{};
	std::vector<glm::mat4> modelMatrices{};
	IERenderableStatus status;

	IERenderable() = default;

	IERenderable(IERenderEngine *, const std::string &);

	static void setAPI(const IEAPI &);

	void create(IERenderEngine *, const std::string &);

	void loadFromDiskToRAM();

	void loadFromRAMToVRAM();

	void update(uint32_t);

	void createShaders();

	void unloadFromVRAM();

	void unloadFromRAM();

private:
	/* API dependent functions */
	static std::function<void(IERenderable &, IERenderEngine *, const std::string &)> _create;

	void _openglCreate(IERenderEngine *, const std::string &);

	void _vulkanCreate(IERenderEngine *, const std::string &);


	static std::function<void(IERenderable &)> _loadFromDiskToRAM;

	void _openglLoadFromDiskToRAM();

	void _vulkanLoadFromDiskToRAM();


	static std::function<void(IERenderable &)> _loadFromRAMToVRAM;

	void _openglLoadFromRAMToVRAM();

	void _vulkanLoadFromRAMToVRAM();


	static std::function<void(IERenderable &)> _createShaders;

	void _openglCreateShaders();

	void _vulkanCreateShaders();


	static std::function<void(IERenderable &, const IECamera &, float)> _update;

	bool _openglUpdate(const IECamera &, float);

	void _vulkanUpdate(const IECamera &camera, float time);


	static std::function<void(IERenderable &)> _unloadFromVRAM;

	void _openglUnloadFromVRAM();

	void _vulkanUnloadFromVRAM();


	static std::function<void(IERenderable &)> _unloadFromRAM;

	void _openglUnloadFromRAM();

	void _vulkanUnloadFromRAM();
};