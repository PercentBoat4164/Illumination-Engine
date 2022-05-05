#include "IEMesh.hpp"
#include "Buffer/IEBuffer.hpp"
#include "Core/LogModule/IELogger.hpp"
#include "IERenderEngine.hpp"

void IEMesh::_vulkanImport(const std::string &directory, const aiScene *scene, aiMesh *mesh) {
    // record indices
    vertices.reserve(mesh->mNumVertices);
    IEVertex temporaryVertex{};
    for (int i = 0; i < mesh->mNumVertices; ++i) {
        if (mesh->HasPositions()) {
            temporaryVertex.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        }
        if (mesh->HasNormals()) {
            temporaryVertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        }
        if (mesh->HasTextureCoords(0)) {
            temporaryVertex.textureCoordinates = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
        }
        if (mesh->HasVertexColors(0)) {
            temporaryVertex.color = {mesh->mColors[0][i].a, mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b};
        }
        if (mesh->HasTangentsAndBitangents()) {
            temporaryVertex.tangent = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
            temporaryVertex.biTangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
        }
        vertices.push_back(temporaryVertex);
    }

    // record vertices
    indices.reserve(3UL * mesh->mNumFaces);
    int j;
    for (int i = 0; i < mesh->mNumFaces; ++i) {
        if (mesh->mFaces[i].mNumIndices != 3) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Detected non-triangular face! Try using the aiProcess_Triangulate flag.");
        }
        for (j = 0; j < mesh->mFaces[i].mNumIndices; ++j) {
            indices.push_back(mesh->mFaces[i].mIndices[j]);
        }
    }

    // assuming all faces are triangles
    triangleCount = mesh->mNumFaces;
    material.index = mesh->mMaterialIndex;

    // texture types to load
    std::vector<std::pair<uint32_t *, aiTextureType>> textureTypes {
            {&material.diffuseTextureIndex, aiTextureType_BASE_COLOR},
    };

    // find all textures in scene including embedded textures
    /**@todo Build a system that tracks duplicate textures and only keeps one copy in memory.*/
    for (std::pair<uint32_t *, aiTextureType> textureType : textureTypes) {
        material.textureCount += scene->mMaterials[material.index]->GetTextureCount(textureType.second);
    }
    linkedRenderEngine->textures.resize(linkedRenderEngine->textures.size() + material.textureCount);

    j = 0;
    aiString texturePath{};
    std::string data{};
    // load all textures despite embedded state
    for (int i = 0; i < material.textureCount; ++i) {
        while (texturePath.length == 0) {
            scene->mMaterials[material.index]->GetTexture(textureTypes[j++].second, i, &texturePath);
        }
        const aiTexture *embeddedTexture = scene->GetEmbeddedTexture(texturePath.C_Str());
        if (embeddedTexture != nullptr && embeddedTexture->mHeight == 0) {
            data = (char *) stbi_load_from_memory((unsigned char *) embeddedTexture->pcData, (int) embeddedTexture->mWidth, nullptr, nullptr, nullptr, 4);
        } else {
            data = (char *) stbi_load((directory + "/" + texturePath.C_Str()).c_str(), nullptr, nullptr, nullptr, 4);
        }
    }
}

void IEMesh::_openglImport(const std::string &directory, const aiScene *scene, aiMesh *mesh) {

}

void IEMesh::_vulkanCreateVertexBuffer() {
    IEBuffer::CreateInfo vertexBufferCreateInfo{
            .size=sizeof(vertices[0]) * vertices.size(),
            .usage=VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU,
            .data=vertices.data(),
            .sizeOfData=static_cast<uint32_t>(vertices.size() * sizeof(vertices[0])),
    };
    vertexBuffer.create(linkedRenderEngine, &vertexBufferCreateInfo);
    deletionQueue.emplace_back([&] { vertexBuffer.destroy(); });
}

void IEMesh::_vulkanCreateIndexBuffer() {
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

std::function<void(IEMesh &)> IEMesh::_create = std::function<void(IEMesh &)>{ [] (const IEMesh&) { return; } };
std::function<void(IEMesh &, const std::string &, const aiScene *, aiMesh *)> IEMesh::_import = std::function<void(IEMesh &, const std::string &, const aiScene *, aiMesh *)>{ [] (const IEMesh&, const std::string &, const aiScene *, aiMesh *) { return; } };
std::function<void(IEMesh &)> IEMesh::_createIndexBuffer = std::function<void(IEMesh &)>{ [] (const IEMesh&) { return; } };
std::function<void(IEMesh &)> IEMesh::_createVertexBuffer = std::function<void(IEMesh &)>{ [] (const IEMesh&) { return; } };


