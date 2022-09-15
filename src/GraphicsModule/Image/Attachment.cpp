#include "Attachment.hpp"

#include "AttachmentVulkan.hpp"
#include "AttachmentOpenGL.hpp"

#include "IERenderEngine.hpp"

template<typename... Args>
std::unique_ptr<IE::Graphics::Attachment>
IE::Graphics::Attachment::create(const std::weak_ptr<IERenderEngine> &t_engineLink, IE::Graphics::Attachment::Preset t_preset, Args... t_args) {
	if (t_engineLink.lock()->API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		return std::unique_ptr<IE::Graphics::Attachment>{
				static_cast<IE::Graphics::Attachment *>(new IE::Graphics::detail::AttachmentVulkan(t_engineLink, t_preset, t_args...))};
	} else if (t_engineLink.lock()->API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		return std::unique_ptr<IE::Graphics::Attachment>{
				static_cast<IE::Graphics::Attachment *>(new IE::Graphics::detail::AttachmentOpenGL(t_engineLink, t_preset, t_args...))};
	}
	t_engineLink.lock()->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR,
											  "failed to create attachment because render engine is using neither Vulkan or OpenGL.");
	return nullptr;
}
