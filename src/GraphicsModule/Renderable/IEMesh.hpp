#pragma once

#include "Buffer/IEBuffer.hpp"
#include "GraphicsModule/Shader/IEDescriptorSet.hpp"
#include "GraphicsModule/Shader/IEPipeline.hpp"
#include "IEMaterial.hpp"
#include "IEVertex.hpp"

#include <cstdint>
#include <vector>

class IERenderEngine;

class IEMesh {
public:
    IEMesh() = default;

    explicit IEMesh(IERenderEngine *);

    static void setAPI(const API &API);


    void create(IERenderEngine *);


    static std::function<void(IEMesh &, const std::string &, const aiScene *, aiMesh *)> _loadFromDiskToRAM;

    void loadFromDiskToRAM(const std::string &, const aiScene *, aiMesh *);

    void _openglLoadFromDiskToRAM(const std::string &, const aiScene *, aiMesh *);

    void _vulkanLoadFromDiskToRAM(const std::string &, const aiScene *, aiMesh *);


    static std::function<void(IEMesh &)> _loadFromRAMToVRAM;

    void loadFromRAMToVRAM();

    void _openglLoadFromRAMToVRAM();

    void _vulkanLoadFromRAMToVRAM();


    static std::function<void(IEMesh &, uint32_t)> _update;

    void update(uint32_t);

    void _openglUpdate(uint32_t);

    void _vulkanUpdate(uint32_t);


    static std::function<void(IEMesh &)> _unloadFromVRAM;

    void unloadFromVRAM();

    void _openglUnloadFromVRAM();

    void _vulkanUnloadFromVRAM();


    static std::function<void(IEMesh &)> _unloadFromRAM;

    void unloadFromRAM();

    void _openglUnloadFromRAM();

    void _vulkanUnloadFromRAM();


    IERenderEngine                        *linkedRenderEngine{};
    std::shared_ptr<IEDescriptorSet>       descriptorSet{};
    std::shared_ptr<IEPipeline>            pipeline{};
    std::vector<IEVertex>                  vertices{};
    std::vector<uint32_t>                  indices{};
    std::shared_ptr<IEBuffer>              vertexBuffer{};
    uint32_t                               triangleCount{};
    std::vector<std::shared_ptr<IEShader>> shaders{};  // Should be moved to material
    std::shared_ptr<IEBuffer>              indexBuffer{};
    std::shared_ptr<IEMaterial>            material{};
    std::vector<std::function<void()>>     deletionQueue{};
    GLuint                                 vertexArray{};
};