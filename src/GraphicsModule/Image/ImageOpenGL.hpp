#pragma once

#include "Image.hpp"

namespace IE { class ImageOpenGL; }
	
class IE::ImageOpenGL : public IE::Image {
	GLuint id{};
};