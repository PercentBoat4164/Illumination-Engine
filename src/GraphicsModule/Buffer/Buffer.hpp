#pragma once

#include "Core/MultiDimensionalVector.hpp"

#include <GL/glew.h>
#include <vulkan/vulkan.hpp>

// System dependencies
#include <cstddef>
#include <memory>
#include <mutex>

namespace IE::Graphics {
class RenderEngine;

class Buffer {
public:
    enum Status {
        IE_BUFFER_STATUS_UNINITIALIZED = 0x0,
        IE_BUFFER_STATUS_CREATED       = 0x1,
    };

    enum Type {
        IE_BUFFER_TYPE_NULL            = 0x0,
        IE_BUFFER_TYPE_INDEX_BUFFER    = 0x1,
        IE_BUFFER_TYPE_VERTEX_BUFFER   = 0x2,
        IE_BUFFER_TYPE_UNIFORM_BUFFER  = 0x3,
        IE_BUFFER_TYPE_STORAGE_BUFFER  = 0x4,
        IE_BUFFER_TYPE_INSTANCE_BUFFER = 0x5,
        IE_BUFFER_TYPE_STAGING_BUFFER  = 0x6
    };

    Status                      m_status{IE_BUFFER_STATUS_UNINITIALIZED};
    Type                        type{};
    std::shared_ptr<std::mutex> m_mutex{};
    IE::Graphics::RenderEngine *m_linkedRenderEngine{};

    Buffer(IE::Graphics::RenderEngine *t_engineLink) : m_linkedRenderEngine(t_engineLink) {
    }

    virtual ~Buffer() = default;

    static std::shared_ptr<IE::Graphics::Buffer> create(IE::Graphics::RenderEngine *t_engineLink);

    void createBuffer(Type t_type, uint64_t t_flags, void *t_data, size_t t_dataSize);

    void destroyBuffer();

    virtual GLenum getGLBuffer() = 0;

    virtual VkBuffer getVkBuffer() = 0;

protected:
    virtual bool _createBuffer(Type t_type, uint64_t t_flags, void *t_data, size_t t_dataSize) = 0;

    virtual bool _destroyBuffer() = 0;
};
}  // namespace IE::Graphics