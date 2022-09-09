#include "ImageOpenGL.hpp"

/**@todo Implement me! */
uint8_t IE::ImageOpenGL::getBytesInFormat() const {
	return 0x8;
}

IE::ImageOpenGL &IE::ImageOpenGL::operator=(const ImageOpenGL &t_other) {
	///@note May be needed?
//	*(IE::Image *)this = (const IE::Image &)t_other;
	id = t_other.id;
	return *this;
}

IE::ImageOpenGL &IE::ImageOpenGL::operator=(ImageOpenGL && t_other) noexcept {
	id = std::exchange(t_other.id, 0);
	return *this;
}

IE::ImageOpenGL::ImageOpenGL() = default;

IE::ImageOpenGL::ImageOpenGL(const std::weak_ptr<IERenderEngine> &t_engineLink) : IE::Image(t_engineLink) {}

void IE::ImageOpenGL::setLocation(IE::Image::Location t_location) {
	if (!m_data.empty() || id != 0 && t_location != m_location) {  // If data already exists somewhere and the new location is not the same as the old
		if (m_location & IE_IMAGE_LOCATION_SYSTEM && t_location & IE_IMAGE_LOCATION_VIDEO) {  // system -> video
			// Create image with specified intrinsics
		} else if (m_location & IE_IMAGE_LOCATION_VIDEO && t_location & IE_IMAGE_LOCATION_SYSTEM) {  // video -> system
			// Copy data to system
		}
		if (m_location & IE_IMAGE_LOCATION_SYSTEM && ~t_location & IE_IMAGE_LOCATION_SYSTEM) {  // system -> none
			// Clear image from system
		}
		if (m_location & IE_IMAGE_LOCATION_VIDEO && ~t_location & IE_IMAGE_LOCATION_VIDEO) {  // video -> none
			// Destroy video memory copy of image
		}
	}
	m_location = t_location;
}

void IE::ImageOpenGL::setData(const IE::Core::MultiDimensionalVector<unsigned char> &t_data) {
	if (m_location & IE_IMAGE_LOCATION_NONE) {  // Nowhere for image data to go.
		// warn and return
	}
	if (m_location & IE_IMAGE_LOCATION_SYSTEM) {
		m_data = t_data;
	}
}