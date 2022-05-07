#include "IEMesh.hpp"
#include "Buffer/IEBuffer.hpp"
#include "Core/LogModule/IELogger.hpp"
#include "IERenderEngine.hpp"

void IEMesh::loadFromDiskToRAM(const std::string &directory, const aiScene *scene, aiMesh *mesh) {
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
	int indexInFace;
	for (int i = 0; i < mesh->mNumFaces; ++i) {
		if (mesh->mFaces[i].mNumIndices != 3) {
			linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN,
													 "Detected non-triangular face! Try using the aiProcess_Triangulate flag.");
		}
		for (indexInFace = 0; indexInFace < mesh->mFaces[i].mNumIndices; ++indexInFace) {
			indices.push_back(mesh->mFaces[i].mIndices[indexInFace]);
		}
	}

	// assuming all faces are triangles
	triangleCount = mesh->mNumFaces;

	// load material
	material.loadFromDiskToRAM(directory, scene, mesh->mMaterialIndex);
}

void IEMesh::loadFromRAMToVRAM() {
	material.loadFromRAMToVRAM();

	IEBuffer::CreateInfo vertexBufferCreateInfo{
			.size=sizeof(vertices[0]) * vertices.size(),
			.usage=VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			.allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU,
	};
	vertexBuffer.create(linkedRenderEngine, &vertexBufferCreateInfo);
	vertexBuffer.loadFromDiskToRAM(vertices.data());
	indexBuffer.loadFromRAMToVRAM();
	deletionQueue.emplace_back([&] { vertexBuffer.destroy(); });

	IEBuffer::CreateInfo indexBufferCreateInfo{
			.size=sizeof(indices[0]) * indices.size(),
			.usage=VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			.allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU,
	};
	indexBuffer.create(linkedRenderEngine, &indexBufferCreateInfo);
	indexBuffer.loadFromDiskToRAM(vertices.data());
	indexBuffer.loadFromRAMToVRAM();
	deletionQueue.emplace_back([&] { indexBuffer.destroy(); });
}

std::function<void(IEMesh &)> IEMesh::_create = std::function<void(IEMesh &)>{[](IEMesh &) { return; }};

void IEMesh::create(IERenderEngine *engineLink) {
	linkedRenderEngine = engineLink;
	material.create(this);
	return _create(*this);
}

IEMesh::~IEMesh() {
	destroy();
}

void IEMesh::destroy() {
	for (const std::function<void()> &function: deletionQueue) {
		function();
	}
	deletionQueue.clear();
}


