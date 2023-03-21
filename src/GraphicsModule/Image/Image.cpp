#include "Image.hpp"

#include "Core/LogModule/Logger.hpp"
#include "RenderEngine.hpp"

void IE::Graphics::Image::createImage(
  IE::Graphics::Image::Preset t_preset,
  uint64_t                    t_flags,
  std::vector<unsigned char> &t_data
) {
    std::lock_guard<std::mutex> lock(*m_mutex);
    if (m_status == IE_IMAGE_STATUS_CREATED) {
        m_linkedRenderEngine->getLogger().log(
          "Attempted to create an image that already exists!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
        return;
    }
    if (_createImage(t_preset, t_flags, t_data)) m_status = IE_IMAGE_STATUS_CREATED;
}

void IE::Graphics::Image::destroyImage() {
    std::lock_guard<std::mutex> lock(*m_mutex);
    if (m_status == IE_IMAGE_STATUS_UNINITIALIZED) {
        m_linkedRenderEngine->getLogger().log(
          "Attempted to destroy an image that does not exist!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
        return;
    }
    if (_destroyImage()) m_status = IE_IMAGE_STATUS_UNINITIALIZED;
}
