#pragma once

#include "Core/FileSystemModule/File.hpp"

#include <string>
#include <vector>

#define GLM_FORCE_RADIANS

#include "IEAspect.hpp"

#include <algorithm>
#include <glm/glm.hpp>
#include <memory>

class IEAsset : public std::enable_shared_from_this<IEAsset> {
public:
    // Things that are shared among all aspects of an asset
    glm::vec3                              position{0.0, 0.0, 0.0};
    glm::vec3                              rotation{0.0, 0.0, 0.0};
    glm::vec3                              scale{1.0, 1.0, 1.0};
    std::string                            filename{};
    std::vector<std::shared_ptr<IEAspect>> aspects{};

    void addAspect(IEAspect *aspect);
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