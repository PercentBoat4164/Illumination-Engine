#include "Image.hpp"

#include "ImageVulkan.hpp"
#include "ImageOpenGL.hpp"

/**
 * @brief Basic private 'constructor' for the Location enum.
 * @details This is used to create IE::Image::Locations from the uint8_ts that are used to perform the
 * binary operators. Before converting the t_location to an IE::Image::Location, validity checks are performed to ensure that the t_location does not
 * represent a physically impossible scenario.
 * @param t_location The value that the new Location should take on.
 * @return A verified IE::Image::Location.
 **/
constexpr IE::Image::Location Location(uint8_t t_location) {
	if (t_location & IE::Image::Location::IE_IMAGE_LOCATION_NONE && (t_location & ~IE::Image::Location::IE_IMAGE_LOCATION_NONE) > IE::Image::Location::IE_IMAGE_LOCATION_NULL)
		throw std::logic_error("IE_IMAGE_LOCATION_NONE mix error!");
	return (IE::Image::Location) t_location;
}

/**
 * @brief An implementation of the OR operator for the IE::Image::Location enum.
 * @param t_first Operand.
 * @param t_second Operand.
 * @return t_first | t_second.
 */
constexpr IE::Image::Location operator|(IE::Image::Location t_first, IE::Image::Location t_second) {
	return Location((uint8_t) t_first | (uint8_t) t_second);
}

/**
 * @brief An implementation of the AND operator for the IE::Image::Location enum.
 * @param t_first Operand.
 * @param t_second Operand.
 * @return t_first & t_second.
 */
constexpr IE::Image::Location operator&(IE::Image::Location t_first, IE::Image::Location t_second) noexcept {
	return Location((uint8_t) t_first & (uint8_t) t_second);
}

/**
 * @brief An implementation of the XOR operator for the IE::Image::Location enum.
 * @param t_first Operand.
 * @param t_second Operand.
 * @return t_first ^ t_second.
 */
constexpr IE::Image::Location operator^(IE::Image::Location t_first, IE::Image::Location t_second) noexcept {
	return Location((uint8_t) t_first ^ (uint8_t) t_second);
}

/**
 * @brief An implementation of the NOT operator for the IE::Image::Location enum.
 * @param t_first Operand.
 * @return ~t_first
 */
constexpr IE::Image::Location operator~(IE::Image::Location t_first) noexcept {
	return Location(~(uint8_t) t_first);
}

//
// ===================== Implementation of IE::Image =====================
//

IE::Image::Image() noexcept: m_location{IE_IMAGE_LOCATION_NULL}, m_size{0}, m_components{0} {}

IE::Image &IE::Image::operator=(const IE::Image &other) {
	if (&other != this) {
		m_size = other.m_size;
		m_dimensions = other.m_dimensions;
		m_components = other.m_components;
		m_location = other.m_location;
		m_data = other.m_data;
		/** @todo Add the parts of this code that copies the VkImage to the new location, and possibly to the new renderEngine*/
	}
	return *this;
}

IE::Image &IE::Image::operator=(IE::Image &&other) noexcept {
	if (&other != this) {
		m_size = std::exchange(other.m_size, 0);
		m_dimensions = std::exchange(other.m_dimensions, std::vector<size_t>{});
		m_components = std::exchange(other.m_components, 0);
		m_location = std::exchange(other.m_location, IE_IMAGE_LOCATION_NULL);
		m_data = std::exchange(other.m_data, std::vector<unsigned char>{});
		/**@todo Add the parts of this function that would destroy the image and possibly rebuild it under the new engine. This may have to be implemented in the classes that inherit this one. */
	}
	return *this;
}

IE::Image::Image(IE::Image &&other) noexcept:
		m_size{std::exchange(other.m_size, 0)},
		m_dimensions{std::exchange(other.m_dimensions, std::vector<size_t>{})},
		m_components{std::exchange(other.m_components, 0)},
		m_location{std::exchange(other.m_location, IE_IMAGE_LOCATION_NULL)},
		m_data{std::exchange(other.m_data, std::vector<unsigned char>{})},
		m_linkedRenderEngine{std::exchange(other.m_linkedRenderEngine, std::weak_ptr<IERenderEngine>{})} {}

		
std::unique_ptr<IE::Image> IE::Image::factory(IERenderEngine *t_engineLink) {
	if (t_engineLink->API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		return std::unique_ptr<IE::Image>{reinterpret_cast<Image *>(new IE::ImageVulkan())};
	} if (t_engineLink->API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		return std::unique_ptr<IE::Image>{reinterpret_cast<Image *>(new IE::ImageOpenGL())};
	}
}
