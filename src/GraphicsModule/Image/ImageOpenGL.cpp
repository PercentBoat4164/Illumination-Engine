#include "ImageOpenGL.hpp"

#include "RenderEngine.hpp"

/**@todo Implement me! */
uint8_t IE::Graphics::detail::ImageOpenGL::getBytesInFormat() const {
    return 0x8;  ///@todo Fill this in to hold correct data.
}

IE::Graphics::detail::ImageOpenGL &IE::Graphics::detail::ImageOpenGL::operator=(const ImageOpenGL &t_other) {
    m_id     = t_other.m_id;
    m_format = t_other.m_format;
    m_type   = t_other.m_type;
    m_size   = t_other.m_size;
    return *this;
}

IE::Graphics::detail::ImageOpenGL &IE::Graphics::detail::ImageOpenGL::operator=(ImageOpenGL &&t_other) noexcept {
    m_id     = std::exchange(t_other.m_id, 0);
    m_format = std::exchange(t_other.m_format, 0);
    m_type   = std::exchange(t_other.m_type, 0);
    m_size   = std::exchange(t_other.m_size, 0);
    return *this;
}

IE::Graphics::detail::ImageOpenGL::ImageOpenGL() noexcept : m_id{}, m_format{}, m_type{}, m_size{} {
}

template<typename... Args>
IE::Graphics::detail::ImageOpenGL::ImageOpenGL(
  const std::weak_ptr<IE::Graphics::RenderEngine> &t_engineLink,
  Args... t_dimensions
) :
        Image(t_engineLink, t_dimensions...),
        m_id{},
        m_format{},
        m_type{},
        m_size{} {
}

void IE::Graphics::detail::ImageOpenGL::setLocation(IE::Graphics::Image::Location t_location) {
    std::unique_lock<std::mutex> lock(*m_mutex);
    if (!m_data.empty() || m_id != 0 && t_location != m_location) {  // If data already exists somewhere and the
                                                                     // new location is not the same as the old
        if (m_location & IE_IMAGE_LOCATION_SYSTEM && t_location & IE_IMAGE_LOCATION_VIDEO) {  // system -> video
            // Create image with specified intrinsics
            lock.release();
            _createImage(m_data);
            lock.lock();
        } else if (m_location & IE_IMAGE_LOCATION_VIDEO && t_location & IE_IMAGE_LOCATION_SYSTEM) {  // video ->
                                                                                                     // system
            // Copy data to system
            lock.release();
            _getImageData(&m_data);
            lock.lock();
        }
        if (m_location & IE_IMAGE_LOCATION_SYSTEM && ~t_location & IE_IMAGE_LOCATION_SYSTEM) {  // system -> none
            // Clear image from system
            m_data.clear();
        }
        if (m_location & IE_IMAGE_LOCATION_VIDEO && ~t_location & IE_IMAGE_LOCATION_VIDEO) {  // video -> none
            // Destroy video memory copy of image
            lock.release();
            _destroyImage();
            lock.lock();
        }
    }
    m_location = t_location;
}

bool IE::Graphics::detail::ImageOpenGL::_createImage(const IE::Core::MultiDimensionalVector<unsigned char> &t_data
) {
    std::unique_lock<std::mutex> lock(*m_mutex);
    m_dimensions = t_data.getDimensions();
    if (t_data.getDimensionality() > 3 || t_data.empty()) {
        IE::Core::Core::getInst().getLogger()->log(
          "Images with more than 3 dimensions or less than 1 cannot be put on the GPU with OpenGL.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }
    glGenTextures(1, &m_id);
    lock.release();
    _setImageData(t_data);
    lock.lock();
    return m_id != 0;
}

void IE::Graphics::detail::ImageOpenGL::_destroyImage() {
    std::unique_lock<std::mutex> lock(*m_mutex);
    glDeleteTextures(1, &m_id);
    m_dimensions = {0};
}

void IE::Graphics::detail::ImageOpenGL::_getImageData(IE::Core::MultiDimensionalVector<unsigned char> *t_pData
) const {
    std::unique_lock<std::mutex> lock(*m_mutex);
    t_pData->resize(m_dimensions);
    glBindTexture(m_type, m_id);
    glGetTexImage(m_type, 0, m_format, GL_UNSIGNED_BYTE, t_pData->data());
    glBindTexture(m_type, 0);
}

void IE::Graphics::detail::ImageOpenGL::_setImageData(const IE::Core::MultiDimensionalVector<unsigned char> &t_data
) {
    std::unique_lock<std::mutex> lock(*m_mutex);
    if (std::accumulate(m_dimensions.begin(), m_dimensions.end(), 1, std::multiplies()) != t_data.size()) {
        lock.release();
        _destroyImage();
        _createImage(t_data);
        lock.lock();
    }
    glBindTexture(m_type, m_id);
    if (m_data.getDimensionality() == 1) {
        glTexImage1D(
          GL_TEXTURE_1D,
          0,
          m_format,
          (GLsizei) t_data.getDimensions()[0],
          0,
          m_components > 3 ? GL_RGBA : GL_RGB,
          GL_UNSIGNED_BYTE,
          t_data.data()
        );
    } else if (m_data.getDimensionality() == 2) {
        glTexImage2D(
          GL_TEXTURE_2D,
          0,
          m_format,
          (GLsizei) t_data.getDimensions()[0],
          (GLsizei) t_data.getDimensions()[1],
          0,
          m_components > 3 ? GL_RGBA : GL_RGB,
          GL_UNSIGNED_BYTE,
          t_data.data()
        );
    } else if (m_data.getDimensionality() == 3) {
        glTexImage3D(
          GL_TEXTURE_3D,
          0,
          m_format,
          (GLsizei) t_data.getDimensions()[0],
          (GLsizei) t_data.getDimensions()[1],
          (GLsizei) t_data.getDimensions()[2],
          0,
          m_components > 3 ? GL_RGBA : GL_RGB,
          GL_UNSIGNED_BYTE,
          t_data.data()
        );
    }
    glBindTexture(m_type, 0);
}

IE::Graphics::detail::ImageOpenGL::~ImageOpenGL() {
    _destroyImage();
}