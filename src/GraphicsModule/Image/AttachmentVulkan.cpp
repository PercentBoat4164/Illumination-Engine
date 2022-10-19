#include "AttachmentVulkan.hpp"

template<typename... Args>
IE::Graphics::detail::AttachmentVulkan::AttachmentVulkan(
  IE::Graphics::RenderEngine      *t_engineLink,
  IE::Graphics::Attachment::Preset t_preset,
  Args... t_args
) : IE::Graphics::Image(t_engineLink, t_preset, t_args...) {
}
