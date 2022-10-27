#include "Image.hpp"

#include "Core/LogModule/IELogger.hpp"
#include "RenderEngine.hpp"

void IE::Graphics::Image::createImage(
  IE::Graphics::Image::Type                        t_type,
  uint64_t                                         t_flags,
  IE::Core::MultiDimensionalVector<unsigned char> &t_data
) {
    std::unique_lock<std::mutex> lock(*m_mutex);
    if (m_status == IE_IMAGE_STATUS_CREATED) {
        m_linkedRenderEngine->getLogger().log(
          "Attempted to create an image that already exists!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
        return;
    }
    if (_createImage(t_type, t_flags, t_data)) m_status = IE_IMAGE_STATUS_CREATED;
}

void IE::Graphics::Image::destroyImage() {
    std::unique_lock<std::mutex> lock(*m_mutex);
    if (m_status == IE_IMAGE_STATUS_UNINITIALIZED) {
        m_linkedRenderEngine->getLogger().log(
          "Attempted to destroy an image that does not exist!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
        return;
    }
    if (_destroyImage()) m_status = IE_IMAGE_STATUS_UNINITIALIZED;
}
