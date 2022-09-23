#pragma once

/* Define macros used throughout the file. */
#define IE_RENDER_ENGINE_API_NAME_VULKAN "Vulkan"
#define IE_RENDER_ENGINE_API_NAME_OPENGL "OpenGL"

/* Predefine classes used with pointers or as return values for functions. */
namespace IE::Graphics { class RenderEngine; }

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "Version.hpp"

// System dependencies
#include <string>

namespace IE::Graphics {
class API {
public:
    explicit API(const std::string &apiName);

    API();

    std::string name{};     // The name of the API to use
    Version     version{};  // The version of the API to use

    /**
	 * @brief Finds the highest supported API version.
	 * @return An Version populated with all the data about the highest supported API version
     */
    Version getHighestSupportedVersion(IE::Graphics::RenderEngine *linkedRenderEngine) const;
};
}