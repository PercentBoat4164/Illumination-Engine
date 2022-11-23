#pragma once

#include "Core/MultiDimensionalVector.hpp"

#include <memory>
#include <mutex>

namespace IE::Graphics {
class RenderEngine;

class Image {
public:
    enum Status {
        IE_IMAGE_STATUS_UNINITIALIZED = 0x0,
        IE_IMAGE_STATUS_CREATED       = 0x1,
    };

    enum Type {
        IE_IMAGE_TYPE_IMAGE      = 0x0,
        IE_IMAGE_TYPE_TEXTURE    = 0x1,
        IE_IMAGE_TYPE_ATTACHMENT = 0x2,
    };

    enum Intent {
        IE_IMAGE_INTENT_COLOR = 0x0,
        IE_IMAGE_INTENT_DEPTH = 0x1,
    };

    enum Preset {
        IE_IMAGE_PRESET_CUSTOM            = 0x0,
        IE_IMAGE_PRESET_FRAMEBUFFER_COLOR = 0x1,
        IE_IMAGE_PRESET_FRAMEBUFFER_DEPTH = 0x2,
    };

    Status                      m_status{IE_IMAGE_STATUS_UNINITIALIZED};
    Preset                      m_preset;
    std::shared_ptr<std::mutex> m_mutex{};
    IE::Graphics::RenderEngine *m_linkedRenderEngine{};

    virtual void
    createImage(Preset t_preset, uint64_t t_flags, IE::Core::MultiDimensionalVector<unsigned char> &t_data);
    virtual bool
    _createImage(Preset t_preset, uint64_t t_flags, IE::Core::MultiDimensionalVector<unsigned char> &t_data) = 0;
    virtual void destroyImage();
    virtual bool _destroyImage() = 0;
};
}  // namespace IE::Graphics
