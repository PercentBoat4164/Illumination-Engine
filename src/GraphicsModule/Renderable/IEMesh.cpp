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

	// Create vertex buffer.
	IEBuffer::CreateInfo vertexBufferCreateInfo{
			.size=sizeof(vertices[0]) * vertices.size(),
			.usage=VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			.allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU,
	};
	vertexBuffer->create(linkedRenderEngine, &vertexBufferCreateInfo);
	vertexBuffer->loadFromDiskToRAM(vertices.data(), vertexBufferCreateInfo.size);

	// assuming all faces are triangles
	triangleCount = mesh->mNumFaces;

	// record vertices
	indices.reserve(3UL * triangleCount);
	int j;
	for (int i = 0; i < triangleCount; ++i) {
		if (mesh->mFaces[i].mNumIndices != 3) {
			linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN,
													 "Attempted to add a non-triangular face to a mesh! Try using the aiProcess_Triangulate flag.");
		}
		for (j = 0; j < mesh->mFaces[i].mNumIndices; ++j) {
			indices.push_back(mesh->mFaces[i].mIndices[j]);
		}
	}

	// Create index buffer
	IEBuffer::CreateInfo indexBufferCreateInfo{
			.size=sizeof(indices[0]) * indices.size(),
			.usage=VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			.allocationUsage=VMA_MEMORY_USAGE_CPU_TO_GPU,
	};
	indexBuffer->create(linkedRenderEngine, &indexBufferCreateInfo);
	indexBuffer->loadFromDiskToRAM(indices.data(), indexBufferCreateInfo.size);

	// load material
	material->loadFromDiskToRAM(directory, scene, mesh->mMaterialIndex);

	// create descriptor set
	IEDescriptorSet::CreateInfo descriptorSetCreateInfo{
			.poolSizes={{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1},
						{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}},
			.shaderStages={static_cast<VkShaderStageFlagBits>(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT),
						   VK_SHADER_STAGE_FRAGMENT_BIT},
			.data={std::nullopt, std::nullopt}
	};
	descriptorSet->create(linkedRenderEngine, &descriptorSetCreateInfo);
	deletionQueue.emplace_back([&] { descriptorSet->destroy(); });
}

void IEMesh::loadFromRAMToVRAM() {
	material->loadFromRAMToVRAM();

	vertexBuffer->loadFromRAMToVRAM();
	deletionQueue.emplace_back([&] { vertexBuffer->destroy(); });

	indexBuffer->loadFromRAMToVRAM();
	deletionQueue.emplace_back([&] { indexBuffer->destroy(); });

	// Set up shaders
	shaders.resize(2);
	shaders[0] = std::make_shared<IEShader>();
	shaders[0]->create(linkedRenderEngine, new IEFile{"shaders/Rasterize/vertexShader.vert.spv"});
	shaders[1] = std::make_shared<IEShader>();
	shaders[1]->create(linkedRenderEngine, new IEFile{"shaders/Rasterize/fragmentShader.frag.spv"});

	// Set up pipeline
	pipeline->create(linkedRenderEngine, new IEPipeline::CreateInfo{
			.shaders=shaders,
			.descriptorSet=descriptorSet,
			.renderPass=linkedRenderEngine->renderPass  /**@todo Make renderPass of pipeline adjustable.*/
	});

	// Set up descriptor set
	linkedRenderEngine->textures[material->diffuseTextureIndex]->transitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	descriptorSet->update({linkedRenderEngine->textures[material->diffuseTextureIndex].get()}, {1});
}

std::function<void(IEMesh &)> IEMesh::_create = std::function<void(IEMesh &)>{[](IEMesh &) { return; }};

void IEMesh::create(IERenderEngine *engineLink) {
	linkedRenderEngine = engineLink;
	vertexBuffer = std::make_shared<IEBuffer>();
	indexBuffer = std::make_shared<IEBuffer>();
	descriptorSet = std::make_shared<IEDescriptorSet>();
	pipeline = std::make_shared<IEPipeline>();
	material = std::make_shared<IEMaterial>();
	material->create(linkedRenderEngine);
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

void IEMesh::update(uint32_t commandBufferIndex) {
	VkDeviceSize offsets[]{0};
	linkedRenderEngine->graphicsCommandPool->index(commandBufferIndex)->recordBindVertexBuffers(0, 1, {vertexBuffer}, offsets);
	linkedRenderEngine->graphicsCommandPool->index(commandBufferIndex)->recordBindIndexBuffer(indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	linkedRenderEngine->graphicsCommandPool->index(commandBufferIndex)->recordBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	linkedRenderEngine->graphicsCommandPool->index(commandBufferIndex)->recordBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline, 0,
																								 {descriptorSet}, {});
	linkedRenderEngine->graphicsCommandPool->index(commandBufferIndex)->recordDrawIndexed(indices.size(), 1, 0, 0, 0);
}


