#include "BufferOpenGL.hpp"

#include "Core/Core.hpp"
#include "RenderEngine.hpp"

bool IE::Graphics::detail::BufferOpenGL::_destroyBuffer() {
    glDeleteBuffers(1, &m_id);
    GLenum result{glGetError()};
    if (result != GL_NO_ERROR) {
        m_linkedRenderEngine->getLogger().log(
          IE::Graphics::RenderEngine::makeErrorMessageReporter<GLenum>(
            {GL_INVALID_ENUM, GL_INVALID_VALUE, GL_INVALID_OPERATION, GL_OUT_OF_MEMORY},
            {"GL_INVALID_ENUM", "GL_INVALID_VALUE", "GL_INVALID_OPERATION", "GL_OUT_OF_MEMORY"},
            "BufferOpenGL->_destroyBuffer->glDeleteBuffers",
            __FILE__,
            __LINE__,
            "https://docs.gl/gl2/glDeleteBuffers"
          )(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
        return false;
    }
    return true;
}

bool IE::Graphics::detail::BufferOpenGL::_createBuffer(
  IE::Graphics::Buffer::Type t_type,
  uint64_t                   t_flags,
  void                      *t_data,
  size_t                     t_dataSize
) {
    GLenum slot = getBufferSlot(t_type);
    glGenBuffers(1, &m_id);
    glBindBuffer(slot, m_id);
    glBufferData(slot, static_cast<GLsizeiptr>(t_dataSize), t_data, GL_STATIC_DRAW);
    GLenum result{glGetError()};
    if (result != GL_NO_ERROR) {
        m_linkedRenderEngine->getLogger().log(
          IE::Graphics::RenderEngine::makeErrorMessageReporter<GLenum>(
            {GL_INVALID_ENUM, GL_INVALID_VALUE, GL_INVALID_OPERATION, GL_OUT_OF_MEMORY},
            {"GL_INVALID_ENUM", "GL_INVALID_VALUE", "GL_INVALID_OPERATION", "GL_OUT_OF_MEMORY"},
            "BufferOpenGL->_createBuffer->glBufferData",
            __FILE__,
            __LINE__,
            "https://docs.gl/gl2/glBufferData"
          )(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
        return false;
    }
    glBindBuffer(slot, 0);
    return true;
}

GLenum IE::Graphics::detail::BufferOpenGL::getBufferSlot(IE::Graphics::Buffer::Type t_type) {
    return m_bufferSlot[t_type];
}

const GLenum IE::Graphics::detail::BufferOpenGL::m_bufferSlot[]{
  GL_ARRAY_BUFFER,
  GL_ELEMENT_ARRAY_BUFFER,
  GL_ARRAY_BUFFER,
  GL_ARRAY_BUFFER,
  GL_ARRAY_BUFFER,
  GL_ARRAY_BUFFER};