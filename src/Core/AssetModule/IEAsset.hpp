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

    /** @brief Aspects can be one of:
     * - Renderable xN  // Control how the asset is displayed visually
     * - SoundPlayerThing xN  // Control what sounds the asset creates in-game
     * - Script xN  // Control the asset based on events or timings
     * - PhysicsBody xN  // Control how the asset interacts with other assets
     * - ???User specified??? xN  // Control low-level interactions with the game engine
     */
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