#pragma once

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS

#include <algorithm>
#include <glm/glm.hpp>
#include <memory>

namespace IE::Core {
class Aspect;

class Asset : public std::enable_shared_from_this<Asset> {
public:
    // Things that are shared among all m_aspects of an asset
    glm::vec3             position{0.0, 0.0, 0.0};
    glm::vec3             rotation{0.0, 0.0, 0.0};
    glm::vec3             m_scale{1.0, 1.0, 1.0};
    std::string           m_filename{};
    std::vector<Aspect *> m_aspects{};

    void addAspect(Aspect *aspect);

    template<typename... Args>
    Asset(Args... args) {
        (m_aspects.push_back(args), ...);
    }

    Asset(std::vector<IE::Core::Aspect *> t_aspects) : m_aspects(t_aspects) {
    }
};
}  // namespace IE::Core
