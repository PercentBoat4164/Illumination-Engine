#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

namespace IE::Graphics {
class Instance {
    glm::mat4 modelMatrix;
    uint32_t  index;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};
}  // namespace IE::Graphics
