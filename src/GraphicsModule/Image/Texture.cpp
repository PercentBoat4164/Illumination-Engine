#include "Texture.hpp"

#include "TextureVulkan.hpp"
#include "ImageOpenGL.hpp"

#include "IERenderEngine.hpp"

IE::Graphics::Texture::Texture() noexcept:
		m_mipLodBias{},
		m_anisotropyLevel{},
		m_magnificationFilter{},
		m_minificationFilter{},
		u{},
		v{},
		w{} {}

template<typename... Args>
IE::Graphics::Texture::Texture(const std::weak_ptr<IERenderEngine> &t_engineLink, Args... args) :
		Image(t_engineLink, args...),
		m_mipLodBias{},
		m_anisotropyLevel{},
		m_magnificationFilter{},
		m_minificationFilter{},
		u{},
		v{},
		w{} {}

IE::Graphics::Texture &IE::Graphics::Texture::operator=(IE::Graphics::Texture &&t_other) noexcept {
	std::unique_lock<std::mutex> lock(*m_mutex);
	std::unique_lock<std::mutex> otherLock(*m_mutex);
	if (&t_other != this) {
		m_mipLodBias = std::exchange(t_other.m_mipLodBias, 0);
		m_anisotropyLevel = std::exchange(t_other.m_anisotropyLevel, 0);
		m_magnificationFilter = std::exchange(t_other.m_magnificationFilter, {});
		m_minificationFilter = std::exchange(t_other.m_minificationFilter, {});
		u = std::exchange(t_other.u, {});
		v = std::exchange(t_other.v, {});
		w = std::exchange(t_other.w, {});
	}
	return *this;
}

IE::Graphics::Texture &IE::Graphics::Texture::operator=(const IE::Graphics::Texture &t_other) {
	std::unique_lock<std::mutex> lock(*m_mutex);
	std::unique_lock<std::mutex> otherLock(*m_mutex);
	if (&t_other != this) {
		m_mipLodBias = t_other.m_mipLodBias;
		m_anisotropyLevel = t_other.m_anisotropyLevel;
		m_magnificationFilter = t_other.m_magnificationFilter;
		m_minificationFilter = t_other.m_minificationFilter;
		u = t_other.u;
		v = t_other.v;
		w = t_other.w;
	}
	return *this;
}

template<typename... Args>
std::unique_ptr<IE::Graphics::Texture> IE::Graphics::Texture::create(const std::weak_ptr<IERenderEngine> &t_engineLink, Args... t_dimensions) {
	if (t_engineLink.lock()->API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		return std::unique_ptr<IE::Graphics::Texture>{
				static_cast<IE::Graphics::Texture *>(new IE::Graphics::detail::TextureVulkan(t_engineLink, t_dimensions...))};
	} else if (t_engineLink.lock()->API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		return std::unique_ptr<IE::Graphics::Texture>{
				static_cast<IE::Graphics::Texture *>(new IE::Graphics::detail::TextureVulkan(t_engineLink, t_dimensions...))};
	}
	t_engineLink.lock()->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR,
											  "failed to create image because render engine is using neither Vulkan or OpenGL.");
	return nullptr;
}
