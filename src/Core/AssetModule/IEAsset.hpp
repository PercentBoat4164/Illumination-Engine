#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "IEAspect.hpp"

class IEAsset {
public:
    // Things that are shared among all aspects of an asset
    glm::vec3 position{};
    glm::vec3 rotation{};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    std::string filename{};

    std::vector<void*> aspects{};

    void addAspect(IEAspect* aspect) {
        aspects.emplace_back(aspect);
        aspect->associatedAssets.push_back(this);
    }
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