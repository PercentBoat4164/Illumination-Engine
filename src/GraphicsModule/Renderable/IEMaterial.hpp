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

class IEMaterial {
    int diffuseTextureIndex = 0;
    glm::vec4 diffuseColor {1.0F, 1.0F, 1.0F, 1.0F};
};