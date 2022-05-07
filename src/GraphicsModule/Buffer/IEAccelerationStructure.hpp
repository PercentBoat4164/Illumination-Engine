#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or _function arguments. */
// Internal dependencies
#include "IEBuffer.hpp"

// External dependencies
#include <vulkan/vulkan.h>


class IEAccelerationStructure : public IEBuffer {
public:
	struct CreateInfo {
		VkAccelerationStructureTypeKHR type{};
		VkTransformMatrixKHR *transformationMatrix{};
		uint32_t primitiveCount{1};

		// Optional
		VkAccelerationStructureKHR accelerationStructureToModify{};

		// Only required if type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
		VkDeviceAddress vertexBufferAddress{};
		VkDeviceAddress indexBufferAddress{};
		VkDeviceAddress transformationBufferAddress{};

		// Only required if type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
		std::vector<VkDeviceAddress> bottomLevelAccelerationStructureDeviceAddresses{};
	};
	VkAccelerationStructureKHR accelerationStructure{};

	IEAccelerationStructure();

	void create(IERenderEngine *, IEAccelerationStructure::CreateInfo *);
};