#pragma once

#include <cstdint>
#include <mutex>
#include <cassert>
#include <vector>
#include "stb_image.h"

enum IEImageLocation {
	IE_IMAGE_LOCATION_NULL = 0x0,
	IE_IMAGE_LOCATION_NONE = 0x1,
	IE_IMAGE_LOCATION_SYSTEM = 0x2,
	IE_IMAGE_LOCATION_VIDEO = 0x4
};

constexpr IEImageLocation operator|(IEImageLocation first, IEImageLocation second) noexcept {
	return (IEImageLocation) ((uint8_t) first | (uint8_t) second);
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


class aiTexture;

template<uint32_t width = 0, uint32_t height = 0, uint8_t channels = 0>
class IEImageNEW {
public:
	std::size_t size;
	IEImageLocation location;
	std::array<char, width * height * channels> *data;

	IEImageNEW() :
			size{width * height * channels},
			location{IE_IMAGE_LOCATION_NONE},
			data{} {}

	explicit IEImageNEW(IEImageNEW<0, 0, 0> &);

	virtual void setLocations(IEImageLocation) {}

	/** Load assimp image data into the currently set location(s)*/
	virtual void uploadTexture(const aiTexture *) {}

	/** Load stb_image data of size 'size' into the currently set location(s) */
	virtual void uploadTexture(stbi_uc *) {}

	/** Load data into the currently set location(s) */
	virtual void uploadData(std::array<char, width * height * channels> *) {}
};


template<>
class IEImageNEW<0, 0, 0> : public IEImageNEW<1, 1, 1> {
public:
	uint32_t width{};
	uint32_t height{};
	uint8_t channels{};
	std::vector<char> *data{};

	virtual void setDimensions(uint32_t, uint32_t, uint8_t);

	using IEImageNEW<1, 1, 1>::setLocations;

	using IEImageNEW<1, 1, 1>::uploadTexture;

	virtual void uploadData(std::vector<char> *) {}
};
