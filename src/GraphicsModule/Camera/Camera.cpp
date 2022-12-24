/* Include this file's header. */
#include "Camera.hpp"

/* Include dependencies within this module. */
#include "RenderEngine.hpp"

/* Include external dependencies. */
#define GLM_FORCE_RADIANS

#include "glm/ext/matrix_clip_space.hpp"

void IE::Graphics::Camera::create(IE::Graphics::RenderEngine *engineLink) {
    linkedRenderEngine = engineLink;
    updateSettings();
    projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, 0.01, renderDistance);
}

void IE::Graphics::Camera::update() {
    front            = glm::normalize(glm::vec3{
      cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
      sin(glm::radians(yaw)) * cos(glm::radians(pitch)),
      sin(glm::radians(pitch))});
    right            = glm::normalize(glm::cross(front, up));
    viewMatrix       = {glm::lookAt(position, position + front, up)};
    projectionMatrix = {glm::perspective(
      glm::radians(horizontalFOV),
      (double) linkedRenderEngine->m_currentResolution[0] / linkedRenderEngine->m_currentResolution[1],
      0.01,
      renderDistance
    )};
    if (linkedRenderEngine->getAPI().name == IE_RENDER_ENGINE_API_NAME_VULKAN) projectionMatrix[1][1] *= -1.0f;
}

void IE::Graphics::Camera::updateSettings() {
    aspectRatio = double(linkedRenderEngine->m_currentResolution[0]) / linkedRenderEngine->m_currentResolution[1];
    horizontalFOV = tanh(tan(fov * (glm::pi<double>() / 360)) * 1 / aspectRatio) * (360 / glm::pi<double>());
}
