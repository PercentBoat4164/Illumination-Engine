/* Include this file's header. */
#include "IECamera.hpp"

/* Include dependencies within this module. */
#include "IERenderEngine.hpp"

/* Include external dependencies. */
#include "glm/ext/matrix_clip_space.hpp"


void IECamera::create(IERenderEngine *engineLink) {
    linkedRenderEngine = engineLink;
    updateSettings();
    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, 0.01, linkedRenderEngine->settings.renderDistance);
}

void IECamera::update() {
    front = glm::normalize(glm::vec3{cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch))});
    right = glm::normalize(glm::cross(front, up));
    viewMatrix = {glm::lookAt(position, position + front, up)};
    projectionMatrix = {glm::perspective(glm::radians(horizontalFOV), double(linkedRenderEngine->settings.resolution[0]) / linkedRenderEngine->settings.resolution[1], 0.01, linkedRenderEngine->settings.renderDistance)};
    projectionMatrix[1][1] *= -1.0f;
}

void IECamera::updateSettings() {
    aspectRatio = double(linkedRenderEngine->settings.resolution[0]) / linkedRenderEngine->settings.resolution[1];
    fov = linkedRenderEngine->settings.fov;
    horizontalFOV = tanh(tan(linkedRenderEngine->settings.fov*(ILLUMINATION_ENGINE_PI / 360)) * 1 / aspectRatio) * (360 / ILLUMINATION_ENGINE_PI);
}
