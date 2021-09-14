#pragma once

#include <string>
#include <vector>

#ifdef ILLUMINATION_ENGINE_VULKAN
#include <vulkan/vulkan.h>
#endif

#ifdef ILLUMINATION_ENGINE_OPENGL
#ifndef GLEW_IMPLEMENTATION
#define GLEW_IMPLEMENTATION
#include <GL/glew.h>
#endif
#endif

#include <VkBootstrap.h>
#include <vk_mem_alloc.h>
#include <GLFW/glfw3.h>
#include <log4cplus/logger.h>

#include "Settings.hpp"

class RenderEngineLink{
public:
    class API{
    public:
        std::string name{"OpenGL"};
        Version version{};

        Version getVersion() {
#ifdef ILLUMINATION_ENGINE_VULKAN
            if (name == "Vulkan") {
                auto vkEnumerateDeviceInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
                if (vkEnumerateDeviceInstanceVersion == nullptr) { version = Version{"1.0.0"}; } else {
                    uint32_t instanceVersion;
                    vkEnumerateDeviceInstanceVersion(&instanceVersion);
                    version = Version{VK_VERSION_MAJOR(instanceVersion), VK_VERSION_MINOR(instanceVersion), VK_VERSION_PATCH(instanceVersion)};
                }
            }
#endif
#ifdef ILLUMINATION_ENGINE_VULKAN
            if (name == "OpenGL") {
                if (GLEW_VERSION_1_1) { version = Version{"1.1.0"}; }
                if (GLEW_VERSION_1_2) { version = Version{"1.2.0"}; }
                if (GLEW_VERSION_1_2_1) { version = Version{"1.2.1"}; }
                if (GLEW_VERSION_1_3) { version = Version{"1.3.0"}; }
                if (GLEW_VERSION_1_4) { version = Version{"1.4.0"}; }
                if (GLEW_VERSION_1_5) { version = Version{"1.5.0"}; }
                if (GLEW_VERSION_2_0) { version = Version{"2.0.0"}; }
                if (GLEW_VERSION_2_1) { version = Version{"2.1.0"}; }
                if (GLEW_VERSION_3_0) { version = Version{"3.0.0"}; }
                if (GLEW_VERSION_3_1) { version = Version{"3.1.0"}; }
                if (GLEW_VERSION_3_2) { version = Version{"3.2.0"}; }
                if (GLEW_VERSION_3_3) { version = Version{"3.3.0"}; }
                if (GLEW_VERSION_4_0) { version = Version{"4.0.0"}; }
                if (GLEW_VERSION_4_1) { version = Version{"4.1.0"}; }
                if (GLEW_VERSION_4_2) { version = Version{"4.2.0"}; }
                if (GLEW_VERSION_4_3) { version = Version{"4.3.0"}; }
                if (GLEW_VERSION_4_4) { version = Version{"4.4.0"}; }
                if (GLEW_VERSION_4_5) { version = Version{"4.5.0"}; }
                if (GLEW_VERSION_4_6) { version = Version{"4.6.0"}; }
                if (version.name == "0.0.0") {
                    std::cerr << "GLEW was not initialized before call to RenderEngineLink::API::getVersion()\n\tAttempting recovery" << std::endl;
                    if (!glfwInit()) { std::cerr << "\tGLFW failed to initialize\n\tStopping attempt to recover from previous error" << std::endl; }
                    GLFWwindow *temporaryWindow = glfwCreateWindow(1, 1, "Gathering OpenGL Data...", nullptr, nullptr);
                    glfwMakeContextCurrent(temporaryWindow);
                    version = Version{std::string(reinterpret_cast<const char *const>(glGetString(GL_VERSION)))};
                    glfwDestroyWindow(temporaryWindow);
                    glfwTerminate();
                    std::cerr << "\tRecovery successful" << std::endl;
                }
            }
#endif
            return version;
        }
    };

    class PhysicalDevice {
    public:
        struct Info {
#ifdef ILLUMINATION_ENGINE_VULKAN
            //Device Properties
            VkPhysicalDeviceProperties properties{};

            // Extension Properties
            VkPhysicalDeviceMemoryProperties memoryProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
            void *pNextHighestProperty = &memoryProperties;

            // Device Features
            VkPhysicalDeviceFeatures physicalDeviceFeatures{};

