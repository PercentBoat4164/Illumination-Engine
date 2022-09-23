/* Include this file's header. */
#include "API.hpp"

/* Include dependencies within this module. */
#include "RenderEngine.hpp"

/* Include dependencies from Core. */
#include "Core/LogModule/IELogger.hpp"

/* Include external dependencies. */
#include <vulkan/vulkan.h>

#define GLEW_IMPLEMENTATION
#include "include/GL/glew.h"

IE::Graphics::API::API(const std::string &apiName) {
    name = apiName;
}

IE::Graphics::API::API() = default;

IE::Graphics::Version IE::Graphics::API::getHighestSupportedVersion(IE::Graphics::RenderEngine *linkedRenderEngine) const {
    Version temporaryVersion;
    if (name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(linkedRenderEngine->getPhysicalDevice(), &properties);
        temporaryVersion = Version(properties.apiVersion);
    }
    if (name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
        GLint OpenGLVersionMajor, OpenGLVersionMinor;
        glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersionMajor);
        glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersionMinor);
        temporaryVersion =
          Version{static_cast<uint32_t>(OpenGLVersionMajor), static_cast<uint32_t>(OpenGLVersionMinor), 0};
    }
    return temporaryVersion;
}
