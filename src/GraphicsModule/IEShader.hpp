#pragma once

/* Define macros used throughout the file. */
#ifdef WIN32
#define GLSLC "glslc.exe "
#else
#define GLSLC "glslc "
#endif

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <string>
#include <vector>
#include <functional>
#include "Core/FileSystemModule/IEFile.hpp"


class IEShader {
public:
    std::vector<char> data{};
    std::vector<std::function<void()>> deletionQueue{};
    VkShaderModule module{};
    IERenderEngine *linkedRenderEngine{};
    IEFile *file;
    bool compiled{false};

    void destroy();

    ~IEShader();

    void compile(const std::string& input, std::string output = "") const;

    IEShader create(IERenderEngine *renderEngineLink, IEFile *shaderFile);
};