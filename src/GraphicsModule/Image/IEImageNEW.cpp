#include "IEImageNEW.hpp"

template<uint32_t width, uint32_t height, uint8_t channels>
IEImageNEW<width, height, channels>::IEImageNEW(IEImageNEW<0, 0, 0> &t_image) :
		size{t_image.size},
		location{t_image.location},
		data{} {
	if (t_image.size != size || t_image.width != width || t_image.height != height || t_image.channels != channels) {
//		throw std::bad_cast();
		// Recover
		size = width * height * channels;
	}
	if (data != nullptr && t_image.data != nullptr) {
		for (size_t i = 0; i < std::min(size, t_image.size); ++i) {
			data->at(i) = t_image.data->at(i);
		}
	}
}

void IEImageNEW<0, 0, 0>::setDimensions(uint32_t t_width, uint32_t t_height, uint8_t t_channels) {
	width = t_width;
	height = t_height;
	channels = t_channels;
	size = width * height * channels;
	if (data != nullptr) {
		data->resize(size);
	}
}