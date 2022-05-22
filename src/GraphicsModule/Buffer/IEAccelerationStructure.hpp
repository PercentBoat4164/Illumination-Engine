#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IEBuffer.hpp"

// External dependencies
#include <vulkan/vulkan.h>


class IEAccelerationStructure : public IEBuffer {
	struct CreateInfo {
		// Required
		VkAccelerationStructureTypeKHR type{};
		VkTransformMatrixKHR transformMatrix{};
		uint32_t primitiveCount{1};

		// Optional
		VkAccelerationStructureKHR oldAccelerationStructure{};

		// Required if type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
		VkDeviceAddress vertexBufferAddress{};
		VkDeviceAddress indexBufferAddress{};
		VkDeviceAddress transformationBufferAddress{};

		// Required if type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
		std::vector<VkDeviceAddress> bottomLevelAccelerationStructureDeviceAddress{};
	};

public:
	VkAccelerationStructureKHR accelerationStructure{};

	IEAccelerationStructure();

	void create(IERenderEngine *renderEngineLink, IEAccelerationStructure::CreateInfo *createInfo);
};