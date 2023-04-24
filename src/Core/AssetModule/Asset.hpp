#pragma once

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS

#include "Aspect.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <memory>

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