#pragma once

/* Define macros used throughout the file. */
#ifdef WIN32
#    define GLSLC "glslc.exe "
#else
#    define GLSLC "glslc "
#endif

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or function arguments. */
#include "IEAPI.hpp"

// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include "Core/FileSystemModule/File.hpp"

#include <functional>
#include <string>
#include <vector>

#define GLEW_IMPLEMENTATION

#include <include/GL/glew.h>

class IEShader {
public:
    std::vector<char>                  data;
    std::vector<std::function<void()>> deletionQueue;
    VkShaderModule                     module;
    IERenderEngine                    *linkedRenderEngine;
    IE::Core::File                    *file;
    GLuint                             shaderID;

private:
    static std::function<void(IEShader &, IERenderEngine *, IE::Core::File *)> _create;
    static std::function<void(IEShader &, const std::string &, std::string)>   _compile;

protected:
    void _openglCompile(const std::string &, std::string);

    void _vulkanCompile(const std::string &, std::string);


    void _openglCreate(IERenderEngine *, IE::Core::File *);

    void _vulkanCreate(IERenderEngine *, IE::Core::File *);

public:
    void destroy();

    ~IEShader();

    void compile(const std::string &, const std::string &);

    void create(IERenderEngine *, IE::Core::File *);

    static void setAPI(const IEAPI &);
};