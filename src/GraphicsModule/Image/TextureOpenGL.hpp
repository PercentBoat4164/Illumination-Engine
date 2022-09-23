#pragma once

#include "Texture.hpp"
#include "ImageOpenGL.hpp"
#include <unordered_map>

namespace IE::Graphics::detail {
	class TextureOpenGL : virtual public IE::Graphics::Texture, virtual public IE::Graphics::detail::ImageOpenGL {
	public:
		GLuint m_unit;  // The OpenGL texture unit to use for this texture.
		
		TextureOpenGL() noexcept;
		
		template<typename... Args>
		explicit TextureOpenGL(const std::weak_ptr<IE::Graphics::RenderEngine> &t_engineLink, Args... t_args);
		
		TextureOpenGL &operator=(TextureOpenGL &&t_other) noexcept;
		
		TextureOpenGL &operator=(const TextureOpenGL &t_other);
		
		bool _createSampler() override;
		
		static const std::unordered_map<Filter, GLint> filter;
		
		static const std::unordered_map<AddressMode, GLint> addressMode;
	};
}
