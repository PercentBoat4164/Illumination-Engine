#pragma once

#include <glm/vec3.hpp>
#include <memory>

namespace IE::Core {
class Asset;
class Aspect;

class Instance {
public:
    glm::vec3                         m_position{0.0, 0.0, 0.0};
    glm::vec3                         m_rotation{0.0, 0.0, 0.0};
    glm::vec3                         m_scale{1.0, 1.0, 1.0};
    std::shared_ptr<IE::Core::Asset>  m_asset;
    std::shared_ptr<IE::Core::Aspect> m_aspect;

    static std::shared_ptr<Instance>
    Factory(const std::shared_ptr<IE::Core::Asset> &t_asset, const std::shared_ptr<IE::Core::Aspect> &t_aspect);

private:
    Instance(const std::shared_ptr<IE::Core::Asset> &t_asset, const std::shared_ptr<IE::Core::Aspect> &t_aspect);
};
}  // namespace IE::Core
