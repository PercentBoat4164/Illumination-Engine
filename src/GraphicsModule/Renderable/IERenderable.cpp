/* Include this file's header. */
#include "IERenderable.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"
#include "IEMesh.hpp"

/* Include external dependencies. */
#include "assimp/postprocess.h"
#include "assimp/scene.h"

#include "stb_image.h"

#include "glm/detail/type_quat.hpp"
#include "glm/glm.hpp"

IERenderable::IERenderable(IERenderEngine *engineLink, const std::string &filePath) {
	create(engineLink, filePath);
}

void IERenderable::setAPI(const IEAPI &api) {
	if (api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		_create = &IERenderable::_openglCreate;
		_update = &IERenderable::_openglUpdate;
		_destroy = &IERenderable::_openglDestroy;
	} else if (api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		_create = &IERenderable::_vulkanCreate;
		_update = &IERenderable::_vulkanUpdate;
		_destroy = &IERenderable::_vulkanDestroy;
	}
}


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

	IEBuffer::CreateInfo modelBufferCreateInfo{.size=sizeof(IEUniformBufferObject), .usage=VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, .allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU};
	modelBuffer.create(linkedRenderEngine, &modelBufferCreateInfo);
	deletionQueue.emplace_back([&] { modelBuffer.destroy(); });

	// Prepare a command buffer for use by this object during creation
	commandBufferIndex = linkedRenderEngine->graphicsCommandPool->commandBuffers.size();
	linkedRenderEngine->graphicsCommandPool->index(commandBufferIndex);
}


void IERenderable::loadFromDiskToRAM() {
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

void IERenderable::loadFromRAMToVRAM() {
	for (IEMesh &mesh: meshes) {
		mesh.loadFromRAMToVRAM();
	}
	modelBuffer.loadFromRAMToVRAM();
}

void IERenderable::_openglLoad() {}

void IERenderable::_vulkanLoad() {
	IETexture::CreateInfo textureCreateInfo{};
	textureCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	textureCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	textureCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	textureCreateInfo.allocationUsage = VMA_MEMORY_USAGE_GPU_ONLY;
}


void IERenderable::createShaders() {
	return _createShaders(*this);
}

void IERenderable::_openglCreateShaders() {}

void IERenderable::_vulkanCreateShaders() {
	std::vector<std::string> shaderFileNames = {"shaders/Rasterize/vertexShader.vert.spv", "shaders/Rasterize/fragmentShader.frag.spv"};
	shaders.resize(shaderFileNames.size());
	for (int i = 0; i < shaders.size(); ++i) {
		shaders[i].create(linkedRenderEngine, new IEFile(shaderFileNames[i]));
	}
}

void IERenderable::update(uint32_t renderCommandBufferIndex) {
	for (IEMesh &mesh: meshes) {
		mesh.descriptorSet->update({&modelBuffer}, {0});
		mesh.update(renderCommandBufferIndex);
	}
	_update(*this, linkedRenderEngine->camera, (float) glfwGetTime());
}

void IERenderable::update(const IECamera &camera, float time) {
	return _update(*this, camera, time);
}

void IERenderable::_vulkanUpdate(const IECamera &camera, float time) {
	modelMatrices.resize(associatedAssets.size());
	for (int i = 0; i < associatedAssets.size(); ++i) {
		modelMatrices[i] = glm::translate(glm::scale(glm::identity<glm::mat4>(), associatedAssets[i].lock()->scale), associatedAssets[i].lock()->position);
		glm::quat(associatedAssets[i].lock()->rotation);
		modelMatrices[i] = glm::rotate(modelMatrices[i], associatedAssets[i].lock()->rotation.y, glm::vec3(-1.0F, 0.0F, 0.0F));
		modelMatrices[i] = glm::rotate(modelMatrices[i], associatedAssets[i].lock()->rotation.x, glm::vec3(0.0F, 1.0F, 0.0F));
		modelMatrices[i] = glm::rotate(modelMatrices[i], associatedAssets[i].lock()->rotation.z, glm::vec3(0.0F, 0.0F, 1.0F));
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

bool IERenderable::_openglUpdate(const IECamera &camera, float time) {}

void IERenderable::_openglUnload() {}

void IERenderable::_vulkanUnload() {}


void IERenderable::destroy() {
	return _destroy(*this);
}

void IERenderable::_vulkanDestroy() {
	if (associatedAssets.empty()) {
		for (const std::function<void()> &function: deletionQueue) {
			function();
		}
		deletionQueue.clear();
	}
}

void IERenderable::_openglDestroy() {}


IERenderable::~IERenderable() {
	destroy();
}


std::function<void(IERenderable &, IERenderEngine *, const std::string &)> IERenderable::_create = std::function<void(IERenderable &,
																													  IERenderEngine *,
																													  const std::string &)>{
		[](const IERenderable &, IERenderEngine *, const std::string &) { return; }};
std::function<void(IERenderable &)> IERenderable::_createShaders = std::function<void(IERenderable &)>{[](const IERenderable &) { return; }};
std::function<void(IERenderable &, const IECamera &, float)> IERenderable::_update = std::function<void(IERenderable &, const IECamera &, float)>{
		[] (const IERenderable &, const IECamera &, float) { return; }};
std::function<void(IERenderable &)> IERenderable::_destroy = std::function<void(IERenderable &)>{[](const IERenderable &) { return; }};

void IERenderable::_openglLoadFromDiskToRAM() {

}

void IERenderable::_vulkanLoadFromDiskToRAM() {

}
