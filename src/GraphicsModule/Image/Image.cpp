#include "Image.hpp"

#include "ImageOpenGL.hpp"
#include "ImageVulkan.hpp"
#include "RenderEngine.hpp"

#include <utility>

/**
 * @brief Basic private 'constructor' for the Status enum.
 * @details This is used to create IE::Graphics::Image::Locations from the uint8_ts that are used to perform the
 * binary operators. Before converting the t_location to an IE::Graphics::Image::Status, validity checks are
 *performed to ensure that the t_location does not represent a physically impossible scenario.
 * @param t_location The value that the new Status should take on.
 * @return A verified IE::Graphics::Image::Status.
 **/
constexpr IE::Graphics::Image::Location Location(uint8_t t_location) {
    if (((t_location & IE::Graphics::Image::Location::IE_IMAGE_LOCATION_NONE) != 0) && (t_location & ~IE::Graphics::Image::Location::IE_IMAGE_LOCATION_NONE) > IE::Graphics::Image::Location::IE_IMAGE_LOCATION_NULL)
        throw std::logic_error("IE_IMAGE_LOCATION_NONE mix error!");
    return static_cast<IE::Graphics::Image::Location>(t_location);
}

/**
 * @brief An implementation of the OR operator for the IE::Graphics::Image::Status enum.
 * @param t_first Operand.
 * @param t_second Operand.
 * @return t_first | t_second.
 */
constexpr IE::Graphics::Image::Location
operator|(IE::Graphics::Image::Location t_first, IE::Graphics::Image::Location t_second) {
    return Location(static_cast<uint8_t>(t_first) | static_cast<uint8_t>(t_second));
}

/**
 * @brief An implementation of the AND operator for the IE::Graphics::Image::Status enum.
 * @param t_first Operand.
 * @param t_second Operand.
 * @return t_first & t_second.
 */
constexpr IE::Graphics::Image::Location
operator&(IE::Graphics::Image::Location t_first, IE::Graphics::Image::Location t_second) noexcept {
    return Location(static_cast<uint8_t>(t_first) & static_cast<uint8_t>(t_second));
}

/**
 * @brief An implementation of the XOR operator for the IE::Graphics::Image::Status enum.
 * @param t_first Operand.
 * @param t_second Operand.
 * @return t_first ^ t_second.
 */
constexpr IE::Graphics::Image::Location
operator^(IE::Graphics::Image::Location t_first, IE::Graphics::Image::Location t_second) noexcept {
    return Location(static_cast<uint8_t>(t_first) ^ static_cast<uint8_t>(t_second));
}

/**
 * @brief An implementation of the NOT operator for the IE::Graphics::Image::Status enum.
 * @details This operator does not necessarily return a valid location. It may return a location that has both the
 * IE_IMAGE_LOCATION_NONE and IE_IMAGE_LOCATION_SYSTEM bits set for example
 * @param t_first Operand.
 * @return ~t_first
 */
constexpr IE::Graphics::Image::Location operator~(IE::Graphics::Image::Location t_first) noexcept {
    return static_cast<IE::Graphics::Image::Location>(~static_cast<uint8_t>(t_first));
}

//
// ===================== Implementation of IE::Graphics::Image =====================
//

IE::Graphics::Image::Image() noexcept :
        m_components{0},
        m_location{IE_IMAGE_LOCATION_NULL} {
}

IE::Graphics::Image &IE::Graphics::Image::operator=(const IE::Graphics::Image &t_other) {
    std::unique_lock<std::mutex> lock(*m_mutex);
    std::unique_lock<std::mutex> otherLock(*t_other.m_mutex);
    if (&t_other != this) {
        if (m_linkedRenderEngine != t_other.m_linkedRenderEngine) {
            IE::Core::MultiDimensionalVector<unsigned char> *temp{
              new IE::Core::MultiDimensionalVector<unsigned char>()};
            otherLock.release();
            t_other._getImageData();
            otherLock.lock();
            lock.release();
            _createImage(*temp);
            lock.lock();
        }
        m_components = t_other.m_components;
        m_location   = t_other.m_location;
        m_data       = t_other.m_data;
        m_dimensions = t_other.m_dimensions;
    }
    return *this;
}

IE::Graphics::Image &IE::Graphics::Image::operator=(IE::Graphics::Image &&t_other) noexcept {
    std::unique_lock<std::mutex> lock(*m_mutex);
    std::unique_lock<std::mutex> otherLock(*t_other.m_mutex);
    if (&t_other != this) {
        if (m_linkedRenderEngine != t_other.m_linkedRenderEngine) {
            IE::Core::MultiDimensionalVector<unsigned char> *temp{
              new IE::Core::MultiDimensionalVector<unsigned char>()};
            otherLock.release();
            t_other._getImageData();
            otherLock.lock();
            lock.release();
            _createImage(*temp);
            lock.lock();
        }
        m_components = std::exchange(t_other.m_components, 0);
        m_location   = std::exchange(t_other.m_location, IE_IMAGE_LOCATION_NULL);
        m_data       = std::exchange(t_other.m_data, IE::Core::MultiDimensionalVector<unsigned char>{});
        m_dimensions = std::exchange(t_other.m_dimensions, {});
    }
    return *this;
}

template<typename... Args>
IE::Graphics::Image *IE::Graphics::Image::create(IE::Graphics::RenderEngine *t_engineLink, Args... t_dimensions) {
    if (t_engineLink->getAPI().name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        return static_cast<IE::Graphics::Image *>(
          new IE::Graphics::detail::ImageVulkan(t_engineLink, t_dimensions...)
        );
    } else if (t_engineLink->getAPI().name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
        return static_cast<IE::Graphics::Image *>(
          new IE::Graphics::detail::ImageOpenGL(t_engineLink, t_dimensions...)
        );
    }
    IE::Core::Core::getInst().getLogger()->log(
      "failed to create image because render engine is using neither Vulkan or OpenGL.",
      IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
    );
    return nullptr;
}

template<typename... Args>
unsigned char IE::Graphics::Image::operator[](Args... t_args) {
    return m_data[std::forward<Args...>(t_args...)];
}

template<typename... Args>
IE::Graphics::Image::Image(IE::Graphics::RenderEngine *t_engineLink, Args... t_dimensions) :
        m_components{4},
        m_location{IE_IMAGE_LOCATION_SYSTEM},
        m_dimensions(),
        m_data{IE::Core::MultiDimensionalVector<unsigned char>(t_dimensions...)},
        m_linkedRenderEngine{t_engineLink},
        m_mutex() {
}

IE::Graphics::Image::Location IE::Graphics::Image::getLocation() const {
    return m_location;
}

IE::Core::MultiDimensionalVector<unsigned char> IE::Graphics::Image::getData() const {
    return m_data;
}

void IE::Graphics::Image::setData(const IE::Core::MultiDimensionalVector<unsigned char> &t_data) {
    std::unique_lock<std::mutex> lock(*m_mutex);
    if (m_location & IE_IMAGE_LOCATION_NONE) {  // Nowhere for image data to go.
        // warn and return
        IE::Core::Core::getInst().getLogger()->log(
          "Attempted to set data to an image stored in IE_IMAGE_LOCATION_NONE!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
        return;
    }
    if (m_location & IE_IMAGE_LOCATION_SYSTEM) m_data = t_data;
    if (m_location & IE_IMAGE_LOCATION_VIDEO) {
        if (m_dimensions == t_data.getDimensions()) {
            lock.release();
            _setImageData(t_data);
            lock.lock();
        } else {
            lock.release();
            _destroyImage();
            _createImage(t_data);
            lock.lock();
        }
    }
}
