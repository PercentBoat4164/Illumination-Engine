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
    linkedRenderEngine = engineLink;
    directory = filePath.substr(0, filePath.find_last_of('/'));
    modelName = filePath.substr(filePath.find_last_of('/'));
    create();
}

void IERenderable::setAPI(const IEAPI &api) {
    if (api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
        _create = &IERenderable::_openglCreate;
        _loadFromDiskToRAM = &IERenderable::_openglLoadFromDiskToRAM;
        _load = &IERenderable::_openglLoad;
        _createModelBuffers = &IERenderable::_openglCreateModelBuffer;
        _createDescriptorSet = &IERenderable::_openglCreateDescriptorSet;
        _createPipeline = &IERenderable::_openglCreatePipeline;
        _createShaders = &IERenderable::_openglCreateShaders;
        _update = &IERenderable::_openglUpdate;
        _unload = &IERenderable::_openglUnload;
        _destroy = &IERenderable::_openglDestroy;
    } else if (api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        _create = &IERenderable::_vulkanCreate;
        _loadFromDiskToRAM = &IERenderable::_vulkanLoadFromDiskToRAM;
        _load = &IERenderable::_vulkanLoad;
        _createModelBuffers = &IERenderable::_vulkanCreateModelBuffer;
        _createDescriptorSet = &IERenderable::_vulkanCreateDescriptorSet;
        _createPipeline = &IERenderable::_vulkanCreatePipeline;
        _createShaders = &IERenderable::_vulkanCreateShaders;
        _update = &IERenderable::_vulkanUpdate;
        _unload = &IERenderable::_vulkanUnload;
        _destroy = &IERenderable::_vulkanDestroy;
    }
}


void IERenderable::create() {
    return _create(*this);
}

void IERenderable::_openglCreate() {
    // Do nothing
}

void IERenderable::_vulkanCreate() {
    // Prepare a command buffer for use by this object during creation
    commandBufferIndex = linkedRenderEngine->graphicsCommandPool.commandBuffers.size();
    linkedRenderEngine->graphicsCommandPool[commandBufferIndex];
}


void IERenderable::loadFromDiskToRAM() {
    return _loadFromDiskToRAM(*this);
}

void IERenderable::_openglLoadFromDiskToRAM() {}

void IERenderable::_vulkanLoadFromDiskToRAM() {
    // Read input file
    const aiScene *scene = importer.ReadFile(directory + modelName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_OptimizeMeshes | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_RemoveRedundantMaterials);
    if ((scene == nullptr) || ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U) || (scene->mRootNode == nullptr)) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "failed to prepare scene from file: " + std::string(directory + modelName));
    }

    meshes.resize(scene->mNumMeshes);
    uint32_t meshIndex = 0;

    // import all meshes
    for (IEMesh &mesh : meshes) {
        mesh.import(directory, scene, scene->mMeshes[meshIndex]);
    }
}


void IERenderable::load() {
    return _load(*this);
}

void IERenderable::_openglLoad() {}

void IERenderable::_vulkanLoad() {
    IETexture::CreateInfo textureCreateInfo{};
    textureCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    textureCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    textureCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureCreateInfo.allocationUsage = VMA_MEMORY_USAGE_GPU_ONLY;
}


void IERenderable::createModelBuffer() {
    return _createModelBuffers(*this);
}

void IERenderable::_openglCreateModelBuffer() {}

void IERenderable::_vulkanCreateModelBuffer() {
    IEBuffer::CreateInfo modelBufferCreateInfo{
            .size=sizeof(IEUniformBufferObject),
            .usage=VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            .allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU
    };
    modelBuffer.create(linkedRenderEngine, &modelBufferCreateInfo);
    deletionQueue.emplace_back([&] {
        modelBuffer.destroy();
    });
}


void IERenderable::createDescriptorSet() {
    return _createDescriptorSet(*this);
}

void IERenderable::_openglCreateDescriptorSet() {}

void IERenderable::_vulkanCreateDescriptorSet() {
    IEDescriptorSet::CreateInfo descriptorSetCreateInfo{
            .poolSizes={{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}},
            .shaderStages={static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT), VK_SHADER_STAGE_FRAGMENT_BIT},
//            .data={&modelBuffer, &linkedRenderEngine->textures[diffuseTexture]}
    };
    descriptorSet.create(linkedRenderEngine, &descriptorSetCreateInfo);
    deletionQueue.emplace_back([&] {
        descriptorSet.destroy();
    });
}


void IERenderable::createPipeline() {
    return _createPipeline(*this);
}

void IERenderable::_openglCreatePipeline() {}

