#pragma once

#define GLFW_INCLUDE_VULKAN  // Needed for glfwCreateWindowSurface
#include <GLFW/glfw3.h>

#include <cstdint>

enum IEPolygonMode {
    IE_POLYGON_MODE_FILL = 0x0,
    IE_POLYGON_MODE_LINE = 0x1,
    IE_POLYGON_MODE_POINTS = 0x2
};

const std::vector<uint32_t> IEPolygonModesOpenGL{GL_FILL, GL_LINE, GL_POINTS};

const std::vector<uint32_t> IEPolygonModesVulkan{VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT};


uint32_t IEPolygonMode(IEPolygonMode polygonMode, const IEGraphicsLink::IEAPI& api) {
    if (api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        return IEPolygonModesVulkan[polygonMode];
    }
    if (api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
        return IEPolygonModesOpenGL[polygonMode];
    }
    else return GL_FILL;
}

struct IERenderableSettings {
public:
    uint32_t mipLevels{};
    uint32_t polygonMode{IE_POLYGON_MODE_FILL};
};