/* Include this file's header. */
#include "IERenderableSettings.hpp"

/* Include dependencies within this module. */
#include "IEAPI.hpp"


uint32_t IEPolygonMode(enum IEPolygonMode polygonMode, const IEAPI &api) {
	if (api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		return IEPolygonModesVulkan[polygonMode];
	}
	if (api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		return IEPolygonModesOpenGL[polygonMode];
	}
	return GL_FILL;
}

uint32_t IECullMode(enum IECullMode cullMode, const IEAPI &api) {
	if (api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		return IECullModesVulkan[cullMode];
	}
	if (api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		return IECullModesOpenGL[cullMode];
	}
	return GL_NONE;
}
