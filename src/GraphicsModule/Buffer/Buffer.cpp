#include "Buffer.hpp"

#include "Core/LogModule/IELogger.hpp"
#include "RenderEngine.hpp"

void IE::Graphics::Buffer::createBuffer(
  IE::Graphics::Buffer::Type t_type,
  uint64_t                   t_flags,
  void                      *t_data,
  size_t                     t_dataSize
) {
    std::unique_lock<std::mutex> lock(*m_mutex);
    if (m_status == IE_BUFFER_STATUS_CREATED) {
        m_linkedRenderEngine->getLogger().log(
          "Attempted to create a buffer that already exists!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
        return;
    }
    if (_createBuffer(t_type, t_flags, t_data, t_dataSize)) m_status = IE_BUFFER_STATUS_CREATED;
}

void IE::Graphics::Buffer::destroyBuffer() {
    std::unique_lock<std::mutex> lock(*m_mutex);
    if (m_status == IE_BUFFER_STATUS_UNINITIALIZED) {
        m_linkedRenderEngine->getLogger().log(
          "Attempted to destroy a buffer that does not exist!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
        return;
    }
    if (_destroyBuffer()) m_status = IE_BUFFER_STATUS_UNINITIALIZED;
}
