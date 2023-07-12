#pragma once

#include "Core/AssetModule/Asset.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"

#include <memory>

namespace IE::Graphics {
struct Instance {
    glm::vec3                        position;
    glm::quat                        rotation;
    glm::vec3                        scale;
    std::shared_ptr<IE::Core::Asset> assetOffset;
};
}  // namespace IE::Graphics