#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <algorithm>
#include <memory>
#include "IEAspect.hpp"

class IEAsset : public std::enable_shared_from_this<IEAsset> {
public:
	// Things that are shared among all aspects of an asset
	glm::vec3 position{};
	glm::vec3 rotation{};
	glm::vec3 scale{1.0f, 1.0f, 1.0f};
	std::string filename{};
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