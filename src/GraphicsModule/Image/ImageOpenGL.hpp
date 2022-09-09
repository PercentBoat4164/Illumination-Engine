#pragma once

#include "Image.hpp"

namespace IE { class ImageOpenGL; }
	
class IE::ImageOpenGL : public IE::Image {
public:
	GLuint id{};
	
	ImageOpenGL();
	
	explicit ImageOpenGL(const std::weak_ptr<IERenderEngine> &t_engineLink);
	
	ImageOpenGL(ImageOpenGL &&t_other) noexcept = default;
	
	ImageOpenGL(const ImageOpenGL &t_other) = default;
	
	ImageOpenGL &operator=(ImageOpenGL &&t_other) noexcept;
	
	ImageOpenGL &operator=(const ImageOpenGL &t_other);
	
	~ImageOpenGL() override = default;
	
	[[nodiscard]] uint8_t getBytesInFormat() const override;
	
	void setLocation(Location t_location) override;
	
	void setData(const Core::MultiDimensionalVector<unsigned char> &t_data) override;
};