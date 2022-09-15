#pragma once

#include "RenderPass.hpp"

#include "Image/Attachment.hpp"

#include <string>
#include <memory>
#include <array>

namespace IE::Graphics {
	class ShadowRenderPass : public IE::Graphics::RenderPass {
	public:
		ShadowRenderPass setMapCount(size_t t_maps);
		
		std::vector<std::shared_ptr<IE::Graphics::Image>> getMaps();
		
		void build();
	};
}
