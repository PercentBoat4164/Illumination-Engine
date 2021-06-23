#pragma once

#include <glm/gtc/matrix_transform.hpp>

class VulkanCamera {
public:
    explicit VulkanCamera(VulkanSettings *vulkanSettings) {
        settings = vulkanSettings;
    }

    void create(VulkanGraphicsEngineLink *engineLink) {
        linkedRenderEngine = engineLink;
        proj = glm::perspective(glm::radians(settings->fov), double(settings->resolution[0]) / settings->resolution[1], 0.01, settings->renderDistance);
        Buffer::CreateInfo uniformBufferObjectCreateInfo{sizeof(CameraUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU};
        uniformBufferObject.create(linkedRenderEngine, &uniformBufferObjectCreateInfo);
        deletionQueue.emplace_back([&] { uniformBufferObject.destroy(); });
        view = glm::lookAt(position, position + front, up);
    }

    void destroy() {
        for (const std::function<void()>& function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

    void update() {
        right = glm::normalize(glm::cross(front, up));
        view = {glm::lookAt(position, position + front, up)};
        proj = {glm::perspective(glm::radians(settings->fov), double(settings->resolution[0]) / settings->resolution[1], 0.01, settings->renderDistance)};
        proj[1][1] *= -1.0f;
        CameraUBO ubo = {glm::inverse(view), glm::inverse(proj)};
        memcpy(uniformBufferObject.data, &ubo, sizeof(CameraUBO));
    }

    struct CameraUBO {
        alignas(16) glm::mat4 view{};
        alignas(16) glm::mat4 proj{};
    };

    VulkanGraphicsEngineLink *linkedRenderEngine{};
    VulkanSettings *settings{};
    Buffer uniformBufferObject{};
    glm::vec3 position{0, 0, 2};
    float yaw{-90};
    float pitch{};
    glm::vec3 front{glm::normalize(glm::vec3{cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch))})};
    glm::vec3 up{0, 0, 1};
    glm::vec3 right{glm::cross(front, up)};
    glm::mat4 view{};
    glm::mat4 proj{};
    std::deque<std::function<void()>> deletionQueue{};
};