void IERenderable::_vulkanCreatePipeline() {
    IEPipeline::CreateInfo pipelineCreateInfo{
            .shaders=&shaders,
            .descriptorSet=&descriptorSet,
            .renderPass=&linkedRenderEngine->renderPass
    };
    pipeline.create(linkedRenderEngine, &pipelineCreateInfo);
    deletionQueue.emplace_back([&] {
        pipeline.destroy();
    });
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

void IERenderable::update() {}

void IERenderable::update(IEAsset *asset, const IECamera &camera, float time) {
    return _update(*this, asset, camera, time);
}

void IERenderable::_vulkanUpdate(IEAsset *asset, const IECamera &camera, float time) {
    glm::mat4 modelMatrix = glm::translate(glm::scale(glm::identity<glm::mat4>(), asset->scale), asset->position);
    glm::quat(asset->rotation);
    modelMatrix = glm::rotate(modelMatrix, asset->rotation.y, glm::vec3(-1.0F, 0.0F, 0.0F));
    modelMatrix = glm::rotate(modelMatrix, asset->rotation.x, glm::vec3(0.0F, 1.0F, 0.0F));
    modelMatrix = glm::rotate(modelMatrix, asset->rotation.z, glm::vec3(0.0F, 0.0F, 1.0F));
    uniformBufferObject.viewModelMatrix = camera.viewMatrix;
    uniformBufferObject.modelMatrix = modelMatrix;
    uniformBufferObject.projectionMatrix = camera.projectionMatrix;
    uniformBufferObject.normalMatrix = glm::mat4(glm::transpose(glm::inverse(modelMatrix)));
    uniformBufferObject.position = camera.position;
    uniformBufferObject.time = time;
    modelBuffer.uploadData(&uniformBufferObject, sizeof(uniformBufferObject));
//    if (linkedRenderEngine->settings->rayTracing) {
//        transformationMatrix = {
//                modelMatrix[0][0], modelMatrix[0][1], modelMatrix[0][2], modelMatrix[3][0],
//                modelMatrix[1][0], modelMatrix[1][1], modelMatrix[1][2], modelMatrix[3][1],
//                modelMatrix[2][0], modelMatrix[2][1], modelMatrix[2][2], modelMatrix[3][2]
//        };
//        transformationBuffer.uploadData(&transformationMatrix, sizeof(transformationMatrix));
//        bottomLevelAccelerationStructure.destroy();
//        IEAccelerationStructure::CreateInfo renderableBottomLevelAccelerationStructureCreateInfo{};
//        renderableBottomLevelAccelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
//        renderableBottomLevelAccelerationStructureCreateInfo.transformationMatrix = &identityTransformMatrix;
//        renderableBottomLevelAccelerationStructureCreateInfo.primitiveCount = triangleCount;
//        renderableBottomLevelAccelerationStructureCreateInfo.vertexBufferAddress = vertexBuffer.deviceAddress;
//        renderableBottomLevelAccelerationStructureCreateInfo.indexBufferAddress = indexBuffer.deviceAddress;
//        renderableBottomLevelAccelerationStructureCreateInfo.transformationBufferAddress = transformationBuffer.deviceAddress;
//        bottomLevelAccelerationStructure.create(linkedRenderEngine, &renderableBottomLevelAccelerationStructureCreateInfo);
//    }
}

void IERenderable::_openglUpdate(IEAsset *asset, const IECamera &camera, float time) {}


void IERenderable::unload() {
    return _unload(*this);
}

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


std::function<void(IERenderable &)> IERenderable::_create = std::function<void(IERenderable &)>{ [] (const IERenderable&) { return; } };
std::function<void(IERenderable &)> IERenderable::_loadFromDiskToRAM = std::function<void(IERenderable &)>{ [] (const IERenderable&) { return; } };
std::function<void(IERenderable &)> IERenderable::_load = std::function<void(IERenderable &)>{ [] (const IERenderable&) { return; } };
std::function<void(IERenderable &)> IERenderable::_createModelBuffers = std::function<void(IERenderable &)>{ [] (const IERenderable&) { return; } };
std::function<void(IERenderable &)> IERenderable::_createDescriptorSet = std::function<void(IERenderable &)>{ [] (const IERenderable&) { return; } };
std::function<void(IERenderable &)> IERenderable::_createPipeline = std::function<void(IERenderable &)>{ [] (const IERenderable&) { return; } };
std::function<void(IERenderable &)> IERenderable::_createShaders = std::function<void(IERenderable &)>{ [] (const IERenderable&) { return; } };
std::function<void(IERenderable &, IEAsset *, const IECamera &, float)> IERenderable::_update = std::function<void(IERenderable &, IEAsset *, const IECamera &, float)>{ [] (const IERenderable &, IEAsset *, const IECamera &, float) { return; } };
std::function<void(IERenderable &)> IERenderable::_unload = std::function<void(IERenderable &)>{ [] (const IERenderable&) { return; } };
std::function<void(IERenderable &)> IERenderable::_destroy = std::function<void(IERenderable &)>{ [] (const IERenderable&) { return; } };
