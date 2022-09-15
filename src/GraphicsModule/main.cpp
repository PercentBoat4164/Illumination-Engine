//#include "IERenderEngine.hpp"
//
//#include "Image/ImageVulkan.hpp"
//#include "Image/ImageOpenGL.hpp"

//void doAThing(IE::Graphics::Image *image) {
//	if (auto vulkan = dynamic_cast<const IE::Graphics::detail::ImageVulkan *>(image)) {
//		std::cout << "Vulkan Image\n";
//	} else if (auto opengl = dynamic_cast<const IE::Graphics::detail::ImageOpenGL *>(image)) {
//		std::cout << "OpenGL Image\n";
//	}
//
//}

#include "RenderPass/RenderPassController.hpp"
#include "RenderPass/ColorRenderPass.hpp"
#include "RenderPass/ShadowRenderPass.hpp"

int main() {
	IE::Graphics::RenderPassController controller{};
	
	// Build shadow pass
	IE::Graphics::ShadowRenderPass shadowPass{};
	shadowPass.setResolution({2048, 2048});
	
	// Build color pass
	IE::Graphics::ColorRenderPass colorPass{};
	colorPass.setResolution({1920, 1080});
	
	// Build and link the render passes
	controller.addRenderPass(shadowPass, "shadowCascade0");
	controller.addRenderPass(colorPass, "color");
	controller.build();
	
	// Get created assets from the passes to be used in descriptor sets and such
	shadowPass.getMaps();

//	IESettings settings{};
//	auto OpenGLEngine{std::make_shared<IERenderEngine>(settings)};
//	auto VulkanEngine{std::make_shared<IERenderEngine>(&settings)};
//	auto OpenGLImage(IE::Graphics::Image::create(OpenGLEngine));
//	auto VulkanImage(IE::Graphics::Image::create(VulkanEngine));
//	doAThing(OpenGLImage.get());
//	doAThing(VulkanImage.get());
//	IE::Core::MultiDimensionalVector<int> vector{12u, 12u};
//	vector[{11, 3}] = 108;
//	IE::Core::MultiDimensionalVector<int> slice{vector[{{5, 4}, {10, 6}}]};
//	int element = vector[{11, 3}];
//	std::cout << slice.m_dimensions[0] << ", " << slice.m_dimensions[1] << std::endl;
//	std::cout << element << std::endl;
}

/* ******************************************
* IE::Graphics::Image image{};
* doAThing(image, ...);
*
****************************************** */