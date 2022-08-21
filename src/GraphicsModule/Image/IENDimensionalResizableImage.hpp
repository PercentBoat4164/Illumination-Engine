#pragma once

#include <vector>
#include <cstdint>
#include <typeinfo>
#include <stdexcept>

typedef enum IEImageLocation {
	IE_IMAGE_LOCATION_NULL = 0x0,
	IE_IMAGE_LOCATION_NONE = 0x1,
	IE_IMAGE_LOCATION_SYSTEM = 0x2,
	IE_IMAGE_LOCATION_VIDEO = 0x4
} IEImageLocation;

constexpr IEImageLocation operator|(IEImageLocation first, IEImageLocation second) noexcept {
	uint8_t result = (uint8_t) first | (uint8_t) second;
	if (result & IE_IMAGE_LOCATION_NONE && (result & ~IE_IMAGE_LOCATION_NONE) > IE_IMAGE_LOCATION_NULL) {
		std::logic_error("IE_IMAGE_LOCATION_NONE mix error!");
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


class IENDimensionalResizableImage {
private:
	char *m_data;
	size_t m_size;
	size_t *m_dimensions;
	size_t m_dimensionCount;
	size_t m_channels;
	IEImageLocation m_location;

public:
	template<typename... Args>
	explicit IENDimensionalResizableImage(size_t t_channels, Args... t_dimensions) :
			m_data{nullptr},
			m_size{1},
			m_dimensionCount{sizeof...(t_dimensions)},
			m_channels{t_channels},
			m_location{IE_IMAGE_LOCATION_SYSTEM} {
		((m_size *= t_dimensions), ...);
		m_dimensions = new size_t[m_dimensionCount];
		if (m_dimensionCount == 0) m_size = 0;
	}

	[[nodiscard]] char *getData() const;

	[[nodiscard]] size_t getSize() const;

	[[nodiscard]] size_t *getDimensions() const;

	[[nodiscard]] size_t getDimensionCount() const;

	[[nodiscard]] size_t getChannels() const;

	void setLocation(IEImageLocation);

	[[nodiscard]] IEImageLocation getLocation() const;
};

