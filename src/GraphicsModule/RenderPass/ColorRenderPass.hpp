#pragma once

#include "RenderPass.hpp"

namespace IE::Graphics {
	class ColorRenderPass : public IE::Graphics::RenderPass {
	public:
		ColorRenderPass setFramebuffer(IE::Graphics::Framebuffer t_framebuffer);
	};
}