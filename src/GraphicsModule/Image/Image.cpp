#include "Image.hpp"

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

