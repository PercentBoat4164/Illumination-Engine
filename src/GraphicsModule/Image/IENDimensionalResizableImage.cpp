#include "IENDimensionalResizableImage.hpp"
#include <cstdint>
#include <typeinfo>

template<typename... Args>
IENDimensionalResizableImage::IENDimensionalResizableImage(size_t t_channels, Args... t_dimensions) :
		m_data{nullptr},
		m_size{1},
		m_dimensionCount{sizeof...(t_dimensions)},
		m_channels{t_channels},
		m_location{IE_IMAGE_LOCATION_NONE} {
	((m_size *= t_dimensions), ...);
	m_dimensions = new size_t[m_dimensionCount];
	if (m_dimensionCount == 0) m_size = 0;
}

char *IENDimensionalResizableImage::getData() const {
	return m_data;
}

size_t IENDimensionalResizableImage::getSize() const {
	return m_size;
}

size_t *IENDimensionalResizableImage::getDimensions() const {
	return m_dimensions;
}

size_t IENDimensionalResizableImage::getDimensionCount() const {
	return m_dimensionCount;
}

size_t IENDimensionalResizableImage::getChannels() const {
	return m_channels;
}

IEImageLocation IENDimensionalResizableImage::getLocation() const {
	return m_location;
}

void IENDimensionalResizableImage::setLocation(IEImageLocation t_location) {
	m_location = t_location;
}

constexpr IEImageLocation operator|(IEImageLocation first, IEImageLocation second) noexcept {
	uint8_t result = (uint8_t) first | (uint8_t) second;
	if (result ^ IE_IMAGE_LOCATION_NONE & (uint8_t) IE_IMAGE_LOCATION_SYSTEM | (uint8_t) IE_IMAGE_LOCATION_VIDEO) {
		throw std::bad_cast();
	}
	return (IEImageLocation) result;
}

constexpr IEImageLocation operator&(IEImageLocation first, IEImageLocation second) noexcept {
	return (IEImageLocation) ((uint8_t) first & (uint8_t) second);
}

constexpr IEImageLocation operator^(IEImageLocation first, IEImageLocation second) noexcept {
	return (IEImageLocation) ((uint8_t) first ^ (uint8_t) second);
}

constexpr IEImageLocation operator~(IEImageLocation first) noexcept {
	return (IEImageLocation) (~(uint8_t) first);
}
