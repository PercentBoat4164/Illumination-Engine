#include "TextureOpenGL.hpp"

bool IE::Graphics::detail::TextureOpenGL::_createSampler() {
    std::unique_lock<std::mutex> lock(*m_mutex);
    glActiveTexture(m_unit);
    glBindTexture(m_type, m_id);
    glTexParameteri(m_type, GL_TEXTURE_WRAP_R, addressMode.at(u));
    glTexParameteri(m_type, GL_TEXTURE_WRAP_S, addressMode.at(v));
    glTexParameteri(m_type, GL_TEXTURE_WRAP_T, addressMode.at(w));
    glTexParameteri(m_type, GL_TEXTURE_MIN_FILTER, filter.at(m_minificationFilter));
    glTexParameteri(m_type, GL_TEXTURE_MAG_FILTER, filter.at(m_magnificationFilter));
    glBindTexture(m_type, 0);
    glActiveTexture(0);
    return true;
}

IE::Graphics::detail::TextureOpenGL::TextureOpenGL() noexcept : m_unit{} {
}

const std::unordered_map<IE::Graphics::Texture::Filter, GLint> IE::Graphics::detail::TextureOpenGL::filter{
  {IE::Graphics::Texture::IE_TEXTURE_FILTER_NEAREST, GL_NEAREST  },
  {IE::Graphics::Texture::IE_TEXTURE_FILTER_LINEAR,  GL_LINEAR   },
  {IE::Graphics::Texture::IE_TEXTURE_FILTER_CUBIC,   GL_CUBIC_IMG}
};


const std::unordered_map<IE::Graphics::Texture::AddressMode, GLint>
  IE::Graphics::detail::TextureOpenGL::addressMode{
    {IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_REPEAT,               GL_REPEAT              },
    {IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_MIRRORED_REPEAT,      GL_MIRRORED_REPEAT     },
    {IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE,        GL_CLAMP_TO_EDGE       },
    {IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_CLAMP_TO_BORDER,      GL_CLAMP_TO_BORDER     },
    {IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE, GL_MIRROR_CLAMP_TO_EDGE}
};

template<typename... Args>
IE::Graphics::detail::TextureOpenGL::TextureOpenGL(
  const std::weak_ptr<IE::Graphics::RenderEngine> &t_engineLink,
  Args... t_args
) :
        Texture(t_engineLink, t_args...),
        m_unit{} {
}

IE::Graphics::detail::TextureOpenGL &IE::Graphics::detail::TextureOpenGL::operator=(TextureOpenGL &&t_other
) noexcept {
    if (&t_other != this) m_unit = std::exchange(t_other.m_unit, 0);
    return *this;
}

IE::Graphics::detail::TextureOpenGL &
IE::Graphics::detail::TextureOpenGL::operator=(const IE::Graphics::detail::TextureOpenGL &t_other) {
    if (&t_other != this) m_unit = t_other.m_unit;
    return *this;
}
