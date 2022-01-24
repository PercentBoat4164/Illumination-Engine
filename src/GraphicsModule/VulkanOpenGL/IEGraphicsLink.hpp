#pragma once

#include "IESettings.hpp"
#include "IEVersion.hpp"

#include <VkBootstrap.h>

#ifndef VMA_INCLUDED
#define VMA_INCLUDED
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#endif

#include <vector>

#define IE_RENDER_ENGINE_API_NAME_VULKAN "Vulkan"
#define IE_RENDER_ENGINE_API_NAME_OPENGL "OpenGL"

#define IE_ENGINE_FEATURE_RAY_QUERY_RAY_TRACING "RayQueryRayTracing"
#define IE_ENGINE_FEATURE_QUERY_VARIABLE_DESCRIPTOR_COUNT "VariableDescriptorCount"

class IECommandPool;

class IERenderPass;

class IETexture;

class IEGraphicsLink {
public:
    class IEAPI{
    public:
        explicit IEAPI(const std::string& apiName) {
            name = apiName;
        }

        IEAPI() = default;

        std::string name{}; // The name of the API to use
        IEVersion version{}; // The version of the API to use

        /**
         * @brief Finds the highest supported API version.
         * @return An IEVersion populated with all the data about the highest supported API version
         */
        IEVersion getHighestSupportedVersion(IEGraphicsLink* linkedRenderEngine) const {
            IEVersion temporaryVersion;
            #ifdef ILLUMINATION_ENGINE_VULKAN
            if (name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(linkedRenderEngine->device.physical_device, &properties);
                temporaryVersion = IEVersion(properties.apiVersion);
            }
            #endif
            #ifdef ILLUMINATION_ENGINE_OPENGL
            if (name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
                // Line of if statements that finds the correct OpenGL version in use
                if (GLEW_VERSION_4_6) { temporaryVersion = IEVersion{"4.6.0"}; }
                else if (GLEW_VERSION_4_5) { temporaryVersion = IEVersion{"4.5.0"}; }
                else if (GLEW_VERSION_4_4) { temporaryVersion = IEVersion{"4.4.0"}; }
                else if (GLEW_VERSION_4_3) { temporaryVersion = IEVersion{"4.3.0"}; }
                else if (GLEW_VERSION_4_2) { temporaryVersion = IEVersion{"4.2.0"}; }
                else if (GLEW_VERSION_4_1) { temporaryVersion = IEVersion{"4.1.0"}; }
                else if (GLEW_VERSION_4_0) { temporaryVersion = IEVersion{"4.0.0"}; }
                else if (GLEW_VERSION_3_3) { temporaryVersion = IEVersion{"3.3.0"}; }
                else if (GLEW_VERSION_3_2) { temporaryVersion = IEVersion{"3.2.0"}; }
                else if (GLEW_VERSION_3_1) { temporaryVersion = IEVersion{"3.1.0"}; }
                else if (GLEW_VERSION_3_0) { temporaryVersion = IEVersion{"3.0.0"}; }
                else if (GLEW_VERSION_2_1) { temporaryVersion = IEVersion{"2.1.0"}; }
                else if (GLEW_VERSION_2_0) { temporaryVersion = IEVersion{"2.0.0"}; }
                else if (GLEW_VERSION_1_5) { temporaryVersion = IEVersion{"1.5.0"}; }
                else if (GLEW_VERSION_1_4) { temporaryVersion = IEVersion{"1.4.0"}; }
                else if (GLEW_VERSION_1_3) { temporaryVersion = IEVersion{"1.3.0"}; }
                else if (GLEW_VERSION_1_2_1) { temporaryVersion = IEVersion{"1.2.1"}; }
                else if (GLEW_VERSION_1_2) { temporaryVersion = IEVersion{"1.2.0"}; }
                else if (GLEW_VERSION_1_1) { temporaryVersion = IEVersion{"1.1.0"}; }
                // If none of these is active then GLEW something is wrong.
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "No OpenGL version was found!");
            }
            #endif
            return temporaryVersion;
        }
    };

    struct ExtensionAndFeatureInfo {
        // Extension Features
        // NOTE: Ray tracing features are on the bottom of the pNext stack so that a pointer to higher up on the stack can grab only the structures supported by RenderDoc.
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT};
        // All the below are ray tracing features, and cannot be loaded by RenderDoc.
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, &descriptorIndexingFeatures};
        VkPhysicalDeviceBufferDeviceAddressFeaturesEXT bufferDeviceAddressFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, &accelerationStructureFeatures};
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR, &bufferDeviceAddressFeatures};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, &rayQueryFeatures};
        void *pNextHighestFeature = &rayTracingPipelineFeatures;
        void *pNextHighestRenderDocCompatibleFeature = &descriptorIndexingFeatures;

        std::vector<std::string> supportedExtensions;

        bool queryFeatureSupport(const std::string& feature) {
            return (this->*extensionAndFeatureSupportQueries[feature])();
        }

        bool queryExtensionSupport(const std::string& extension) {
            return std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end();
        }

        std::vector<const char *> queryEngineFeatureExtensionRequirements(const std::string& engineFeature, IEAPI* API) {
            if (API->name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
                return engineFeatureExtensionRequirementQueries[engineFeature][API->version.minor];
            }
            return {};
        }

        ExtensionAndFeatureInfo() {
            uint32_t count;
            vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());
            supportedExtensions.reserve(count);
            for (VkExtensionProperties extension : extensions) {
                supportedExtensions.emplace_back(extension.extensionName);
            }
        }

    private:
        std::unordered_map<std::string, std::vector<std::vector<const char *>>> engineFeatureExtensionRequirementQueries {
                {IE_ENGINE_FEATURE_RAY_QUERY_RAY_TRACING, { // ray query ray tracing extensions
                        {  // 1.0

                        },
                        {  // 1.1

                        },
                        {  // 1.2
                            VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
                            VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                            VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                            VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                            VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                            VK_KHR_RAY_QUERY_EXTENSION_NAME,
                            VK_KHR_SPIRV_1_4_EXTENSION_NAME
                        }
                    }
                }
        };

        std::unordered_map<std::string, bool(ExtensionAndFeatureInfo::*)()const> extensionAndFeatureSupportQueries {
                {IE_ENGINE_FEATURE_RAY_QUERY_RAY_TRACING, &ExtensionAndFeatureInfo::rayTracingWithRayQuerySupportQuery},
                {IE_ENGINE_FEATURE_QUERY_VARIABLE_DESCRIPTOR_COUNT, &ExtensionAndFeatureInfo::variableDescriptorCountSupportQuery}
        };

        bool rayTracingWithRayQuerySupportQuery() const {
            return bufferDeviceAddressFeatures.bufferDeviceAddress & accelerationStructureFeatures.accelerationStructure & rayQueryFeatures.rayQuery & rayTracingPipelineFeatures.rayTracingPipeline;
        }

        bool variableDescriptorCountSupportQuery() const {
            return descriptorIndexingFeatures.descriptorBindingVariableDescriptorCount;
        }
    };

    IESettings settings{};
    IEAPI api;
    std::vector<IETexture>* textures;
    IERenderPass* renderPass{};
    vkb::Device device{};
    vkb::Swapchain swapchain{};
    vkb::Instance instance{};
    vkb::detail::Result<vkb::SystemInfo> systemInfo = vkb::SystemInfo::get_system_info();
    VkSurfaceKHR surface{};
    VmaAllocator allocator{};
    IECommandPool* graphicsCommandPool{};
    IECommandPool* presentCommandPool{};
    IECommandPool* transferCommandPool{};
    IECommandPool* computeCommandPool{};
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};
    VkQueue transferQueue{};
    VkQueue computeQueue{};
    std::vector<std::function<void()>> deletionQueue{};
    ExtensionAndFeatureInfo extensionAndFeatureInfo{};
    std::vector<VkImageView> swapchainImageViews{};
    PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddressKHR{};
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR{};
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR{};
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR{};
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR{};
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR{};
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKhr{};

    void build() {
        vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device.device, "vkGetBufferDeviceAddressKHR"));
        vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device.device, "vkCmdBuildAccelerationStructuresKHR"));
        vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device.device, "vkCreateAccelerationStructureKHR"));
        vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device.device, "vkDestroyAccelerationStructureKHR"));
        vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device.device, "vkGetAccelerationStructureBuildSizesKHR"));
        vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device.device, "vkGetAccelerationStructureDeviceAddressKHR"));
        vkAcquireNextImageKhr = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(device.device, "vkAcquireNextImageKHR"));
        graphicsQueue = device.get_queue(vkb::QueueType::graphics).value();
        presentQueue = device.get_queue(vkb::QueueType::present).value();
        transferQueue = device.get_queue(vkb::QueueType::transfer).value();
        computeQueue = device.get_queue(vkb::QueueType::compute).value();
        api = IEAPI{IE_RENDER_ENGINE_API_NAME_VULKAN};
        api.version = api.getHighestSupportedVersion(this);
    }

    void destroy() {
        for (std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

    ~IEGraphicsLink() {
        destroy();
    }
};