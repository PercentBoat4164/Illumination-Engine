#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IEAPI;

/* Include classes used as attributes or _function arguments. */
// External dependencies
#define GLEW_IMPLEMENTATION  // Must precede GLEW inclusion.
#include <GL/glew.h>

#include <vulkan/vulkan.h>

// System dependencies
#include <cstdint>
#include <vector>


enum IEPolygonMode {
    IE_POLYGON_MODE_FILL = 0x0,
    IE_POLYGON_MODE_LINE = 0x1,
    IE_POLYGON_MODE_POINTS = 0x2
};

const std::vector<uint32_t> IEPolygonModesOpenGL{GL_FILL, GL_LINE, GL_POINTS};

const std::vector<uint32_t> IEPolygonModesVulkan{VK_POLYGON_MODE_FILL, VK_POLYGON_MODE_LINE, VK_POLYGON_MODE_POINT};


uint32_t IEPolygonMode(IEPolygonMode polygonMode, const IEAPI& api);

enum IECullMode {
    IE_CULL_MODE_BACK = 0x0,
    IE_CULL_MODE_FRONT = 0x1,
    IE_CULL_MODE_FRONT_AND_BACK = 0x3,
    IE_CULL_MODE_NONE = 0x4
};

const std::vector<uint32_t> IECullModesOpenGL{GL_BACK, GL_FRONT, GL_FRONT_AND_BACK, GL_NONE};

const std::vector<uint32_t> IECullModesVulkan{VK_CULL_MODE_BACK_BIT, VK_CULL_MODE_FRONT_BIT, VK_CULL_MODE_FRONT_AND_BACK, VK_CULL_MODE_NONE};

uint32_t IECullMode(IECullMode cullMode, const IEAPI& api);

struct IERenderableSettings {
public:
    uint32_t mipLevels{};
    uint32_t polygonMode{IE_POLYGON_MODE_FILL};
};