#pragma once

#include "IERenderEngineLink.hpp"

#ifdef ILLUMINATION_ENGINE_VULKAN
#include "spirv_cross/spirv.h"
#endif

#include <cstring>

class IEShader {
public:
    struct CreateInfo {
        //Required
        std::string filename{};
    };

    CreateInfo createdWith{};
    IERenderEngineLink linkedRenderEngine{};

    void compile() const {

    }
};