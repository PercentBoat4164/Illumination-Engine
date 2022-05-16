#include "GraphicsModule/IERenderEngine.cpp"
#include "InputModule/IEKeyboard.hpp"
#include "Core/AssetModule/IEAsset.hpp"
#include "Core/FileSystemModule/IEFileSystem.hpp"

int main() {
	IESettings settings{};
	IERenderEngine renderEngine{&settings};

	IERenderable::setAPI(renderEngine.API);

	IEKeyboard keyboard{renderEngine.window};
	keyboard.editActions(GLFW_KEY_W, [&](GLFWwindow *) {
		renderEngine.camera.position += renderEngine.camera.front * renderEngine.frameTime * renderEngine.camera.speed;
	});
	keyboard.editActions(GLFW_KEY_A, [&](GLFWwindow *) {
		renderEngine.camera.position -= renderEngine.camera.right * renderEngine.frameTime * renderEngine.camera.speed;
	});
	keyboard.editActions(GLFW_KEY_S, [&](GLFWwindow *) {
		renderEngine.camera.position -= renderEngine.camera.front * renderEngine.frameTime * renderEngine.camera.speed;
	});
	keyboard.editActions(GLFW_KEY_D, [&](GLFWwindow *) {
		renderEngine.camera.position += renderEngine.camera.right * renderEngine.frameTime * renderEngine.camera.speed;
	});
	keyboard.editActions(GLFW_KEY_SPACE, [&](GLFWwindow *) {
		renderEngine.camera.position += renderEngine.camera.up * renderEngine.frameTime * renderEngine.camera.speed;
	});
	keyboard.editActions(GLFW_KEY_LEFT_SHIFT, [&](GLFWwindow *) {
		renderEngine.camera.position -= renderEngine.camera.up * renderEngine.frameTime * renderEngine.camera.speed;
	});
	keyboard.editActions({GLFW_KEY_LEFT_CONTROL, GLFW_PRESS}, [&](GLFWwindow *) { renderEngine.camera.speed *= 6; });
	keyboard.editActions({GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE}, [&](GLFWwindow *) { renderEngine.camera.speed /= 6; });
	keyboard.editActions({GLFW_KEY_F11, GLFW_PRESS}, [&](GLFWwindow *) { renderEngine.toggleFullscreen(); });
	keyboard.editActions({GLFW_KEY_ESCAPE, GLFW_REPEAT}, [&](GLFWwindow *) { glfwSetWindowShouldClose(renderEngine.window, 1); });

	IEWindowUser windowUser{&renderEngine, &keyboard};
	glfwSetWindowUserPointer(renderEngine.window, &windowUser);

	IEAsset fbx{.filename="res/Models/AncientStatue/ancientStatue.fbx"};
	fbx.addAspect(new IERenderable(&renderEngine, "res/Models/AncientStatue/ancientStatue.fbx"));
	renderEngine.addAsset(&fbx);

	fbx.position = {0.0F, -1.0F, 0.0F};
	renderEngine.camera.position = {0.0F, 2.0F, 0.0F};

	renderEngine.settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_INFO, fmt::format("Beginning main loop on thread {:#x}.", pthread_self()));

	glfwSetTime(0);
	while (renderEngine.update()) {
		fbx.rotation += glm::vec3(0, 0, glm::pi<double>()) * renderEngine.frameTime;
		glfwPollEvents();
		keyboard.handleQueue();
	}
}