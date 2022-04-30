/* Include this file's header. */
#include "IERenderable.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"

/* Include external dependencies. */
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/GltfMaterial.h"

#include "stb_image.h"

#include "glm/detail/type_quat.hpp"
#include "glm/glm.hpp"

IERenderable::IERenderable(IERenderEngine *engineLink, const std::string &filePath) {
    linkedRenderEngine = engineLink;
    childType = IE_CHILD_TYPE_RENDERABLE;
    modelName = filePath;
    directory = filePath.substr(0, filePath.find_last_of('/'));
    Assimp::Importer importer{};
    const aiScene *scene = importer.ReadFile(modelName, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_OptimizeMeshes | aiProcess_GenUVCoords | aiProcess_GenNormals | aiProcess_RemoveRedundantMaterials);
    if ((scene == nullptr) || ((scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) != 0U) || (scene->mRootNode == nullptr)) {
        throw std::runtime_error("failed to prepare scene from file: " + std::string(filePath));
    }
    processMesh(*scene->mMeshes[0], *scene);
    linkedRenderEngine->graphicsCommandPool[0].execute();
}

void IERenderable::destroy() {
    if (associatedAssets.empty()) {
        for (const std::function<void()> &function: deletionQueue) {
            function();
        }
        deletionQueue.clear();
    }
}

void IERenderable::createShaders() {
    std::vector<std::string> shaderFileNames = {"shaders/Rasterize/vertexShader.vert.spv", "shaders/Rasterize/fragmentShader.frag.spv"};
    shaders.resize(shaderFileNames.size());
    for (int i = 0; i < shaders.size(); ++i) {
        shaders[i].create(linkedRenderEngine, new IEFile(shaderFileNames[i]));
    }
}

void IERenderable::createModelBuffer() {
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

void IERenderable::createVertexBuffer() {
    IEBuffer::CreateInfo vertexBufferCreateInfo{
            .size=sizeof(vertices[0]) * vertices.size(),
            .usage=VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU,
            .data=vertices.data(),
            .sizeOfData=static_cast<uint32_t>(vertices.size() * sizeof(vertices[0])),
    };
    vertexBuffer.create(linkedRenderEngine, &vertexBufferCreateInfo);
    deletionQueue.emplace_back([&] {
        vertexBuffer.destroy();
    });
}

IERenderable::~IERenderable() {
    destroy();
}

void IERenderable::update(IEAsset *const asset, const IECamera &camera, float time) {
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
    if (linkedRenderEngine->settings->rayTracing) {
        transformationMatrix = {
                modelMatrix[0][0], modelMatrix[0][1], modelMatrix[0][2], modelMatrix[3][0],
                modelMatrix[1][0], modelMatrix[1][1], modelMatrix[1][2], modelMatrix[3][1],
                modelMatrix[2][0], modelMatrix[2][1], modelMatrix[2][2], modelMatrix[3][2]
        };
        transformationBuffer.uploadData(&transformationMatrix, sizeof(transformationMatrix));
        bottomLevelAccelerationStructure.destroy();
        IEAccelerationStructure::CreateInfo renderableBottomLevelAccelerationStructureCreateInfo{};
        renderableBottomLevelAccelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        renderableBottomLevelAccelerationStructureCreateInfo.transformationMatrix = &identityTransformMatrix;
        renderableBottomLevelAccelerationStructureCreateInfo.primitiveCount = triangleCount;
        renderableBottomLevelAccelerationStructureCreateInfo.vertexBufferAddress = vertexBuffer.deviceAddress;
        renderableBottomLevelAccelerationStructureCreateInfo.indexBufferAddress = indexBuffer.deviceAddress;
        renderableBottomLevelAccelerationStructureCreateInfo.transformationBufferAddress = transformationBuffer.deviceAddress;
        bottomLevelAccelerationStructure.create(linkedRenderEngine, &renderableBottomLevelAccelerationStructureCreateInfo);
    }
}

void IERenderable::createIndexBuffer() {
    IEBuffer::CreateInfo indexBufferCreateInfo{
            .size=sizeof(indices[0]) * indices.size(),
            .usage=VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            .allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU,
            .data=indices.data(),
            .sizeOfData=static_cast<uint32_t>(indices.size() * sizeof(indices[0])),
    };
    indexBuffer.create(linkedRenderEngine, &indexBufferCreateInfo);
    deletionQueue.emplace_back([&] {
        indexBuffer.destroy();
    });
}

void IERenderable::createDescriptorSet() {
    IEDescriptorSet::CreateInfo descriptorSetCreateInfo{
            .poolSizes={{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}, {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}},
            .shaderStages={static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT), VK_SHADER_STAGE_FRAGMENT_BIT},
            .data={&modelBuffer, &textures[diffuseTexture]}
    };
    descriptorSet.create(linkedRenderEngine, &descriptorSetCreateInfo);
    deletionQueue.emplace_back([&] {
        descriptorSet.destroy();
    });
}

