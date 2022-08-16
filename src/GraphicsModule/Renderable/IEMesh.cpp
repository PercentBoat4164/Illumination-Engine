#include "IEMesh.hpp"
#include "Buffer/IEBuffer.hpp"
#include "Core/LogModule/IELogger.hpp"
#include "IERenderEngine.hpp"

IEMesh::IEMesh(IERenderEngine *engineLink) {
	create(engineLink);
}

void IEMesh::setAPI(const IEAPI &API) {
	if (API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		_loadFromDiskToRAM = &IEMesh::_openglLoadFromDiskToRAM;
		_loadFromRAMToVRAM = &IEMesh::_openglLoadFromRAMToVRAM;
		_update = &IEMesh::_openglUpdate;
		_unloadFromVRAM = &IEMesh::_openglUnloadFromVRAM;
		_unloadFromRAM = &IEMesh::_openglUnloadFromRAM;
	} else if (API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		_loadFromDiskToRAM = &IEMesh::_vulkanLoadFromDiskToRAM;
		_loadFromRAMToVRAM = &IEMesh::_vulkanLoadFromRAMToVRAM;
		_update = &IEMesh::_vulkanUpdate;
		_unloadFromVRAM = &IEMesh::_vulkanUnloadFromVRAM;
		_unloadFromRAM = &IEMesh::_vulkanUnloadFromRAM;
	}
}

void IEMesh::create(IERenderEngine *engineLink) {
	linkedRenderEngine = engineLink;
	vertexBuffer = std::make_shared<IEBuffer>();
	indexBuffer = std::make_shared<IEBuffer>();
	descriptorSet = std::make_shared<IEDescriptorSet>();
	pipeline = std::make_shared<IEPipeline>();
	material = std::make_shared<IEMaterial>();
	material->create(linkedRenderEngine);
}


std::function<void(IEMesh &, const std::string &, const aiScene *, aiMesh *)> IEMesh::_loadFromDiskToRAM{nullptr};

void IEMesh::loadFromDiskToRAM(const std::string &directory, const aiScene *scene, aiMesh *mesh) {
	_loadFromDiskToRAM(*this, directory, scene, mesh);
}

void IEMesh::_openglLoadFromDiskToRAM(const std::string &directory, const aiScene *scene, aiMesh *mesh) {
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
			.type=GL_ARRAY_BUFFER,
	};
	vertexBuffer->create(linkedRenderEngine, &vertexBufferCreateInfo);
	vertexBuffer->uploadToRAM(vertices.data(), vertexBufferCreateInfo.size);

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
			.type=GL_ELEMENT_ARRAY_BUFFER,
	};
	indexBuffer->create(linkedRenderEngine, &indexBufferCreateInfo);
	indexBuffer->uploadToRAM(indices.data(), indexBufferCreateInfo.size);

	// load material
	material->loadFromDiskToRAM(directory, scene, mesh->mMaterialIndex);
}

void IEMesh::_vulkanLoadFromDiskToRAM(const std::string &directory, const aiScene *scene, aiMesh *mesh) {
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
	vertexBuffer->uploadToRAM(vertices.data(), vertexBufferCreateInfo.size);

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
	indexBuffer->uploadToRAM(indices.data(), indexBufferCreateInfo.size);

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


std::function<void(IEMesh &)> IEMesh::_loadFromRAMToVRAM{nullptr};

void IEMesh::loadFromRAMToVRAM() {
	_loadFromRAMToVRAM(*this);
}

void IEMesh::_openglLoadFromRAMToVRAM() {
	material->loadFromRAMToVRAM();

	vertexBuffer->uploadToVRAM();

	indexBuffer->uploadToVRAM();

	shaders.resize(2);
	shaders[0] = std::make_shared<IEShader>();
	shaders[0]->create(linkedRenderEngine, new IEFile{"shaders/OpenGL/vertexShader.vert"});
	shaders[1] = std::make_shared<IEShader>();
	shaders[1]->create(linkedRenderEngine, new IEFile{"shaders/OpenGL/fragmentShader.frag"});

	pipeline->create(linkedRenderEngine, new IEPipeline::CreateInfo{
			.shaders=shaders,
	});
	
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);
	
	glBindBuffer(vertexBuffer->type, vertexBuffer->id);
	glBindBuffer(indexBuffer->type, indexBuffer->id);

	IEVertex::useVertexAttributesWithProgram(pipeline->programID);
	
	glBindBuffer(vertexBuffer->type, 0);
}

