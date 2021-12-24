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

class IEGraphicsLink {
public:
    class IEAPI{
    public:
        std::string name{IE_RENDER_ENGINE_API_NAME_OPENGL}; // The name of the API to use. Defaults to "OpenGL"
        IEVersion version; // The version of the API to use

        /**
         * @brief Finds the highest supported API version.
         * @return An IEVersion populated with all the data about the highest supported API version
         */
        IEVersion getHighestSupportedVersion() {
        #ifdef ILLUMINATION_ENGINE_VULKAN
            if (name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
                auto vkEnumerateDeviceInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
                if (vkEnumerateDeviceInstanceVersion == nullptr) { version = IEVersion{"1.0.0"}; } else {
                    uint32_t instanceVersion;
                    vkEnumerateDeviceInstanceVersion(&instanceVersion);
                    version = IEVersion{VK_VERSION_MAJOR(instanceVersion), VK_VERSION_MINOR(instanceVersion), VK_VERSION_PATCH(instanceVersion)};
                    version.number = instanceVersion;
                }
            }
            #endif
            #ifdef ILLUMINATION_ENGINE_OPENGL
            if (name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
                // Line of if statements that finds the correct OpenGL version in use
                if (GLEW_VERSION_4_6) { version = IEVersion{"4.6.0"}; }
                else if (GLEW_VERSION_4_5) { version = IEVersion{"4.5.0"}; }
                else if (GLEW_VERSION_4_4) { version = IEVersion{"4.4.0"}; }
                else if (GLEW_VERSION_4_3) { version = IEVersion{"4.3.0"}; }
                else if (GLEW_VERSION_4_2) { version = IEVersion{"4.2.0"}; }
                else if (GLEW_VERSION_4_1) { version = IEVersion{"4.1.0"}; }
                else if (GLEW_VERSION_4_0) { version = IEVersion{"4.0.0"}; }
                else if (GLEW_VERSION_3_3) { version = IEVersion{"3.3.0"}; }
                else if (GLEW_VERSION_3_2) { version = IEVersion{"3.2.0"}; }
                else if (GLEW_VERSION_3_1) { version = IEVersion{"3.1.0"}; }
                else if (GLEW_VERSION_3_0) { version = IEVersion{"3.0.0"}; }
                else if (GLEW_VERSION_2_1) { version = IEVersion{"2.1.0"}; }
                else if (GLEW_VERSION_2_0) { version = IEVersion{"2.0.0"}; }
                else if (GLEW_VERSION_1_5) { version = IEVersion{"1.5.0"}; }
                else if (GLEW_VERSION_1_4) { version = IEVersion{"1.4.0"}; }
                else if (GLEW_VERSION_1_3) { version = IEVersion{"1.3.0"}; }
                else if (GLEW_VERSION_1_2_1) { version = IEVersion{"1.2.1"}; }
                else if (GLEW_VERSION_1_2) { version = IEVersion{"1.2.0"}; }
                else if (GLEW_VERSION_1_1) { version = IEVersion{"1.1.0"}; }
                // Alternative solution if version is still not found
                else {
                    glfwInit();
                    GLFWwindow *temporaryWindow = glfwCreateWindow(1, 1, "Gathering OpenGL Data...", nullptr, nullptr);
                    glfwMakeContextCurrent(temporaryWindow);
                    version = IEVersion{std::string(reinterpret_cast<const char *const>(glGetString(GL_VERSION)))};
                    glfwDestroyWindow(temporaryWindow);
                    glfwTerminate();
                }
                /**@todo Remove this section if it is not necessary.*/
                for (char character : version.name) {
                    if (character != '.') {
                        version.number *= 10;
                        version.number += std::stoi(std::string{character});
                    }
                }
            }
            #endif
            return version;
        }
    };
    struct PhysicalDeviceInfo {
        //Device Properties
        VkPhysicalDeviceProperties physicalDeviceProperties{};

        // Extension Properties
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
        void *pNextHighestProperty = &physicalDeviceMemoryProperties;

        // Device Features
        VkPhysicalDeviceFeatures physicalDeviceFeatures{};

