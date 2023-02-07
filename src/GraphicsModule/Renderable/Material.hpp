#pragma once

#include "Image/Image.hpp"

#include <assimp/material.h>
#include <assimp/scene.h>

namespace IE::Core {
class File;
}  // namespace IE::Core

namespace IE::Graphics {
class Material {
public:
    std::shared_ptr<Image> diffuseTexture;
    std::shared_ptr<Image> specularTexture;

    void load(const aiScene *scene, aiMaterial *material, IE::Core::File *file);

private:
    std::vector<aiTextureType> supportedTextureTypes{aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR};
};
}  // namespace IE::Graphics
