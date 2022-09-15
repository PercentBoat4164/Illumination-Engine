#pragma once

#include "Image.hpp"

namespace IE::Graphics {
	class Attachment : virtual public IE::Graphics::Image {
	public:
		enum Preset {
			IE_ATTACHMENT_PRESET_COLOR_OUTPUT,
			IE_ATTACHMENT_PRESET_COLOR_OUTPUT_MULTISAMPLE,
			IE_ATTACHMENT_PRESET_SHADOW_MAP,
			IE_ATTACHMENT_PRESET_SHADOW_MAP_MULTISAMPLE
		};
		template<typename... Args>
		static std::unique_ptr<IE::Graphics::Attachment>
		create(const std::weak_ptr<IERenderEngine> &t_engineLink, IE::Graphics::Attachment::Preset t_preset = IE_ATTACHMENT_PRESET_COLOR_OUTPUT,
			   Args... t_args);
	};
}