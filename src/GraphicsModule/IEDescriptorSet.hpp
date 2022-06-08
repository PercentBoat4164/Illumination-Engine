#pragma once

/* Predefine classes used with pointers or as return values for functions. */

class IEBuffer;

class IEImage;

class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "GraphicsModule/CommandBuffer/IEDependency.hpp"

// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <cstdint>
#include <functional>
#include <optional>
#include <variant>
#include <vector>


class IEDescriptorSet : public IEDependency {
public:
	struct CreateInfo {
		//Required
		std::vector<VkDescriptorPoolSize> poolSizes{};
		std::vector<VkShaderStageFlagBits> shaderStages{};
		std::vector<std::optional<std::variant<IEImage *, IEBuffer *>>> data{};

		//Optional
		uint32_t maxIndex{1};

		//Required if maxIndex != 1
		VkDescriptorBindingFlagsEXT flags{0};
	};

	VkDescriptorPool descriptorPool{};
	VkDescriptorSet descriptorSet{};
	VkDescriptorSetLayout descriptorSetLayout{};
	CreateInfo createdWith{};

	void destroy();

	void create(IERenderEngine *renderEngineLink, CreateInfo *createInfo);

	void update(std::vector<std::optional<std::variant<IEImage *, IEBuffer *>>> newData, std::vector<int> bindings = {});

	~IEDescriptorSet();

private:
	IERenderEngine *linkedRenderEngine{};
	std::vector<std::function<void()>> deletionQueue{};
};