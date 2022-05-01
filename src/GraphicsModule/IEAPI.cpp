/* Include this file's header. */
#include "IEAPI.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"

/* Include dependencies from Core. */
#include "Core/LogModule/IELogger.hpp"

/* Include external dependencies. */
#include <vulkan/vulkan.h>

#include <GL/glew.h>


IEAPI::IEAPI(const std::string &apiName) {
    name = apiName;
}

IEAPI::IEAPI() = default;

IEVersion IEAPI::getHighestSupportedVersion(IERenderEngine *linkedRenderEngine) const {
    IEVersion temporaryVersion;
    #ifdef ILLUMINATION_ENGINE_VULKAN
    if (name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(linkedRenderEngine->device.physical_device, &properties);
        temporaryVersion = IEVersion(properties.apiVersion);
    }
    #endif
    #ifdef ILLUMINATION_ENGINE_OPENGL
    if (name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
        std::string openGLVersion = std::string(reinterpret_cast<const char *>(glGetString(GL_VERSION)));
        temporaryVersion = IEVersion{static_cast<uint32_t>(std::stoi(openGLVersion.substr(0, 1))), static_cast<uint32_t>(std::stoi(openGLVersion.substr(2, 1))), 0};
        if (temporaryVersion.major == 0) {
            linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "No OpenGL version was found!");
        }
    }
    #endif
    return temporaryVersion;
}

