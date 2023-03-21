#pragma once

#include <memory>
#include <mutex>
#include <vector>

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
    std::shared_ptr<std::mutex> m_mutex{std::make_shared<std::mutex>()};
    IE::Graphics::RenderEngine *m_linkedRenderEngine{};

    explicit Image(IE::Graphics::RenderEngine *t_engineLink) : m_linkedRenderEngine(t_engineLink) {
    }

    virtual void createImage(Preset t_preset, uint64_t t_flags, std::vector<unsigned char> &t_data);

    virtual void destroyImage();

protected:
    virtual bool _createImage(Preset t_preset, uint64_t t_flags, std::vector<unsigned char> &t_data) = 0;

    virtual bool _destroyImage() = 0;
};
}  // namespace IE::Graphics
