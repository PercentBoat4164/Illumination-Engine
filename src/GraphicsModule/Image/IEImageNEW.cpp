#include <c++/12/iostream>
#include "IEImageNEW.hpp"

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

template<uint32_t width, uint32_t height, uint8_t channels>
IEImageNEW<width, height, channels>::IEImageNEW(IEImageNEW<0, 0, 0> &t_image) :
		size{t_image.getSize()},
		location{t_image.getLocation()},
		data{} {
	if (t_image.getSize() != size || t_image.getWidth() != width || t_image.getHeight() != height || t_image.getChannels() != channels) {
		// Resize image
		size = width * height * channels;
	}
	if (data != nullptr && t_image.getData() != nullptr) {
		for (size_t i = 0; i < std::min(size, t_image.getSize()); ++i) {
			data->at(i) = t_image.getData()->at(i);
		}
	}
}

template<uint32_t width, uint32_t height, uint8_t channels>
void IEImageNEW<width, height, channels>::setLocation(IEImageLocation) {
	std::cout << getWidth() << '\n';
}

template<uint32_t width, uint32_t height, uint8_t channels>
uint32_t IEImageNEW<width, height, channels>::getWidth() const {
	return width;
}

template<uint32_t width, uint32_t height, uint8_t channels>
uint32_t IEImageNEW<width, height, channels>::getHeight() const {
	return height;
}

template<uint32_t width, uint32_t height, uint8_t channels>
uint8_t IEImageNEW<width, height, channels>::getChannels() const {
	return channels;
}

template<uint32_t width, uint32_t height, uint8_t channels>
std::array<char, width * height * channels> *IEImageNEW<width, height, channels>::getData() const {
	return data;
}

template<uint32_t width, uint32_t height, uint8_t channels>
size_t IEImageNEW<width, height, channels>::getSize() const {
	return size;
}

template<uint32_t width, uint32_t height, uint8_t channels>
IEImageLocation IEImageNEW<width, height, channels>::getLocation() const {
	return location;
}

/* Specializations for the dynamically resizable image */
void IEImageNEW<0, 0, 0>::setDimensions(uint32_t t_width, uint32_t t_height, uint8_t t_channels) {
	width = t_width;
	height = t_height;
	channels = t_channels;
	size = width * height * channels;
	if (location & IE_IMAGE_LOCATION_SYSTEM && data != nullptr) {
		data->resize(size);
	}
}

uint32_t IEImageNEW<0, 0, 0>::getWidth() const {
	return width;
}

uint32_t IEImageNEW<0, 0, 0>::getHeight() const {
	return height;
}

uint8_t IEImageNEW<0, 0, 0>::getChannels() const {
	return channels;
}

std::vector<char> *IEImageNEW<0, 0, 0>::getData() const {
	return data;
}