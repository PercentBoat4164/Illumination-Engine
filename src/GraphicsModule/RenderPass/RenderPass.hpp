#pragma once

#include "GraphicsModule/RenderPass/Framebuffer.hpp"
#include "GraphicsModule/RenderPass/Subpass.hpp"

#include <map>
#include <memory>
#include <vector>

namespace IE::Graphics {
class RenderPass {
public:
    enum Presets {
        IE_GRAPHICS_RENDER_PASS_PRESET_COLOR = 0x0,
    };

    RenderPass(IE::Graphics::RenderEngine *t_engineLink, Presets t_preset);
};
}  // namespace IE::Graphics