void IERenderable::createPipeline() {
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

void IERenderable::processMesh(const aiMesh &mesh, const aiScene &scene) {
    // process mesh
    vertices.reserve(mesh.mNumVertices);
    for (int i = 0; i < mesh.mNumVertices; ++i) {
        IEVertex temporaryVertex{};
        if (mesh.HasPositions()) {
            temporaryVertex.position = {mesh.mVertices[i].x, mesh.mVertices[i].y, mesh.mVertices[i].z};
        }
        if (mesh.HasNormals()) {
            temporaryVertex.normal = {mesh.mNormals[i].x, mesh.mNormals[i].y, mesh.mNormals[i].z};
        }
        if (mesh.HasTextureCoords(0)) {
            temporaryVertex.textureCoordinates = {mesh.mTextureCoords[0][i].x, mesh.mTextureCoords[0][i].y};
        }
        if (mesh.HasVertexColors(0)) {
            temporaryVertex.color = {mesh.mColors[0][i].a, mesh.mColors[0][i].r, mesh.mColors[0][i].g, mesh.mColors[0][i].b};
        }
        if (mesh.HasTangentsAndBitangents()) {
            temporaryVertex.tangent = {mesh.mTangents[i].x, mesh.mTangents[i].y, mesh.mTangents[i].z};
            temporaryVertex.biTangent = {mesh.mBitangents[i].x, mesh.mBitangents[i].y, mesh.mBitangents[i].z};
        }
        vertices.push_back(temporaryVertex);
    }
    indices.reserve(3UL * mesh.mNumFaces);
    for (int i = 0; i < mesh.mNumFaces; ++i) {
        if (mesh.mFaces[i].mNumIndices != 3) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Detected non-triangular face! Try using the aiProcess_Triangulate flag.");
        } else {
            for (int j = 0; j < mesh.mFaces[i].mNumIndices; ++j) {
                indices.push_back(mesh.mFaces[i].mIndices[j]);
            }
        }
    }
    triangleCount = mesh.mNumFaces;
    std::vector<std::pair<uint32_t *, aiTextureType>> textureTypes{
            {&diffuseTexture, aiTextureType_DIFFUSE},
            {&emissionTexture, aiTextureType_EMISSIVE},
            {&heightTexture, aiTextureType_HEIGHT},
            {&metallicTexture, aiTextureType_METALNESS},
            {&normalTexture, aiTextureType_NORMALS},
            {&roughnessTexture, aiTextureType_DIFFUSE_ROUGHNESS},
            {&specularTexture, aiTextureType_SPECULAR}
    };
    IETexture::CreateInfo textureCreateInfo{};
    textureCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    textureCreateInfo.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    textureCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    textureCreateInfo.allocationUsage = VMA_MEMORY_USAGE_GPU_ONLY;
    for (std::pair<uint32_t *, aiTextureType> textureType : textureTypes) {
        unsigned int numTextures = scene.mMaterials[mesh.mMaterialIndex]->GetTextureCount(textureType.second);
        for (int i = 0; numTextures > 0; ++i) {
            aiString filename;
            scene.mMaterials[mesh.mMaterialIndex]->GetTexture(textureType.second, i, &filename);
            if (filename.length == 0) {
                continue;
            }
            textures.resize(textures.size() + 1);
            --numTextures;
            const aiTexture *embeddedTexture = scene.GetEmbeddedTexture(filename.C_Str());
            if (embeddedTexture != nullptr && embeddedTexture->mHeight == 0) {
                textureCreateInfo.filename = std::string(embeddedTexture->mFilename.C_Str()) + "." + embeddedTexture->achFormatHint;
                textureCreateInfo.data = (unsigned char *)embeddedTexture->pcData;
                int channels;
                textureCreateInfo.data = stbi_load_from_memory(textureCreateInfo.data, (int)embeddedTexture->mWidth, reinterpret_cast<int *>(&textureCreateInfo.width), reinterpret_cast<int *>(&textureCreateInfo.height), &channels, 4);
            } else {
                int channels;
                textureCreateInfo.filename = directory + "/" + filename.C_Str();
                textureCreateInfo.data = stbi_load(textureCreateInfo.filename.c_str(), reinterpret_cast<int *>(&textureCreateInfo.width), reinterpret_cast<int *>(&textureCreateInfo.height), &channels, 4);
            }
            textures[i].create(linkedRenderEngine, &textureCreateInfo);
            stbi_image_free(textureCreateInfo.data);
        }
    }
}
