#include <memory>

#include "GraphicsModule/IERenderEngine.hpp"
#include "GraphicsModule/Renderable/IERenderable.hpp"
#include "InputModule/IEKeyboard.hpp"
#include "Core/AssetModule/IEAsset.hpp"
#include "Core/FileSystemModule/IEFileSystem.hpp"
#include "Image/IEImageNEW.cpp"

int main() {
	// Create new resizable image
	IEImageNEW image{};
	// Resize it to fit the dimensions of our pretend image
	image.setDimensions(512, 512, 4);
	// Set image to be stored on system and video memory
	image.setLocations(IE_IMAGE_LOCATION_VIDEO | IE_IMAGE_LOCATION_SYSTEM);
	// Upload texture data
	image.uploadTexture((const aiTexture *) nullptr);
	// At this point the texture data can be changed, the size of the image can be changed (cropping or extension may have to occur), or the location
	// 	in which the texture is stored can change, each independently of the others

	// Create a statically sized 512x512x4 image
	IEImageNEW<512, 512, 4> knownImage{};
	// Upload fake texture data
	knownImage.uploadTexture((stbi_uc *) nullptr);
	// Data is now put into video memory
	knownImage.setLocations(IE_IMAGE_LOCATION_VIDEO);
	// At this point the data, or the location, but not the size of the knownImage can change.

	// Copy the dynamically sized image into a statically sized image
	IEImageNEW<120, 120, 3> newImage{image};  // This will work even if sizes don't match. The image will be cropped or extended.
	newImage.uploadData(nullptr);
	newImage.setLocations(IE_IMAGE_LOCATION_SYSTEM);
	// At this point two images exist in system memory and two exist in video memory.


//	IESettings settings = IESettings();
//	// RenderEngine must be allocated on the heap.
//	std::shared_ptr<IERenderEngine> renderEngine = std::make_shared<IERenderEngine>(settings);
//
//	IEKeyboard keyboard{renderEngine->window};
//	keyboard.editActions(GLFW_KEY_W, [&](GLFWwindow *) { renderEngine->camera.position += renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed; });
//	keyboard.editActions(GLFW_KEY_A, [&](GLFWwindow *) { renderEngine->camera.position -= renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed; });
//	keyboard.editActions(GLFW_KEY_S, [&](GLFWwindow *) { renderEngine->camera.position -= renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed; });
//	keyboard.editActions(GLFW_KEY_D, [&](GLFWwindow *) { renderEngine->camera.position += renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed; });
//	keyboard.editActions(GLFW_KEY_SPACE, [&](GLFWwindow *) { renderEngine->camera.position += renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed; });
//	keyboard.editActions(GLFW_KEY_LEFT_SHIFT, [&](GLFWwindow *) { renderEngine->camera.position -= renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed; });
//	keyboard.editActions({GLFW_KEY_LEFT_CONTROL, GLFW_PRESS}, [&](GLFWwindow *) { renderEngine->camera.speed *= 6; });
//	keyboard.editActions({GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE}, [&](GLFWwindow *) { renderEngine->camera.speed /= 6; });
//	keyboard.editActions({GLFW_KEY_F11, GLFW_PRESS}, [&](GLFWwindow *) { renderEngine->toggleFullscreen(); });
//	keyboard.editActions({GLFW_KEY_ESCAPE, GLFW_REPEAT}, [&](GLFWwindow *) { glfwSetWindowShouldClose(renderEngine->window, 1); });
//
//	IEWindowUser windowUser{std::shared_ptr<IERenderEngine>(renderEngine), &keyboard};
//	glfwSetWindowUserPointer(renderEngine->window, &windowUser);
//
//	std::shared_ptr<IEAsset> fbx = std::make_shared<IEAsset>();
//	fbx->filename = "res/assets/AncientStatue/models/ancientStatue.fbx";
//	fbx->addAspect(new IERenderable{});
//	fbx->position = {2, 1, 0};
//	renderEngine->addAsset(fbx);
//	std::shared_ptr<IEAsset> obj = std::make_shared<IEAsset>();
//	obj->filename = "res/assets/AncientStatue/models/ancientStatue.obj";
//	obj->addAspect(new IERenderable{});
//	obj->position = {0, 1, 0};
//	renderEngine->addAsset(obj);
//	std::shared_ptr<IEAsset> glb = std::make_shared<IEAsset>();
//	glb->filename = "res/assets/AncientStatue/models/ancientStatue.glb";
//	glb->addAspect(new IERenderable{});
//	glb->position = {-2, 1, 0};
//	renderEngine->addAsset(glb);
//	std::shared_ptr<IEAsset> floor = std::make_shared<IEAsset>();
//	floor->filename = "res/assets/DeepslateFloor/models/DeepslateFloor.fbx";
//	floor->addAspect(new IERenderable{});
//	renderEngine->addAsset(floor);
//	floor->position = {0, 0, -1};
//
//	renderEngine->camera.position = {0.0F, -2.0F, 1.0F};
//
//	renderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, "Beginning main loop.");
//
//	glfwSetTime(0);
//	while (renderEngine->update()) {
//		glfwPollEvents();
//		keyboard.handleQueue();
//	}
}