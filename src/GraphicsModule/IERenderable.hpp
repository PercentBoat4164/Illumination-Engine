#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class aiNode;

class aiScene;

class IECamera;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "Buffer/IEAccelerationStructure.hpp"
#include "Buffer/IEBuffer.hpp"
#include "IEDescriptorSet.hpp"
#include "IEPipeline.hpp"
#include "IEShader.hpp"
#include "IEUniformBufferObject.hpp"
#include "IEVertex.hpp"

#include "GraphicsModule/Image/IETexture.hpp"

// Modular dependencies
#include "Core/AssetModule/IEAspect.hpp"

// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <string>
#include <functional>

#define IE_CHILD_TYPE_RENDERABLE "Renderable"


class IERenderable : public IEAspect {
public:
    const char *modelName{};
    std::vector<uint32_t> indices{};
    std::vector<IEVertex> vertices{};
    VkTransformMatrixKHR transformationMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    uint32_t diffuseTexture{};
    uint32_t emissionTexture{};
    uint32_t heightTexture{};
    uint32_t metallicTexture{};
    uint32_t normalTexture{};
    uint32_t roughnessTexture{};
    uint32_t specularTexture{};
    uint32_t triangleCount{};
    IEBuffer vertexBuffer{};
    IEBuffer indexBuffer{};
    IEBuffer transformationBuffer{};
    IEDescriptorSet descriptorSet{};
    IEPipeline pipeline{};
    IEAccelerationStructure bottomLevelAccelerationStructure{};

    IERenderable(IERenderEngine* engineLink, const std::string& filePath);

    void destroy();

    void createModelBuffer();

    void createVertexBuffer();

    ~IERenderable();

    void update(const IECamera &camera, float time);

    std::vector<std::function<void()>> deletionQueue{};
    IEBuffer modelBuffer{};
    IERenderEngine *linkedRenderEngine{};
    IEUniformBufferObject uniformBufferObject{};
    std::vector<IETexture>* textures;
    std::vector<IEShader> shaders{};
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    bool render{true};
    bool created{false};
    std::string directory{};
    VkTransformMatrixKHR identityTransformMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};

    void createIndexBuffer();

    /**@todo Rewrite the shader abstraction to generate a required descriptor set. Create pipelines based off of descriptor set / shader pairs. Sort and render geometry by pipeline used.*/

    void createDescriptorSet();

    void createPipeline();

    void createShaders();

private:
    /**@todo Write this better.*/
    void processNode(aiNode *node, const aiScene *scene);

    /**@todo Make this auto-detect shader stage.*/
    void loadShaders(const std::vector<const char *> &filenames);

};