#pragma once

#include "Image/Image.hpp"

#include <assimp/material.h>
#include <assimp/scene.h>

namespace IE::Graphics {
class Material {
public:
    std::shared_ptr<Image> diffuseTexture;
    std::shared_ptr<Image> specularTexture;

    void load(const aiScene *scene, size_t index) {
        aiMaterial *material = scene->mMaterials[index];

        // find all textures in scene including embedded textures
        uint32_t thisCount;
        /**@todo Build a system that tracks duplicate textures and only keeps one copy in memory.*/
        for (size_t i = 0; i < supportedTextureTypes.size(); ++i) {
            thisCount = material->GetTextureCount(supportedTextureTypes[i]);
            // Remove any unused texture types
            if (thisCount == 0) supportedTextureTypes.erase(supportedTextureTypes.begin() + i--);
        }

        aiString    texturePath{};
        std::string data{};
        aiTexture  *texture;
        uint32_t    textureIndex{0};

        // load all textures despite embedded state
        //        for (aiTextureType &textureType : supportedTextureTypes) {
        //            while (texturePath.length == 0) material->GetTexture(textureType, textureIndex++,
        //            &texturePath); texture = const_cast<aiTexture
        //            *>(scene->GetEmbeddedTexture(texturePath.C_Str())); if (texture == nullptr ||
        //            texture->mHeight != 0) {  // is the texture not an embedded texture?
        //                texture = new aiTexture;
        //                texture->mFilename =
        //                  directory.substr(0, directory.find_last_of('/')) + "/textures/" + texturePath.C_Str();
        //                texture->mHeight = 1;  // flag texture as not embedded
        //            }
        //            *textureType.first = linkedRenderEngine->textures.size();
        //
        //            linkedRenderEngine->textures.push_back(std::make_shared<IETexture>(linkedRenderEngine,
        //            &imageCreateInfo)); linkedRenderEngine->textures[*textureType.first]->uploadToRAM(texture);
        //        }
    }

private:
    std::vector<aiTextureType> supportedTextureTypes{aiTextureType_DIFFUSE, aiTextureType_BASE_COLOR};
};
}  // namespace IE::Graphics
