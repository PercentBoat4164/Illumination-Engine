#include "IEMaterial.hpp"

#include <memory>

#include "IERenderEngine.hpp"
#include "IEMesh.hpp"

void IEMaterial::create(IEMesh *pParentMesh) {
	parentMesh = pParentMesh;
}

void IEMaterial::loadFromDiskToRAM(const std::string &directory, const aiScene *scene, uint32_t index) {
	aiMaterial *material = scene->mMaterials[index];

	// texture types to load
	std::vector<std::pair<uint32_t *, aiTextureType>> textureTypes{
			{&diffuseTextureIndex, aiTextureType_BASE_COLOR},
	};

	// find all textures in scene including embedded textures
	/**@todo Build a system that tracks duplicate textures and only keeps one copy in memory.*/
	for (int i = 0; i < textureTypes.size(); ++i) {
		uint32_t thisCount = material->GetTextureCount(textureTypes[i].second);
		if (thisCount == 0) {
			textureTypes.erase(textureTypes.begin() + i--);
		}
		textureCount += thisCount;
	}
	parentMesh->linkedRenderEngine->textures.resize(parentMesh->linkedRenderEngine->textures.size() + textureCount,
													std::make_shared<IETexture>(parentMesh->linkedRenderEngine, new IETexture::CreateInfo));

	aiString texturePath{};
	std::string data{};
	// load all textures despite embedded state
	uint32_t textureIndex = 0;
	for (std::pair<uint32_t *, aiTextureType> textureType: textureTypes) {
		while (texturePath.length == 0 && textureIndex < textureCount) {
			material->GetTexture(textureType.second, textureIndex++, &texturePath);
		}
		auto *embeddedTexture = const_cast<aiTexture *>(scene->GetEmbeddedTexture(texturePath.C_Str()));
		embeddedTexture->mFilename = directory + embeddedTexture->mFilename.C_Str();
		parentMesh->linkedRenderEngine->textures[parentMesh->linkedRenderEngine->textures.size()]->loadFromDiskToRAM(embeddedTexture);
	}
}

void IEMaterial::loadFromRAMToVRAM() const {
	parentMesh->linkedRenderEngine->textures[diffuseTextureIndex]->loadFromRAMToVRAM();
}