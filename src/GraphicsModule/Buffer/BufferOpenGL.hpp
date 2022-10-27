#pragma once

#include "Buffer.hpp"

#include <GL/glew.h>

namespace IE::Graphics::detail {
class BufferOpenGL : public IE::Graphics::Buffer {
public:
    GLuint m_id{};

protected:
    bool _createBuffer(Type t_type, uint64_t t_flags, void *t_data, size_t t_dataSize) override;
    bool _destroyBuffer() override;

    static GLenum getBufferSlot(Type t_type);

private:
    static const GLenum m_bufferSlot[];
};
}  // namespace IE::Graphics::detail
