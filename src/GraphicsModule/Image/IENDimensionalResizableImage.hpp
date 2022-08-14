#pragma once

#include <vector>

enum IEImageLocation {
	IE_IMAGE_LOCATION_NULL = 0x0,
	IE_IMAGE_LOCATION_NONE = 0x1,
	IE_IMAGE_LOCATION_SYSTEM = 0x2,
	IE_IMAGE_LOCATION_VIDEO = 0x4
};

constexpr IEImageLocation operator|(IEImageLocation first, IEImageLocation second) noexcept;

constexpr IEImageLocation operator&(IEImageLocation first, IEImageLocation second) noexcept;

constexpr IEImageLocation operator^(IEImageLocation first, IEImageLocation second) noexcept;

constexpr IEImageLocation operator~(IEImageLocation first) noexcept;

class IENDimensionalResizableImage {
private:
	char *m_data;
	size_t m_size;
	size_t *m_dimensions;
	size_t m_dimensionCount;
	size_t m_channels;
	IEImageLocation m_location;

public:
	template<typename ...Args>
	explicit IENDimensionalResizableImage(size_t t_channels, Args... t_dimensions);

	[[nodiscard]] char *getData() const;

	[[nodiscard]] size_t getSize() const;

	[[nodiscard]] size_t *getDimensions() const;

	[[nodiscard]] size_t getDimensionCount() const;

	[[nodiscard]] size_t getChannels() const;

	void setLocation(IEImageLocation);

	[[nodiscard]] IEImageLocation getLocation() const;
};

