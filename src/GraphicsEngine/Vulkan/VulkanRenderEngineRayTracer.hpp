#pragma once

#include <vector>

#include "BufferManager.hpp"
#include "GPUData.hpp"
#include "Asset.hpp"
#include "VulkanRenderEngine.hpp"
#include "AccelerationStructureManager.hpp"

class VulkanRenderEngineRayTracer : public VulkanRenderEngine {
private:
    struct ShaderBindingTables {
        BufferManager rayGen{};
        BufferManager miss{};
        BufferManager hit{};
        BufferManager callable{};
    };

public:
    AccelerationStructureManager bottomLevelAccelerationStructure{};
    AccelerationStructureManager topLevelAccelerationStructure{};
    ShaderBindingTables shaderBindingTables{};
    UniformBufferObject uniformBufferObject{};
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
    std::vector<Asset *> assets{};
};