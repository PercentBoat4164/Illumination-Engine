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
    // Things that are shared among all aspects of an asset
    glm::vec3                              position{0.0, 0.0, 0.0};
    glm::vec3                              rotation{0.0, 0.0, 0.0};
    glm::vec3                              scale{1.0, 1.0, 1.0};
    std::string                            filename{};
    std::vector<std::shared_ptr<Aspect>> aspects{};

    void addAspect(Aspect *aspect);
};
}  // namespace IE::Core
