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


class IEShader {
public:
    struct CreateInfo {
        std::string filename{};
    };

    std::vector<char> data{};
    std::vector<std::function<void()>> deletionQueue{};
    VkShaderModule module{};
    IERenderEngine *linkedRenderEngine{};
    CreateInfo createdWith{};
    bool compiled{false};

    void destroy();

    void create(IERenderEngine *renderEngineLink, CreateInfo *createInfo);

    ~IEShader();

    void compile(const std::string& input, std::string output = "") const;
};