        // Extension Features
        // NOTE: Ray tracing features are on the bottom of the pNext stack so that a pointer to higher up on the stack can grab only the structures supported by RenderDoc.
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT physicalDeviceDescriptorIndexingFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT};
        // All the below are ray tracing features, and cannot be loaded by RenderDoc.
        VkPhysicalDeviceAccelerationStructureFeaturesKHR physicalDeviceAccelerationStructureFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, &physicalDeviceDescriptorIndexingFeatures};
        VkPhysicalDeviceBufferDeviceAddressFeaturesEXT physicalDeviceBufferDeviceAddressFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, &physicalDeviceAccelerationStructureFeatures};
        VkPhysicalDeviceRayQueryFeaturesKHR physicalDeviceRayQueryFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR, &physicalDeviceBufferDeviceAddressFeatures};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR physicalDeviceRayTracingPipelineFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, &physicalDeviceRayQueryFeatures};
        void *pNextHighestFeature = &physicalDeviceRayTracingPipelineFeatures;
        void *pNextHighestRenderDocCompatibleFeature = &physicalDeviceDescriptorIndexingFeatures;

        //Engine Features
        VkBool32 anisotropicFiltering{false};
        VkBool32 msaaSmoothing{false};
        VkBool32 rayTracing{false};
    };

    IESettings settings{};
    IEAPI api{};
    vkb::Device device{};
    vkb::Swapchain swapchain{};
    vkb::Instance instance{};
    VkSurfaceKHR surface{};
    VkCommandPool commandPool{};
    VmaAllocator allocator{};
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};
    VkQueue transferQueue{};
    VkQueue computeQueue{};
    std::vector<VkImageView> swapchainImageViews{};
    PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddressKHR{};
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR{};
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR{};
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR{};
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR{};
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR{};
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKhr{};
    PhysicalDeviceInfo supportedPhysicalDeviceInfo{};
    PhysicalDeviceInfo enabledPhysicalDeviceInfo{};

    void build(bool renderDocCapturing = false) {
        vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device.device, "vkGetBufferDeviceAddressKHR"));
        vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device.device, "vkCmdBuildAccelerationStructuresKHR"));
        vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device.device, "vkCreateAccelerationStructureKHR"));
        vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device.device, "vkDestroyAccelerationStructureKHR"));
        vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device.device, "vkGetAccelerationStructureBuildSizesKHR"));
        vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device.device, "vkGetAccelerationStructureDeviceAddressKHR"));
        vkAcquireNextImageKhr = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(device.device, "vkAcquireNextImageKHR"));
        VkPhysicalDeviceProperties2 physicalDeviceProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        vkGetPhysicalDeviceProperties2(device.physical_device.physical_device, &physicalDeviceProperties);
        supportedPhysicalDeviceInfo.physicalDeviceProperties = physicalDeviceProperties.properties;
        VkPhysicalDeviceFeatures2 physicalDeviceFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        physicalDeviceFeatures.pNext = renderDocCapturing ? supportedPhysicalDeviceInfo.pNextHighestRenderDocCompatibleFeature : supportedPhysicalDeviceInfo.pNextHighestFeature;
        vkGetPhysicalDeviceFeatures2(device.physical_device.physical_device, &physicalDeviceFeatures);
        supportedPhysicalDeviceInfo.physicalDeviceFeatures = physicalDeviceFeatures.features;
        vkGetPhysicalDeviceMemoryProperties(device.physical_device.physical_device, &supportedPhysicalDeviceInfo.physicalDeviceMemoryProperties);
    }

    [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const {
        VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device.device, &allocInfo, &commandBuffer);
        VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const {
        vkEndCommandBuffer(commandBuffer);
        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);
        vkFreeCommandBuffers(device.device, commandPool, 1, &commandBuffer);
    }

    VkBool32 enableFeature(VkBool32 *feature) {
        *feature = *testFeature(feature);
        return *feature;
    }

    std::vector<VkBool32> enableFeature(const std::vector<VkBool32 *> &features) {
        std::vector<VkBool32> results{};
        results.reserve(static_cast<unsigned int>(features.size() + 1));
        results.push_back(VK_TRUE);
        for (VkBool32 *feature : features) { results.push_back(enableFeature(feature)); }
        for (VkBool32 result : results) {
            if (!result) {
                results[0] = VK_FALSE;
                break;
            }
        }
        return results;
    }

    VkBool32 *testFeature(const VkBool32 *feature) {
        return feature - (VkBool32 *)&enabledPhysicalDeviceInfo + (VkBool32 *)&supportedPhysicalDeviceInfo;
    }

    std::vector<VkBool32> testFeature(const std::vector<VkBool32 *> &features) {
        std::vector<VkBool32> results{};
        results.reserve(static_cast<unsigned int>(features.size() + 1));
        results.push_back(VK_TRUE);
        for (VkBool32 *feature : features) { results.push_back(*testFeature(feature)); }
        for (VkBool32 result : results) {
            if (!result) {
                results[0] = VK_FALSE;
                break;
            }
        }
        return results;
    }
};