            // Extension Features
            // NOTE: Ray tracing features are on the bottom of the pNext stack so that a pointer to higher up on the stack can grab only the structures supported by RenderDoc.
            VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT};
            // All the below are ray tracing features, and cannot be loaded by RenderDoc.
            VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, &descriptorIndexingFeatures};
            VkPhysicalDeviceBufferDeviceAddressFeaturesEXT bufferDeviceAddressFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, &accelerationStructureFeatures};
            VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR, &bufferDeviceAddressFeatures};
            VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, &rayQueryFeatures};
            // Automatically checks for RenderDoc capture, and simulates a device that does not support anything that RenderDoc does not support.
            void *pNextHighestFeature = std::getenv("ENABLE_VULKAN_RENDERDOC_CAPTURE") != nullptr ? static_cast<void *>(&descriptorIndexingFeatures) : static_cast<void *>(&rayTracingPipelineFeatures);
#endif
            // Engine Features
            bool anisotropicFiltering{false};
            bool msaaSmoothing{false};
            bool rayTracing{false};
        };

        Info enabledInfo;
        Info supportedInfo;

#ifdef ILLUMINATION_ENGINE_VULKAN

        VkBool32 enableFeature(VkBool32 *feature) {
            *feature = *testFeature(feature);
            return *feature;
        }

        std::vector<VkBool32> enableFeature(const std::vector<VkBool32 *> &features) {
            std::vector<VkBool32> results{VK_FALSE};
            results.reserve(static_cast<unsigned int>(features.size() + 1));
            results.push_back(VK_FALSE);
            for (VkBool32 *feature: features) { results.push_back(enableFeature(feature)); }
            for (VkBool32 result: results) { if (!result) { return results; }}
            results[0] = VK_TRUE;
            return results;
        }

        VkBool32 *testFeature(const VkBool32 *feature) {
            return feature - (VkBool32 *)&enabledInfo + (VkBool32 *)&supportedInfo;
        }

        std::vector<VkBool32> testFeature(const std::vector<VkBool32 *> &features) {
            std::vector<VkBool32> results{VK_FALSE};
            results.reserve(static_cast<unsigned int>(features.size() + 1));
            results.push_back(VK_FALSE);
            for (VkBool32 *feature: features) { results.push_back(*testFeature(feature)); }
            for (VkBool32 result: results) { if (!result) { return results; }}
            results[0] = VK_TRUE;
            return results;
        }
#endif
    };

    void create() {
#ifdef ILLUMINATION_ENGINE_VULKAN
        if (api.name == "Vulkan") {
            vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device.device, "vkGetBufferDeviceAddressKHR"));
            vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device.device, "vkCmdBuildAccelerationStructuresKHR"));
            vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device.device, "vkCreateAccelerationStructureKHR"));
            vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device.device, "vkDestroyAccelerationStructureKHR"));
            vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device.device, "vkGetAccelerationStructureBuildSizesKHR"));
            vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device.device, "vkGetAccelerationStructureDeviceAddressKHR"));
            vkAcquireNextImageKhr = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(device.device, "vkAcquireNextImageKHR"));
            VkPhysicalDeviceProperties2 physicalDeviceProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
            vkGetPhysicalDeviceProperties2(device.physical_device.physical_device, &physicalDeviceProperties);
            physicalDevice.supportedInfo.properties = physicalDeviceProperties.properties;
            VkPhysicalDeviceFeatures2 physicalDeviceFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
            physicalDeviceFeatures.pNext = physicalDevice.supportedInfo.pNextHighestFeature;
            vkGetPhysicalDeviceFeatures2(device.physical_device.physical_device, &physicalDeviceFeatures);
            physicalDevice.supportedInfo.physicalDeviceFeatures = physicalDeviceFeatures.features;
            vkGetPhysicalDeviceMemoryProperties(device.physical_device.physical_device, &physicalDevice.supportedInfo.memoryProperties);
        }
#endif
    }

    API api;
    PhysicalDevice physicalDevice;
    Settings settings;
    GLFWwindow *window{};
    log4cplus::Logger graphicsModuleLogger;
#ifdef ILLUMINATION_ENGINE_VULKAN
    vkb::Device device{};
    vkb::Instance instance{};
    PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddressKHR{};
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR{};
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR{};
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR{};
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR{};
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR{};
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKhr{};
#endif

    ~RenderEngineLink() {
#ifdef ILLUMINATION_ENGINE_VULKAN
        if (api.name == "Vulkan") {
            vkb::destroy_device(device);
            vkb::destroy_instance(instance);
        }
#endif
    }
};