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


class IERenderable : public IEAspect {
public:
	std::string modelName{};
	std::vector<IEMesh> meshes{};
	VkTransformMatrixKHR transformationMatrix{1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F};
	IEBuffer transformationBuffer{};
	std::shared_ptr<IEDescriptorSet> descriptorSet{};
	IEPipeline pipeline{};
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
	VkTransformMatrixKHR identityTransformMatrix{1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F, 0.0F, 0.0F, 0.0F, 1.0F, 0.0F};

	IERenderable(IERenderEngine *, const std::string &);

	static void setAPI(const IEAPI &);

	void create(IERenderEngine *, const std::string &);

	void loadFromDiskToRAM();

	void update(const IECamera &camera, float time);

	void destroy() override;

	~IERenderable() override;

	void loadFromRAMToVRAM();

	void update(uint32_t) final;

private:
	/* API dependent functions */
	void createShaders();

	static std::function<void(IERenderable &, IERenderEngine *, const std::string &)> _create;

	void _openglCreate(IERenderEngine *, const std::string &);

	void _vulkanCreate(IERenderEngine *, const std::string &);


	static std::function<void(IERenderable &)> _loadFromDiskToRAM;

	void _openglLoadFromDiskToRAM();

	void _vulkanLoadFromDiskToRAM();


	static std::function<void(IERenderable &)> _loadFromRAMToVRAM;

	void _openglLoad();

	void _vulkanLoad();


	static std::function<void(IERenderable &)> _createShaders;

	void _openglCreateShaders();

	void _vulkanCreateShaders();


	static std::function<void(IERenderable &, const IECamera &, float)> _update;

	bool _openglUpdate(const IECamera &, float);

	void _vulkanUpdate(const IECamera &camera, float time);


	static std::function<void(IERenderable &)> _unload;

	void _openglUnload();

	void _vulkanUnload();


	static std::function<void(IERenderable &)> _destroy;

	void _openglDestroy();

	void _vulkanDestroy();
};