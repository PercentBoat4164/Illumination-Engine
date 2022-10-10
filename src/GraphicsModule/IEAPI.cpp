/* Include this file's header. */
#include "IEAPI.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"

/* Include dependencies from Core. */
#include "Core/LogModule/IELogger.hpp"

/* Include external dependencies. */
#include <vulkan/vulkan.h>

#define GLEW_IMPLEMENTATION

#include <GL/glew.h>
IEAPI::IEAPI(const std::string &apiName) {
    name = apiName;
}

IEAPI::IEAPI() = default;

IEVersion IEAPI::getHighestSupportedVersion(IERenderEngine *linkedRenderEngine) const {
    IEVersion temporaryVersion;
    if (name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(linkedRenderEngine->device.physical_device, &properties);
        temporaryVersion = IEVersion(properties.apiVersion);
    }
    if (name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
        GLint OpenGLVersionMajor, OpenGLVersionMinor;
        glGetIntegerv(GL_MAJOR_VERSION, &OpenGLVersionMajor);
        glGetIntegerv(GL_MINOR_VERSION, &OpenGLVersionMinor);
        temporaryVersion =
          IEVersion{static_cast<uint32_t>(OpenGLVersionMajor), static_cast<uint32_t>(OpenGLVersionMinor), 0};
    }
    return temporaryVersion;
}
