#pragma once

#include "Aspect.hpp"

#define GLM_FORCE_RADIANS
#include <algorithm>
#include <glm/glm.hpp>
#include <memory>
#include <string>
#include <vector>

namespace IE::Core {
class File;

class Asset : public std::enable_shared_from_this<Asset> {
public:
    // Things that are shared among all aspects of an asset
    glm::vec3                            m_position{0.0, 0.0, 0.0};
    glm::vec3                            m_rotation{0.0, 0.0, 0.0};
    glm::vec3                            m_scale{1.0, 1.0, 1.0};
    IE::Core::File                      *m_file;
    std::vector<std::shared_ptr<Aspect>> m_aspects{};

    explicit Asset(IE::Core::File *t_file);

    void addAspect(std::shared_ptr<Aspect> t_aspect);
};
}  // namespace IE::Core