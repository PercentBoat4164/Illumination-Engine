#pragma once

/* Define macros used throughout the file. */
#define IE_RENDER_ENGINE_API_NAME_VULKAN "Vulkan"
#define IE_RENDER_ENGINE_API_NAME_OPENGL "OpenGL"

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IEVersion.hpp"

// System dependencies
#include <string>

class IEAPI{
public:
    explicit IEAPI(const std::string& apiName);

    IEAPI();

    std::string name{}; // The name of the API to use
    IEVersion version{}; // The version of the API to use

    /**
     * @brief Finds the highest supported API version.
     * @return An IEVersion populated with all the data about the highest supported API version
     */
    IEVersion getHighestSupportedVersion(IERenderEngine* linkedRenderEngine) const;
};