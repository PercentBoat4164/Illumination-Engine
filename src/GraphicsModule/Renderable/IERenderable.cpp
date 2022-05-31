/* Include this file's header. */
#include "IERenderable.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"
#include "IEMesh.hpp"

/* Include external dependencies. */
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/gtc/quaternion.hpp>

IERenderable::IERenderable(IERenderEngine *engineLink, const std::string &filePath) {
	create(engineLink, filePath);
}

void IERenderable::setAPI(const IEAPI &api) {
	if (api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		_create = &IERenderable::_openglCreate;
		_update = &IERenderable::_openglUpdate;
		_loadFromDiskToRAM = &IERenderable::_openglLoadFromDiskToRAM;
		_loadFromRAMToVRAM = &IERenderable::_openglLoadFromRAMToVRAM;
		_createShaders = &IERenderable::_openglCreateShaders;
	} else if (api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		_create = &IERenderable::_vulkanCreate;
		_update = &::IERenderable::_vulkanUpdate;
		_loadFromDiskToRAM = &IERenderable::_vulkanLoadFromDiskToRAM;
		_loadFromRAMToVRAM = &IERenderable::_vulkanLoadFromRAMToVRAM;
		_createShaders = &IERenderable::_vulkanCreateShaders;
	}
}


std::function<void(IERenderable &, IERenderEngine *, const std::string &)> IERenderable::_create{
		[](const IERenderable &, IERenderEngine *, const std::string &) {
			return;
		}
};

void IERenderable::create(IERenderEngine *engineLink, const std::string &filePath) {
	linkedRenderEngine = engineLink;
	directory = filePath.substr(0, filePath.find_last_of('/'));
	modelName = filePath.substr(filePath.find_last_of('/'));
	return _create(*this, engineLink, filePath);
}

void IERenderable::_openglCreate(IERenderEngine *engineLink, const std::string &filePath) {
	// Do nothing
}

void IERenderable::_vulkanCreate(IERenderEngine *engineLink, const std::string &filePath) {
	linkedRenderEngine = engineLink;
	for (IEMesh &mesh: meshes) {
		mesh.create(linkedRenderEngine);
	}

	IEBuffer::CreateInfo modelBufferCreateInfo{
			.size=sizeof(IEUniformBufferObject),
			.usage=VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			.allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU
	};
	modelBuffer.create(linkedRenderEngine, &modelBufferCreateInfo);
	deletionQueue.emplace_back([&] {
		modelBuffer.destroy();
	});

	// Prepare a command buffer for use by this object during creation
	commandBufferIndex = linkedRenderEngine->graphicsCommandPool->commandBuffers.size();
	linkedRenderEngine->graphicsCommandPool->index(commandBufferIndex);
}


std::function<void(IERenderable &)> IERenderable::_loadFromDiskToRAM{
		[](IERenderable &) {
			return;
		}
};

void IERenderable::loadFromDiskToRAM() {
	_loadFromDiskToRAM(*this);
}

void IERenderable::_openglLoadFromDiskToRAM() {
	// Read input file
	const aiScene *scene = importer.ReadFile(directory + modelName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_OptimizeMeshes |
																	aiProcess_RemoveRedundantMaterials | aiProcess_JoinIdenticalVertices |
																	aiProcess_PreTransformVertices);
	if ((scene == nullptr) || ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U) || (scene->mRootNode == nullptr)) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN,
												 "failed to prepare scene from file: " + std::string(directory + modelName));
	}

	meshes.resize(scene->mNumMeshes);
	uint32_t meshIndex = 0;

	// import all meshes
	for (IEMesh &mesh: meshes) {
		mesh.create(linkedRenderEngine);
		mesh.loadFromDiskToRAM(directory, scene, scene->mMeshes[meshIndex++]);
	}

	modelBuffer.loadFromDiskToRAM(std::vector<char>{sizeof(glm::mat4)});
}

void IERenderable::_vulkanLoadFromDiskToRAM() {
	// Read input file
	const aiScene *scene = importer.ReadFile(directory + modelName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_OptimizeMeshes |
																	aiProcess_RemoveRedundantMaterials | aiProcess_JoinIdenticalVertices |
																	aiProcess_PreTransformVertices);
	if ((scene == nullptr) || ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U) || (scene->mRootNode == nullptr)) {
		linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN,
												 "failed to prepare scene from file: " + std::string(directory + modelName));
	}

	meshes.resize(scene->mNumMeshes);
	uint32_t meshIndex = 0;

	// import all meshes
	for (IEMesh &mesh: meshes) {
		mesh.create(linkedRenderEngine);
		mesh.loadFromDiskToRAM(directory, scene, scene->mMeshes[meshIndex++]);
	}

	modelBuffer.loadFromDiskToRAM(std::vector<char>{sizeof(glm::mat4)});
}


