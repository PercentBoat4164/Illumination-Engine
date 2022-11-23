#pragma once

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS

#include "Aspect.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <memory>

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

    Asset(IE::Core::File *t_file);

    void addAspect(std::shared_ptr<Aspect> t_aspect);
};
}  // namespace IE::Core

/*
 * AssetName
 * |--models
 * |  |--model1
 * |  |--model2
 * |
 * |--textures
 * |  |--texture1
 * |  |--texture2
 * |
 * |--sounds
 * |  |--scarySounds!
 * |  |  |--sound1
 * |  |
 * |  |--normalSounds
 * |     |--sound2
 * |
 * AssetName
 * |--models
 * |  |--model1
 * |  |--model3
 */

/*
 * ExampleAsset
 * |--models
 * |  |--BillyBobJoe Jr.
 * |
 * ExampleAsset2
 * |--models
 * |  |--BillyBobJoe Jr.
 * |  |--Cube
 */