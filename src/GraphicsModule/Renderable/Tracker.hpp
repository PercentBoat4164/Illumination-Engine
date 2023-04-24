#pragma once

#include "Buffer/IEBuffer.hpp"
#include "Core/AssetModule/Aspect.hpp"
#include "glm/glm.hpp"
#include "Shader/IEUniformBufferObject.hpp"

class IERenderable;

namespace IE::Graphics {
struct Tracker : public Aspect {
public:
    glm::mat4                     modelMatrix{};
    std::shared_ptr<IERenderable> renderable{};
    std::weak_ptr<Asset>        asset{};
};
}  // namespace IE::Graphics