void IEMesh::_vulkanLoadFromRAMToVRAM() {
	material->loadFromRAMToVRAM();

	vertexBuffer->uploadToVRAM();
	deletionQueue.emplace_back([&] { vertexBuffer->destroy(); });

	indexBuffer->uploadToVRAM();
	deletionQueue.emplace_back([&] { indexBuffer->destroy(); });

	// Set up shaders
	shaders.resize(2);
	shaders[0] = std::make_shared<IEShader>();
	shaders[0]->create(linkedRenderEngine, new IEFile{"shaders/Vulkan/vertexShader.vert.spv"});
	shaders[1] = std::make_shared<IEShader>();
	shaders[1]->create(linkedRenderEngine, new IEFile{"shaders/Vulkan/fragmentShader.frag.spv"});

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


std::function<void(IEMesh &, uint32_t)> IEMesh::_update{nullptr};

void IEMesh::update(uint32_t commandBufferIndex) {
	_update(*this, commandBufferIndex);
}

void IEMesh::_openglUpdate(uint32_t commandBufferIndex) {
	// Set shader program
	glUseProgram(pipeline->programID);
	
	// Set vertices
	glBindVertexArray(vertexArray);
	glBindBuffer(indexBuffer->type, indexBuffer->id);
	
	// Update texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, linkedRenderEngine->textures[material->diffuseTextureIndex]->id);
	
	// Set active texture for diffuseTexture in shader
	glUniform1i(glGetUniformLocation(pipeline->programID, "diffuseTexture"), 0);  // Magic number is the active texture.
	
	// Draw mesh
	glDrawElements(GL_TRIANGLES, (GLsizei) indices.size(), GL_UNSIGNED_INT, nullptr);
	
	//Reset All
	glUseProgram(0);
	glBindVertexArray(0);
	glBindBuffer(indexBuffer->type, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
}

void IEMesh::_vulkanUpdate(uint32_t commandBufferIndex) {
	std::array<VkDeviceSize, 1> offsets{0};
	linkedRenderEngine->graphicsCommandPool->index(commandBufferIndex)->recordBindVertexBuffers(0, 1, {vertexBuffer}, offsets.data());
	linkedRenderEngine->graphicsCommandPool->index(commandBufferIndex)->recordBindIndexBuffer(indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	linkedRenderEngine->graphicsCommandPool->index(commandBufferIndex)->recordBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	linkedRenderEngine->graphicsCommandPool->index(commandBufferIndex)->recordBindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline, 0,
																								 {descriptorSet}, {});
	linkedRenderEngine->graphicsCommandPool->index(commandBufferIndex)->recordDrawIndexed(indices.size(), 1, 0, 0, 0);
}


std::function<void(IEMesh &)> IEMesh::_unloadFromVRAM{nullptr};

void IEMesh::unloadFromVRAM() {
	_unloadFromVRAM(*this);
}

void IEMesh::_openglUnloadFromVRAM() {

}

void IEMesh::_vulkanUnloadFromVRAM() {
	vertexBuffer->unloadFromVRAM();
	indexBuffer->unloadFromVRAM();
}


std::function<void(IEMesh &)> IEMesh::_unloadFromRAM{nullptr};

void IEMesh::unloadFromRAM() {
	_unloadFromRAM(*this);
}

void IEMesh::_openglUnloadFromRAM() {

}

void IEMesh::_vulkanUnloadFromRAM() {
	vertices.clear();
	indices.clear();
}
