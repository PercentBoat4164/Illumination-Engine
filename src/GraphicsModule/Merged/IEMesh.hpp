#pragma once

#include "IEBuffer.hpp"
#include "IERenderEngineLink.hpp"
#include "IEVertex.hpp"
#include "IEPipeline.hpp"

#include "assimp/postprocess.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"

#include <vector>

class IEMesh {
public:
    std::vector<uint32_t> indices{};
    std::vector<IEVertex> vertices{};
    uint32_t diffuseTextureIndex{};
    uint32_t emissionTextureIndex{};
    uint32_t heightTextureIndex{};
    uint32_t metallicTextureIndex{};
    uint32_t normalTextureIndex{};
    uint32_t roughnessTextureIndex{};
    uint32_t specularTextureIndex{};
    uint32_t triangleCount{};
    IEBuffer vertexBuffer{};
    IEBuffer indexBuffer{};
    IERenderEngineLink* linkedRenderEngine{};
    IEPipeline pipeline{};

    void destroy() {
        vertexBuffer.destroy();
        indexBuffer.destroy();
        pipeline.destroy();
    }

    IEMesh load(const std::string& file) {
        Assimp::Importer importer{};
        const aiScene *scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_GenUVCoords | aiProcess_CalcTangentSpace | aiProcess_GenNormals);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to generate scene from file: " + file + ".");
        }
        std::string directory = std::string(file).substr(0, std::string(file).find_last_of('/'));
        return *this;
    }
};