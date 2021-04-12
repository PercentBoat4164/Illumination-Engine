#pragma once

class VulkanRenderEngineRayTracer : public VulkanRenderEngine {
    struct AccelerationStructure {
        VkAccelerationStructureKHR handle{};
        AllocatedBuffer ASBuffer{};
    };

    class ShaderBindingTable : public AllocatedBuffer {
    public:
        VkStridedDeviceAddressRegionKHR stridedDeviceAddressRegion{};
    };
};