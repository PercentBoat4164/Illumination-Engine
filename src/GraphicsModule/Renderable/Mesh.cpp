#include "Mesh.hpp"

#include "assimp/mesh.h"
#include "RenderEngine.hpp"

void IE::Graphics::Mesh::load(const aiScene *t_scene) {
    aiMesh *mesh = t_scene->mMeshes[0];

    // record indices
    m_vertices.reserve(mesh->mNumVertices);
    Vertex temporaryVertex{};
    for (size_t i = 0; i < mesh->mNumVertices; ++i) {
        if (mesh->HasPositions())
            temporaryVertex.position = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
        if (mesh->HasNormals())
            temporaryVertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
        if (mesh->HasTextureCoords(0))
            temporaryVertex.textureCoordinates = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
        if (mesh->HasVertexColors(0))
            temporaryVertex.color =
              {mesh->mColors[0][i].a, mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b};
        if (mesh->HasTangentsAndBitangents()) {
            temporaryVertex.tangent   = {mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z};
            temporaryVertex.biTangent = {mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z};
        }
        m_vertices.push_back(temporaryVertex);
    }

    // Create vertex buffer.
    m_vertexBuffer->createBuffer(
      Buffer::IE_BUFFER_TYPE_VERTEX_BUFFER,
      0x0,
      m_vertices.data(),
      sizeof(decltype(m_vertices)::value_type) * m_vertices.size()
    );

    // assuming all faces are triangles
    m_triangleCount = mesh->mNumFaces;

    // record vertices
    m_indices.reserve(3UL * m_triangleCount);
    for (size_t i = 0; i < m_triangleCount; ++i) {
        if (mesh->mFaces[i].mNumIndices != 3)
            m_linkedRenderEngine->getLogger().log(
              "Attempted to add a non-triangular face to a mesh! Try using the aiProcess_Triangulate flag.",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
            );
        for (size_t j{0}; j < mesh->mFaces[i].mNumIndices; ++j) m_indices.push_back(mesh->mFaces[i].mIndices[j]);
    }
    m_indices.shrink_to_fit();

    // Create index buffer
    m_indexBuffer->createBuffer(
      Buffer::IE_BUFFER_TYPE_INDEX_BUFFER,
      0x0,
      m_indices.data(),
      sizeof(decltype(m_indices)::value_type)
    );
}

IE::Graphics::Mesh::Mesh(IE::Graphics::RenderEngine *t_renderEngine) :
        m_indexBuffer(Buffer::create(t_renderEngine)),
        m_vertexBuffer(Buffer::create(t_renderEngine)) {
}
