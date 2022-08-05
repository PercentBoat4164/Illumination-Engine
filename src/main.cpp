#include <memory>

#include "GraphicsModule/IERenderEngine.cpp"
#include "InputModule/IEKeyboard.hpp"
#include "Core/AssetModule/IEAsset.hpp"
#include "Core/FileSystemModule/IEFileSystem.hpp"

int main() {
	IESettings settings = IESettings();
	// RenderEngine must be allocated on the heap.
	std::shared_ptr<IERenderEngine> renderEngine = std::make_shared<IERenderEngine>(settings);

	IEKeyboard keyboard{renderEngine->window};
	keyboard.editActions(GLFW_KEY_W, [&](GLFWwindow *) { renderEngine->camera.position += renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed; });
	keyboard.editActions(GLFW_KEY_A, [&](GLFWwindow *) { renderEngine->camera.position -= renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed; });
	keyboard.editActions(GLFW_KEY_S, [&](GLFWwindow *) { renderEngine->camera.position -= renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed; });
	keyboard.editActions(GLFW_KEY_D, [&](GLFWwindow *) { renderEngine->camera.position += renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed; });
	keyboard.editActions(GLFW_KEY_SPACE, [&](GLFWwindow *) { renderEngine->camera.position += renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed; });
	keyboard.editActions(GLFW_KEY_LEFT_SHIFT, [&](GLFWwindow *) { renderEngine->camera.position -= renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed; });
	keyboard.editActions({GLFW_KEY_LEFT_CONTROL, GLFW_PRESS}, [&](GLFWwindow *) { renderEngine->camera.speed *= 6; });
	keyboard.editActions({GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE}, [&](GLFWwindow *) { renderEngine->camera.speed /= 6; });
	keyboard.editActions({GLFW_KEY_F11, GLFW_PRESS}, [&](GLFWwindow *) { renderEngine->toggleFullscreen(); });
	keyboard.editActions({GLFW_KEY_ESCAPE, GLFW_REPEAT}, [&](GLFWwindow *) { glfwSetWindowShouldClose(renderEngine->window, 1); });

	IEWindowUser windowUser{std::shared_ptr<IERenderEngine>(renderEngine), &keyboard};
	glfwSetWindowUserPointer(renderEngine->window, &windowUser);

	std::shared_ptr<IEAsset> fbx = std::make_shared<IEAsset>();
	fbx->filename = "res/assets/AncientStatueOptimized/model/ancientStatue.fbx";
	fbx->addAspect(new IERenderable{});
	fbx->position = {0.0F, -1.0F, 2.0F};
	renderEngine->addAsset(fbx);
	std::shared_ptr<IEAsset> obj = std::make_shared<IEAsset>();
	obj->filename = "res/assets/AncientStatueOptimized/model/ancientStatue.fbx";
	obj->addAspect(new IERenderable{});
	obj->position = {0.0F, -1.0F, -0.0F};
	renderEngine->addAsset(obj);
	std::shared_ptr<IEAsset> glb = std::make_shared<IEAsset>();
	glb->filename = "res/assets/AncientStatueOptimized/model/ancientStatue.fbx";
	glb->addAspect(new IERenderable{});
	glb->position = {0.0F, -1.0F, -2.0F};
	renderEngine->addAsset(glb);

	renderEngine->camera.position = {0.0F, 2.0F, 0.0F};

	renderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, fmt::format("Beginning main loop on thread {:#x}.", pthread_self()));

	glfwSetTime(0);
	while (renderEngine->update()) {
		fbx->rotation += glm::vec3(0, 0, glm::pi<double>()) * renderEngine->frameTime;
		obj->rotation += glm::vec3(0, -glm::pi<double>(), 0) * renderEngine->frameTime;
		glb->rotation += glm::vec3(-glm::pi<double>(), 0, 0) * renderEngine->frameTime;
		glfwPollEvents();
		keyboard.handleQueue();
	}
}