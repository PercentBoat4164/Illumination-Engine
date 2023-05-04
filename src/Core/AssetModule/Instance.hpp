#pragma once

#include <glm/vec3.hpp>

namespace IE::Core {
class Asset;
class Aspect;

class Instance {
public:
    glm::vec3         m_position{0.0, 0.0, 0.0};
    glm::vec3         m_rotation{0.0, 0.0, 0.0};
    glm::vec3         m_scale{1.0, 1.0, 1.0};
    IE::Core::Asset  *m_asset;
    IE::Core::Aspect *m_aspect;

    Instance(IE::Core::Asset &t_asset, IE::Core::Aspect &t_aspect);

    Instance(IE::Core::Asset *t_asset, IE::Core::Aspect *t_aspect);
};
}  // namespace IE::Core