std::function<void(IERenderable &)> IERenderable::_loadFromRAMToVRAM{
		[](IERenderable &) {
			return;
		}
};

void IERenderable::loadFromRAMToVRAM() {
	_loadFromRAMToVRAM(*this);
}

void IERenderable::_openglLoadFromRAMToVRAM() {
	for (IEMesh &mesh: meshes) {
		mesh.loadFromRAMToVRAM();
	}
	modelBuffer.loadFromRAMToVRAM();
}

void IERenderable::_vulkanLoadFromRAMToVRAM() {
	for (IEMesh &mesh: meshes) {
		mesh.loadFromRAMToVRAM();
	}
	modelBuffer.loadFromRAMToVRAM();
}


std::function<void(IERenderable &)> IERenderable::_createShaders{
		[](const IERenderable &) {
			return;
		}
};

void IERenderable::createShaders() {
	return _createShaders(*this);
}

void IERenderable::_openglCreateShaders() {
	return;
}

void IERenderable::_vulkanCreateShaders() {
	std::vector<std::string> shaderFileNames = {"shaders/Rasterize/vertexShader.vert.spv", "shaders/Rasterize/fragmentShader.frag.spv"};
	shaders.resize(shaderFileNames.size());
	for (int i = 0; i < shaders.size(); ++i) {
		shaders[i].create(linkedRenderEngine, new IEFile(shaderFileNames[i]));
	}
}


std::function<void(IERenderable &, const IECamera &, float)> IERenderable::_update{
		[](const IERenderable &, const IECamera &, float) {
			return;
		}
};

void IERenderable::update(uint32_t renderCommandBufferIndex) {
	for (IEMesh &mesh: meshes) {
		mesh.descriptorSet->update({&modelBuffer}, {0});
		mesh.update(renderCommandBufferIndex);
	}
	_update(*this, linkedRenderEngine->camera, (float) glfwGetTime());
}

void IERenderable::_vulkanUpdate(const IECamera &camera, float time) {
	modelMatrices.resize(associatedAssets.size());
	for (int i = 0; i < associatedAssets.size(); ++i) {
		glm::quat quaternion = glm::yawPitchRoll(associatedAssets[i].lock()->rotation.x, associatedAssets[i].lock()->rotation.y, associatedAssets[i].lock()->rotation.z);
		modelMatrices[i] = glm::rotate(glm::translate(glm::scale(glm::identity<glm::mat4>(), associatedAssets[i].lock()->scale), associatedAssets[i].lock()->position), glm::angle(quaternion), glm::axis(quaternion));
	}
	uniformBufferObject.viewModelMatrix = camera.viewMatrix;
	uniformBufferObject.modelMatrix = modelMatrices[0];
	uniformBufferObject.projectionMatrix = camera.projectionMatrix;
	uniformBufferObject.normalMatrix = glm::mat4(glm::transpose(glm::inverse(modelMatrices[0])));
	uniformBufferObject.position = camera.position;
	uniformBufferObject.time = time;
	modelBuffer.loadFromDiskToRAM(&uniformBufferObject, sizeof(uniformBufferObject));
	modelBuffer.loadFromRAMToVRAM();
}

bool IERenderable::_openglUpdate(const IECamera &camera, float time) {
	return true;
}


std::function<void(IERenderable &)> IERenderable::_unloadFromVRAM{
		[](IERenderable &) {
			return;
		}
};

void IERenderable::unloadFromVRAM() {
	return _unloadFromVRAM(*this);
}

void IERenderable::_openglUnloadFromVRAM() {
	for (IEMesh &mesh: meshes) {
		mesh.unloadFromVRAM();
	}
	modelBuffer.unloadFromVRAM();
}

void IERenderable::_vulkanUnloadFromVRAM() {
	for (IEMesh &mesh: meshes) {
		mesh.unloadFromVRAM();
	}
	modelBuffer.unloadFromVRAM();
}


std::function<void(IERenderable &)> IERenderable::_unloadFromRAM{
		[](IERenderable &) {
			return;
		}
};

void IERenderable::unloadFromRAM() {
	return _unloadFromRAM(*this);
}

void IERenderable::_openglUnloadFromRAM() {
	for (IEMesh &mesh: meshes) {
		mesh.unloadFromRAM();
	}
}

void IERenderable::_vulkanUnloadFromRAM() {
	for (IEMesh &mesh: meshes) {
		mesh.unloadFromRAM();
	}
}
