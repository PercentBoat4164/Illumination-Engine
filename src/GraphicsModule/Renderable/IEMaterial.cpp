#include "IEMaterial.hpp"

#include <memory>

#include "IERenderEngine.hpp"
#include "IEMesh.hpp"

void IEMaterial::create(IERenderEngine *engineLink) {
	linkedRenderEngine = engineLink;
}

void IEMaterial::loadFromDiskToRAM(const std::string &directory, const aiScene *scene, uint32_t index) {
	aiMaterial *material = scene->mMaterials[index];

	// find all textures in scene including embedded textures
	textureCount = 0;
	uint32_t thisCount;
	/**@todo Build a system that tracks duplicate textures and only keeps one copy in memory.*/
	for (int i = 0; i < textureTypes.size(); ++i) {
		thisCount = material->GetTextureCount(textureTypes[i].second);
		if (thisCount == 0) {
			textureTypes.erase(textureTypes.begin() + i--);
		}
		textureCount += thisCount;
	}
	linkedRenderEngine->textures.reserve(linkedRenderEngine->textures.size() + textureCount);

	aiString texturePath{};
	std::string data{};
	aiTexture *texture;
	uint32_t textureIndex{0};

	// load all textures despite embedded state
	for (std::pair<uint32_t *, aiTextureType> textureType: textureTypes) {
		while (texturePath.length == 0 && textureIndex < textureCount) {
			material->GetTexture(textureType.second, textureIndex++, &texturePath);
		}
		texture = const_cast<aiTexture *>(scene->GetEmbeddedTexture(texturePath.C_Str()));
		if (texture == nullptr || texture->mHeight != 0) {  // is the texture not an embedded texture?
			texture = new aiTexture;
			texture->mFilename = directory + "/" + texturePath.C_Str();
			texture->mHeight = 1;  // flag texture as not embedded
		}
		*textureType.first = linkedRenderEngine->textures.size();
		linkedRenderEngine->textures.push_back(std::make_shared<IETexture>(linkedRenderEngine, new IETexture::CreateInfo));
		linkedRenderEngine->textures[*textureType.first]->loadFromDiskToRAM(texture);
	}
}

void IEMaterial::loadFromRAMToVRAM() const {
	for (std::pair<uint32_t *, aiTextureType> textureType: textureTypes) {
		linkedRenderEngine->textures[*textureType.first]->loadFromRAMToVRAM();
	}
}
