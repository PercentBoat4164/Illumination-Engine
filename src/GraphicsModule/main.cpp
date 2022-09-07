#include "IERenderEngine.hpp"

#include "Image/Image.hpp"
#include "Image/ImageVulkan.hpp"
#include "Image/ImageOpenGL.hpp"

void doAThing(IE::Image *image) {
	if (auto vulkan = dynamic_cast<const IE::ImageVulkan *>(image)) {
		std::cout << "Vulkan Image";
	} else if (auto opengl = dynamic_cast<const IE::ImageOpenGL *>(image)) {
		std::cout << "OpenGL Image";
	}
	
}

int main() {
	IESettings settings{};
	IERenderEngine renderEngine{&settings};
	IE::Image *imageFromRenderEngine{renderEngine.createImage()};
	auto imageFromImageFactory(IE::Image::factory(&renderEngine));
	doAThing(imageFromRenderEngine);
	doAThing(imageFromImageFactory.get());
//	std::cout << vulkan.format << std::endl;
}

/* ******************************************
* IE::Image image{};
* doAThing(image, ...);
*
****************************************** */