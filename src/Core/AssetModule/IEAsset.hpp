#pragma once

#include "Core/EngineModule/Engine.hpp"
#include "IEAspect.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include <algorithm>
#include <string>
#include <vector>
#include <memory>

class IEAsset : public std::enable_shared_from_this<IEAsset> {
public:
	// Things that are shared among all aspects of an asset
	glm::vec3 position{0.0, 0.0, 0.0};
	glm::vec3 rotation{0.0, 0.0, 0.0};
	glm::vec3 scale{1.0, 1.0, 1.0};
	std::string filename{};
	std::vector<std::weak_ptr<IEAspect>> aspects{};

	void addAspect(IE::Core::Engine *, const std::string &);
};