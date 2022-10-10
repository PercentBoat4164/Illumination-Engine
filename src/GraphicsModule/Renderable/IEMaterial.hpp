#pragma once

#include "API/API.hpp"
#include "assimp/material.h"
#include "assimp/scene.h"
#include "assimp/texture.h"
#include "Image/Texture.hpp"
#include "stb_image.h"

#include <iostream>
#include <string>
#include <unordered_map>

#include <../contrib/stb/stb_image.h>
#include <iostream>
#include <string>
#include <unordered_map>

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>

class IEMesh;

class IEMaterial {
public:
    uint32_t        textureCount{};
    uint32_t        diffuseTextureIndex{};
    glm::vec4       diffuseColor{1.0F, 1.0F, 1.0F, 1.0F};
    IERenderEngine *linkedRenderEngine{};

    IEMaterial() = default;

    explicit IEMaterial(IERenderEngine *engineLink);

    static void setAPI(const API &API);


    static std::function<void(IEMaterial &)> _create;

    void create(IERenderEngine *);

    void _openglCreate();

    void _vulkanCreate();


    static std::function<void(IEMaterial &, const std::string &, const aiScene *, uint32_t)> _loadFromDiskToRAM;

    void loadFromDiskToRAM(const std::string &, const aiScene *, uint32_t);

    void _openglLoadFromDiskToRAM(const std::string &, const aiScene *, uint32_t);

    void _vulkanLoadFromDiskToRAM(const std::string &, const aiScene *, uint32_t);


    static std::function<void(IEMaterial &)> _loadFromRAMToVRAM;

    void loadFromRAMToVRAM();

    void _openglLoadFromRAMToVRAM();

    void _vulkanLoadFromRAMToVRAM();


    static std::function<void(IEMaterial &)> _unloadFromVRAM;

    void unloadFromVRAM();

    void _openglUnloadFromVRAM();

    void _vulkanUnloadFromVRAM();


    static std::function<void(IEMaterial &)> _unloadFromRAM;

    void unloadFromRAM();

    void _openglUnloadFromRAM();

    void _vulkanUnloadFromRAM();

private:
    std::vector<std::pair<uint32_t *, aiTextureType>> supportedTextureTypes = {
      {&diffuseTextureIndex, aiTextureType_DIFFUSE   },
      {&diffuseTextureIndex, aiTextureType_BASE_COLOR},
    };
};