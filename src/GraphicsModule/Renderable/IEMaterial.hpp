#pragma once

#include <iostream>
#include "assimp/material.h"
#include "stb_image.h"
#include "assimp/texture.h"
#include "assimp/scene.h"
#include <unordered_map>
#include <string>
#include "Image/IETexture.hpp"
#include "IERenderableSettings.hpp"
#include "glm/glm.hpp"

class IEMesh;

class IEMaterial {
public:
	IEMesh *parentMesh{};
	uint32_t textureCount{};
	uint32_t diffuseTextureIndex{};
	glm::vec4 diffuseColor{1.0F, 1.0F, 1.0F, 1.0F};

	void create(IEMesh *);

	void loadFromDiskToRAM(const std::string &, const aiScene *, uint32_t);

	void loadFromRAMToVRAM() const;

private:
	std::vector<std::pair<uint32_t *, aiTextureType>> textureTypes = {
			{&diffuseTextureIndex, aiTextureType_DIFFUSE},
			{&diffuseTextureIndex, aiTextureType_BASE_COLOR},
	};
};