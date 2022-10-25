#include "BufferOpenGL.hpp"

#include "Core/Core.hpp"
#include "RenderEngine.hpp"

void IE::Graphics::detail::BufferOpenGL::_destroyBuffer() {
    glDeleteBuffers(1, &m_id);
}

void IE::Graphics::detail::BufferOpenGL::_createBuffer(
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
    switch (result) {
        case GL_INVALID_ENUM:
            m_linkedRenderEngine->getLogger().log(IE::Graphics::RenderEngine::makeErrorMessage(
              "GL_INVALID_ENUM",
              "glBufferData",
              __FILE__,
              __LINE__,
              "https://docs.gl/gl2/glBufferData"
            ));
            break;
        case GL_INVALID_VALUE:
            m_linkedRenderEngine->getLogger().log(IE::Graphics::RenderEngine::makeErrorMessage(
              "GL_INVALID_VALUE",
              "glBufferData",
              __FILE__,
              __LINE__,
              "https://docs.gl/gl2/glBufferData"
            ));
            break;
        case GL_INVALID_OPERATION:
            m_linkedRenderEngine->getLogger().log(IE::Graphics::RenderEngine::makeErrorMessage(
              "GL_INVALID_OPERATION",
              "glBufferData",
              __FILE__,
              __LINE__,
              "https://docs.gl/gl2/glBufferData"
            ));
            break;
        case GL_OUT_OF_MEMORY:
            m_linkedRenderEngine->getLogger().log(IE::Graphics::RenderEngine::makeErrorMessage(
              "GL_OUT_OF_MEMORY",
              "glBufferData",
              __FILE__,
              __LINE__,
              "https://docs.gl/gl2/glBufferData"
            ));
            break;
    }
    glBindBuffer(slot, 0);
    m_status = IE_BUFFER_STATUS_CREATED;
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