#pragma once

#include "Texture.hpp"
#include "ImageOpenGL.hpp"
#include <unordered_map>

namespace IE::Graphics::detail {
	class TextureOpenGL : virtual public IE::Graphics::Texture, virtual public IE::Graphics::detail::ImageOpenGL {
	public:
		static const std::unordered_map<Filter, GLuint> filter;
		
	};
}
