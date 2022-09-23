#pragma once

#include "Image.hpp"

#define GLEW_IMPLEMENTATION
#include "include/GL/glew.h"

namespace IE::Graphics::detail {
class ImageOpenGL : virtual public IE::Graphics::Image {
public:
    GLuint m_id;
    GLint  m_format;
    GLuint m_type;
    size_t m_size;  // Specifies the size of the image in OpenGL memory

    ImageOpenGL() noexcept;

    template<typename... Args>
    explicit ImageOpenGL(const std::weak_ptr<IE::Graphics::RenderEngine> &t_engineLink, Args... t_dimensions);

    ImageOpenGL &operator=(ImageOpenGL &&t_other) noexcept;

    ImageOpenGL &operator=(const ImageOpenGL &t_other);

    ~ImageOpenGL() override;

    [[nodiscard]] uint8_t getBytesInFormat() const override;

    void setLocation(Location t_location) override;

protected:
    bool _createImage(const IE::Core::MultiDimensionalVector<unsigned char> &t_data) override;

    void _destroyImage() final;

    void _getImageData(IE::Core::MultiDimensionalVector<unsigned char> *pVector) const override;

    void _setImageData(const IE::Core::MultiDimensionalVector<unsigned char> &t_data) override;
};
}  // namespace IE::Graphics::detail