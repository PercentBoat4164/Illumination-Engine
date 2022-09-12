#pragma once

/* Predefine classes used with pointers or as return values for functions. */
struct aiScene;

class IECamera;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "Buffer/IEBuffer.hpp"
#include "Image/Texture.hpp"
#include "Shader/IEDescriptorSet.hpp"
#include "Renderable/IEMaterial.hpp"
#include "Shader/IEPipeline.hpp"
#include "Shader/IEShader.hpp"
#include "Shader/IEUniformBufferObject.hpp"
#include "Renderable/IEVertex.hpp"

// Modular dependencies
#include "Core/AssetModule/IEAspect.hpp"
#include "IEMesh.hpp"

// External dependencies
#include <assimp/Importer.hpp>
#include <assimp/BaseImporter.h>

#include <vulkan/vulkan.h>

// System dependencies
#include <string>
#include <functional>


enum IERenderableStatus {
	IE_RENDERABLE_STATE_UNKNOWN = 0x0,
	IE_RENDERABLE_STATE_UNLOADED = 0x1,
	IE_RENDERABLE_STATE_IN_RAM = 0x2,
	IE_RENDERABLE_STATE_IN_VRAM = 0x4
};

class IERenderable : public IEAspect {
public:
	std::string modelName{};
	std::vector<IEMesh> meshes{};
	IEBuffer modelBuffer{};
	IERenderEngine *linkedRenderEngine{};
	IEUniformBufferObject uniformBufferObject{};
	std::vector<IEShader> shaders{};
	Assimp::Importer importer{};
	bool render{true};
	uint32_t commandBufferIndex{};
	std::string directory{};
	glm::mat4 modelMatrix{};
	IERenderableStatus status{IE_RENDERABLE_STATE_UNKNOWN};

	IERenderable() = default;

	IERenderable(IERenderEngine *, const std::string &);

	static void setAPI(const IEAPI &);

	/* API dependent functions */
	static std::function<void(IERenderable &, IERenderEngine *, const std::string &)> _create;

	void create(IERenderEngine *, const std::string &);

	void _openglCreate(IERenderEngine *, const std::string &);

	void _vulkanCreate(IERenderEngine *, const std::string &);


	static std::function<void(IERenderable &)> _loadFromDiskToRAM;

	void loadFromDiskToRAM();

	void _openglLoadFromDiskToRAM();

	void _vulkanLoadFromDiskToRAM();


	static std::function<void(IERenderable &)> _loadFromRAMToVRAM;

	void loadFromRAMToVRAM();

	void _openglLoadFromRAMToVRAM();

	void _vulkanLoadFromRAMToVRAM();


	static std::function<void(IERenderable &, const IECamera &, float, uint32_t)> _update;

	void update(uint32_t);

	void _openglUpdate(const IECamera &camera, float time, uint32_t renderCommandBufferIndex);

	void _vulkanUpdate(const IECamera &camera, float time, uint32_t renderCommandBufferIndex);


	static std::function<void(IERenderable &)> _unloadFromVRAM;

	void unloadFromVRAM();

	void _openglUnloadFromVRAM();

	void _vulkanUnloadFromVRAM();


	static std::function<void(IERenderable &)> _unloadFromRAM;

	void unloadFromRAM();

	void _openglUnloadFromRAM();

	void _vulkanUnloadFromRAM();
};