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
	auto renderEngine{std::make_shared<IERenderEngine>(settings)};
	auto imageFromImageFactory(IE::Image::create(renderEngine));
	doAThing(imageFromImageFactory.get());
	IE::Core::MultiDimensionalVector<int> vector{12u, 12u};
	vector[{11, 3}] = 108;
	IE::Core::MultiDimensionalVector<int> slice{vector[{{5, 4}, {10, 6}}]};
	int element = vector[{11, 3}];
	std::cout << slice.m_dimensions[0] << ", " << slice.m_dimensions[1] << std::endl;
	std::cout << element << std::endl;
}

/* ******************************************
* IE::Image image{};
* doAThing(image, ...);
*
****************************************** */