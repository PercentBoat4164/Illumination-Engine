#include "Material.hpp"

#include "../contrib/stb/stb_image.h"
#include "Core/Core.hpp"
#include "Core/FileSystemModule/FileSystem.hpp"

void IE::Graphics::Material::load(const aiScene *scene, aiMaterial *material, IE::Core::File *file) {
    // find all textures in scene including embedded textures
    uint32_t thisCount;
    for (size_t i = 0; i < supportedTextureTypes.size(); ++i) {
        thisCount = material->GetTextureCount(supportedTextureTypes[i]);
        // Remove any unused texture types
        if (thisCount == 0) supportedTextureTypes.erase(supportedTextureTypes.begin() + i--);
    }

    aiString   texturePath{};
    aiTexture *texture;
    uint32_t   textureIndex{0};

    // load textures
    stbi_uc *tempData;
    int      width, height, channels;
    for (aiTextureType &textureType : supportedTextureTypes) {
        while (texturePath.length == 0) material->GetTexture(textureType, textureIndex++, &texturePath);
        texture = const_cast<aiTexture *>(scene->GetEmbeddedTexture(texturePath.C_Str()));
        if (texture == nullptr || texture->mHeight != 0) {  // is the texture not an embedded texture?
            tempData = stbi_load(
              reinterpret_cast<const char *>(
                (file->path.parent_path().parent_path() / "textures" / texturePath.C_Str()).c_str()
              ),
              reinterpret_cast<int *>(&width),
              reinterpret_cast<int *>(&height),
              reinterpret_cast<int *>(&channels),
              4
            );
        } else {
            tempData = stbi_load_from_memory(
              (unsigned char *) texture->pcData,
              (int) texture->mWidth,
              reinterpret_cast<int *>(&width),
              reinterpret_cast<int *>(&height),
              reinterpret_cast<int *>(&channels),
              4
            );
        }
        std::vector<char> data{(char *) tempData, (char *) ((uint64_t) tempData + width * height * channels)};
        stbi_image_free(tempData);
        if (data.empty()) {
            IE::Core::Core::getLogger().log(
              std::string{"Failed to load image data from file: '"} + texture->mFilename.C_Str() + "' due to " +
                stbi_failure_reason(),
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
            );
        }
    }

    //
}
