#pragma once

/* Define macros used throughout the file. */
#define IE_ENGINE_FEATURE_RAY_QUERY_RAY_TRACING "RayQueryRayTracing"
#define IE_ENGINE_FEATURE_QUERY_VARIABLE_DESCRIPTOR_COUNT "VariableDescriptorCount"

/* Predefine classes used with pointers or as return values for functions. */
class GLFWwindow;

class GLFWmonitor;

class IERenderable;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IEAPI.hpp"
#include "IECamera.hpp"
#include "IERenderPass.hpp"
#include "IESettings.hpp"

#include "Buffer/IEAccelerationStructure.hpp"

#include "CommandBuffer/IECommandPool.hpp"

#include "Image/IEFramebuffer.hpp"
#include "Image/IETexture.hpp"

// External dependencies
#include <vk_mem_alloc.h>

#include <VkBootstrap.h>

#include <vulkan/vulkan.h>

// System dependencies
#include <string>
#include <vector>
#include <algorithm>


class IERenderEngine {
private:
    /**
     * @brief Private helper function that creates a Vulkan instance.
     * @return The newly created Vulkan instance.
     */
    vkb::Instance createVulkanInstance();

    /**
     * @brief Creates a window using hardcoded hints and settings from settings.
     * @return The window that was just created.
     */
    GLFWwindow* createWindow() const;

    /**
     * @brief Changes the window icon to all of the images found in [path] with a name that contains last part of [path].
     * @param path string of path to icons folder plus the identifier.
     */
    void setWindowIcons(const std::string& path) const;

    /**
     * @brief Creates a VkSurface for the window.
     * @return The newly created surface.
     */
    VkSurfaceKHR createWindowSurface();

    /**
     * @brief Creates a physical and logical device with the extensions and features provided activated.
     * @param desiredExtensions vector of vectors of Vulkan extension names.
     * @param desiredExtensionFeatures pointer to pNext chain of desired features.
     * @return The newly created vkb::Device.
     */
    vkb::Device setUpDevice(std::vector<std::vector<const char *>>* desiredExtensions=nullptr, void* desiredExtensionFeatures=nullptr);

    /**
     * @brief Creates a VmaAllocator with no flags activated.
     * @return The newly created VmaAllocator.
     */
    VmaAllocator setUpGPUMemoryAllocator();

    /**
     * @brief Creates a new swapchain. Uses the old swapchain if [useOldSwapchain] is true.
     * @param useOldSwapchain
     * @return The newly created swapchain.
     */
    vkb::Swapchain createSwapchain(bool useOldSwapchain=true);

    /**
     * @brief Creates the basic sync objects. Recreates them if the already exist.
     */
    void createSyncObjects();

    /**
     * @brief Initializes all the command pools in  Each one is set to use its respective queue.
     */
    void createCommandPools();

    IEAPI *autoDetectAPI();

    /**
     * @brief Replaces the only IEGraphicsLink::build() function.
     */
    void buildFunctionPointers();

    /**
     * @brief Destroys all the sync objects required by the program.
     */
    void destroySyncObjects();

    /**
     * @brief Destroys the swapchain.
     */
    void destroySwapchain();

    /**
     * Destroys the command pools and all associated command buffers.
     */
    void destroyCommandPools();

public:
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

        bool queryFeatureSupport(const std::string& feature);

        bool queryExtensionSupport(const std::string& extension);

        std::vector<const char *> queryEngineFeatureExtensionRequirements(const std::string& engineFeature, IEAPI* API);

        ExtensionAndFeatureInfo();

    private:
        std::unordered_map<std::string, std::vector<std::vector<const char *>>> engineFeatureExtensionRequirementQueries {
                {  // ray query ray tracing extensions
                        IE_ENGINE_FEATURE_RAY_QUERY_RAY_TRACING, {
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
                                },
                                {  // 1.3
                                        VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
                                        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                                        VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                                        VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                                        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                                        VK_KHR_RAY_QUERY_EXTENSION_NAME,
                                        VK_KHR_SPIRV_1_4_EXTENSION_NAME
                                }
                        },

                }
        };

        std::unordered_map<std::string, bool(ExtensionAndFeatureInfo::*)()const> extensionAndFeatureSupportQueries {
                {IE_ENGINE_FEATURE_RAY_QUERY_RAY_TRACING, &ExtensionAndFeatureInfo::rayTracingWithRayQuerySupportQuery},
                {IE_ENGINE_FEATURE_QUERY_VARIABLE_DESCRIPTOR_COUNT, &ExtensionAndFeatureInfo::variableDescriptorCountSupportQuery}
        };

        bool rayTracingWithRayQuerySupportQuery() const;

        bool variableDescriptorCountSupportQuery() const;
    };

    explicit IERenderEngine(IESettings *settings = new IESettings{});

    void loadRenderable(IERenderable* renderable);

    bool update();

    void reloadRenderables();

    void handleFullscreenSettingsChange();

    static std::string translateVkResultCodes(VkResult result);

    IEBuffer *createGlobalBuffer(IEBuffer::CreateInfo *createInfo) {
        globalBuffers.emplace_back(this, createInfo);
        return &globalBuffers[globalBuffers.size()];
    }

    IEBuffer *registerGlobalBuffer(IEBuffer buffer) {
        globalBuffers.push_back(buffer);
        buffer.destroy(false);
        return &globalBuffers[globalBuffers.size()];
    }

    IEImage *createGlobalImage(IEImage::CreateInfo *createInfo) {
        globalImages.emplace_back(this, createInfo);
        return &globalImages[globalImages.size()];
    }

    void destroy();

    ~IERenderEngine();

    GLFWmonitor *monitor{};
    GLFWwindow *window{};
    IECamera camera{};
    IERenderPass renderPass{};
    std::vector<IETexture> textures{1};
    IECommandPool graphicsCommandPool{};
    IECommandPool presentCommandPool{};
    IECommandPool transferCommandPool{};
    IECommandPool computeCommandPool{};
    float frameTime{};
    int frameNumber{};

    // Stuff from GraphicsLink
    IESettings *settings;
    IEAPI api;
    vkb::Device device{};
    vkb::Swapchain swapchain{};
    vkb::Instance instance{};
    vkb::detail::Result<vkb::SystemInfo> systemInfo{vkb::SystemInfo::get_system_info()};
    VkSurfaceKHR surface{};
    VmaAllocator allocator{};
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};
    VkQueue transferQueue{};
    VkQueue computeQueue{};
    ExtensionAndFeatureInfo extensionAndFeatureInfo{};
    std::vector<IEBuffer> globalBuffers{};
    std::vector<IEImage> globalImages{};
    std::vector<VkImageView> swapchainImageViews{};
    PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddressKHR{};
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR{};
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR{};
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR{};
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR{};
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR{};
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKhr{};


private:
    VkTransformMatrixKHR identityTransformMatrix{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
    std::vector<VkFence> inFlightFences{};
    std::vector<VkFence> imagesInFlight{};
    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<IEFramebuffer> framebuffers{};
    std::vector<IERenderable*> renderables{};
    IEAccelerationStructure topLevelAccelerationStructure{};
    std::vector<std::function<void()>> fullRecreationDeletionQueue{};
    std::vector<std::function<void()>> recreationDeletionQueue{};
    std::vector<std::function<void()>> deletionQueue{};
    size_t currentFrame{};
    bool framebufferResized{false};
    float previousTime{};

    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height);
};