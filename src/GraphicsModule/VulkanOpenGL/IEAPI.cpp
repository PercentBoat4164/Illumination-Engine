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
        // Line of if statements that finds the correct OpenGL version in use
        if (GLEW_VERSION_4_6) { temporaryVersion = IEVersion{"4.6.0"}; }
        else if (GLEW_VERSION_4_5) { temporaryVersion = IEVersion{"4.5.0"}; }
        else if (GLEW_VERSION_4_4) { temporaryVersion = IEVersion{"4.4.0"}; }
        else if (GLEW_VERSION_4_3) { temporaryVersion = IEVersion{"4.3.0"}; }
        else if (GLEW_VERSION_4_2) { temporaryVersion = IEVersion{"4.2.0"}; }
        else if (GLEW_VERSION_4_1) { temporaryVersion = IEVersion{"4.1.0"}; }
        else if (GLEW_VERSION_4_0) { temporaryVersion = IEVersion{"4.0.0"}; }
        else if (GLEW_VERSION_3_3) { temporaryVersion = IEVersion{"3.3.0"}; }
        else if (GLEW_VERSION_3_2) { temporaryVersion = IEVersion{"3.2.0"}; }
        else if (GLEW_VERSION_3_1) { temporaryVersion = IEVersion{"3.1.0"}; }
        else if (GLEW_VERSION_3_0) { temporaryVersion = IEVersion{"3.0.0"}; }
        else if (GLEW_VERSION_2_1) { temporaryVersion = IEVersion{"2.1.0"}; }
        else if (GLEW_VERSION_2_0) { temporaryVersion = IEVersion{"2.0.0"}; }
        else if (GLEW_VERSION_1_5) { temporaryVersion = IEVersion{"1.5.0"}; }
        else if (GLEW_VERSION_1_4) { temporaryVersion = IEVersion{"1.4.0"}; }
        else if (GLEW_VERSION_1_3) { temporaryVersion = IEVersion{"1.3.0"}; }
        else if (GLEW_VERSION_1_2_1) { temporaryVersion = IEVersion{"1.2.1"}; }
        else if (GLEW_VERSION_1_2) { temporaryVersion = IEVersion{"1.2.0"}; }
        else if (GLEW_VERSION_1_1) { temporaryVersion = IEVersion{"1.1.0"}; }
        // If none of these is active then GLEW something is wrong.
        IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "No OpenGL version was found!");
    }
    #endif
    return temporaryVersion;
}

