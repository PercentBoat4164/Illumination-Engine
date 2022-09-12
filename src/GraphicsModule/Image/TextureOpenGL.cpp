#include "TextureOpenGL.hpp"

const std::unordered_map<IE::Graphics::Texture::Filter, GLuint> IE::Graphics::detail::TextureOpenGL::filter{
		{IE_TEXTURE_FILTER_NEAREST, GL_NEAREST},
		{IE_TEXTURE_FILTER_LINEAR,  GL_LINEAR},
		{IE_TEXTURE_FILTER_CUBIC,   GL_CUBIC_IMG}
};
