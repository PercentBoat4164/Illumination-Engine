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
    VkAccelerationStructureKHR accelerationStructure{};

    IEAccelerationStructure();

    void create(IERenderEngine *renderEngineLink, CreateInfo *createInfo) override;
};