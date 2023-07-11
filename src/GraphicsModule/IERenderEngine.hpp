#pragma once

/* Define macros used throughout the file. */
#include <type_traits>
#define IE_ENGINE_FEATURE_RAY_QUERY_RAY_TRACING           "RayQueryRayTracing"
#define IE_ENGINE_FEATURE_QUERY_VARIABLE_DESCRIPTOR_COUNT "VariableDescriptorCount"

/* Predefine classes used with pointers or as return values for functions. */
struct GLFWwindow;

struct GLFWmonitor;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "CommandBuffer/IECommandPool.hpp"
#include "Core/Core.hpp"
#include "IEAPI.hpp"
#include "IECamera.hpp"
#include "IESettings.hpp"
#include "Image/IETexture.hpp"
#include "Renderable/IERenderable.hpp"
#include "RenderPass/IEFramebuffer.hpp"
#include "RenderPass/IERenderPass.hpp"

// External dependencies
#include <VkBootstrap.h>
#include <vulkan/vulkan.h>

// System dependencies
#include <algorithm>
#include <string>
#include <vector>

class IERenderEngine : public IE::Core::Engine {
public:
    using AspectType = IERenderable;

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
    GLFWwindow * createWindow() const;

    /**
     * @brief Changes the window icon to all of the images found in [m_path] with a
     * m_name that contains last part of
     * [m_path].
     * @param m_path string of m_path to icons folder plus the identifier.
     */
    void setWindowIcons(const std::filesystem::path &path) const;

    /**
     * @brief Creates a VkSurface for the window.
     * @return The newly created surface.
     */
    VkSurfaceKHR createWindowSurface();

    /**
     * @brief Creates a physical and logical device with the extensions and
     * features provided activated.
     * @param desiredExtensions vector of vectors of Vulkan m_extension names.
     * @param desiredExtensionFeatures pointer to pNext chain of desired features.
     * @return The newly created vkb::Device.
     */
    vkb::Device setUpDevice(
      std::vector<std::vector<const char *>> *desiredExtensions        = nullptr,
      void                                   *desiredExtensionFeatures = nullptr
    );

    /**
     * @brief Creates a VmaAllocator with no t_flags activated.
     * @return The newly created VmaAllocator.
     */
    VmaAllocator setUpGPUMemoryAllocator();

    /**
     * @brief Creates a new swapchain. Uses the old swapchain if [useOldSwapchain]
     * is true.
     * @param useOldSwapchain
     * @return The newly created swapchain.
     */
    vkb::Swapchain createSwapchain(bool useOldSwapchain = true);

    /**
     * @brief Creates the basic sync objects. Recreates them if the already exist.
     */
    void createSyncObjects();

    /**
     * @brief Initializes all the command pools in  Each one is set to use its
     * respective queue.
     */
    void createCommandPools();

    IEAPI *autoDetectAPIVersion(const std::string &api);

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
        // NOTE: Ray tracing features are on the bottom of the pNext stack so that a
        // pointer to higher up on the
        // stack can grab only the structures supported
        // by RenderDoc.
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT descriptorIndexingFeatures{
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT};
        // All the below are ray tracing features, and cannot be loaded by
        // RenderDoc.
        VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
          &descriptorIndexingFeatures};
        VkPhysicalDeviceBufferDeviceAddressFeaturesEXT bufferDeviceAddressFeatures{
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES,
          &accelerationStructureFeatures};
        VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures{
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR,
          &bufferDeviceAddressFeatures};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{
          VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
          &rayQueryFeatures};
        void *pNextHighestFeature                    = &rayTracingPipelineFeatures;
        void *pNextHighestRenderDocCompatibleFeature = &descriptorIndexingFeatures;

        std::vector<std::string> supportedExtensions;

        bool queryFeatureSupport(const std::string &feature);

        bool queryExtensionSupport(const std::string &extension);

        std::vector<const char *>
        queryEngineFeatureExtensionRequirements(const std::string &engineFeature, IEAPI *API);

        ExtensionAndFeatureInfo();

    private:
        std::unordered_map<std::string, std::vector<std::vector<const char *>>>
          engineFeatureExtensionRequirementQueries{
            {
             // ray query ray tracing extensions
              IE_ENGINE_FEATURE_RAY_QUERY_RAY_TRACING, {{
                 // 1.0

               },
               {
                 // 1.1

               },
               {// 1.2
                VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
                VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                VK_KHR_RAY_QUERY_EXTENSION_NAME,
                VK_KHR_SPIRV_1_4_EXTENSION_NAME},
               {// 1.3
                VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
                VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
                VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
                VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
                VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
                VK_KHR_RAY_QUERY_EXTENSION_NAME,
                VK_KHR_SPIRV_1_4_EXTENSION_NAME}},

             }
        };

        std::unordered_map<std::string, bool (ExtensionAndFeatureInfo::*)() const>
          extensionAndFeatureSupportQueries{
            {IE_ENGINE_FEATURE_RAY_QUERY_RAY_TRACING,
             &ExtensionAndFeatureInfo::rayTracingWithRayQuerySupportQuery },
            {IE_ENGINE_FEATURE_QUERY_VARIABLE_DESCRIPTOR_COUNT,
             &ExtensionAndFeatureInfo::variableDescriptorCountSupportQuery}
        };

        bool rayTracingWithRayQuerySupportQuery() const;

        bool variableDescriptorCountSupportQuery() const;
    };

    void toggleFullscreen();

    static std::string translateVkResultCodes(VkResult result);

    ~IERenderEngine();

    IECamera                                       camera{};
    std::shared_ptr<IERenderPass>                  renderPass{};
    IESettings                                    *settings;
    std::shared_ptr<IECommandPool>                 graphicsCommandPool{};
    std::shared_ptr<IECommandPool>                 presentCommandPool{};
    std::shared_ptr<IECommandPool>                 transferCommandPool{};
    std::shared_ptr<IECommandPool>                 computeCommandPool{};
    IEAPI                                          API;
    ExtensionAndFeatureInfo                        extensionAndFeatureInfo{};
    GLFWmonitor                                   *monitor{};
    GLFWwindow                                    *window{};
    vkb::Device                                    device{};
    vkb::Swapchain                                 swapchain{};
    vkb::Instance                                  instance{};
    vkb::Result<vkb::SystemInfo>                   systemInfo{vkb::SystemInfo::get_system_info()};
    VkSurfaceKHR                                   surface{};
    VmaAllocator                                   allocator{};
    VkQueue                                        graphicsQueue{};
    VkQueue                                        presentQueue{};
    VkQueue                                        transferQueue{};
    VkQueue                                        computeQueue{};
    PFN_vkGetBufferDeviceAddress                   vkGetBufferDeviceAddressKHR{};
    PFN_vkCmdBuildAccelerationStructuresKHR        vkCmdBuildAccelerationStructuresKHR{};
    PFN_vkCreateAccelerationStructureKHR           vkCreateAccelerationStructureKHR{};
    PFN_vkDestroyAccelerationStructureKHR          vkDestroyAccelerationStructureKHR{};
    PFN_vkGetAccelerationStructureBuildSizesKHR    vkGetAccelerationStructureBuildSizesKHR{};
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR{};
    PFN_vkAcquireNextImageKHR                      vkAcquireNextImageKhr{};
    std::vector<std::shared_ptr<IETexture>>        textures{};
    std::vector<VkImageView>                       swapchainImageViews{};
    std::vector<std::weak_ptr<IERenderable>>       renderables{};
    float                                          frameTime{};
    int                                            frameNumber{};
    // global depth image used by all framebuffers. Should this be here?
    std::shared_ptr<IEImage>                       depthImage{};

    void addAsset(const std::shared_ptr<IE::Core::Asset> &asset);

    bool update();

    static IE::Core::Threading::Task<std::shared_ptr<IERenderEngine>>
    Factory(const std::string &t_id, IESettings &t_settings);

    static IE::Core::Threading::Task<std::shared_ptr<IERenderEngine>> Factory(const std::string &t_id, IESettings *t_settings);


private:
    std::vector<VkFence>               inFlightFences{};
    std::vector<VkFence>               imagesInFlight{};
    std::vector<VkSemaphore>           imageAvailableSemaphores{};
    std::vector<VkSemaphore>           renderFinishedSemaphores{};
    std::vector<std::function<void()>> fullRecreationDeletionQueue{};
    std::vector<std::function<void()>> recreationDeletionQueue{};
    std::vector<std::function<void()>> renderableDeletionQueue{};
    std::vector<std::function<void()>> deletionQueue{};
    size_t                             currentFrame{};
    bool                               framebufferResized{settings->fullscreen};
    float                              previousTime{};
    bool                               shouldBeFullscreen{settings->fullscreen};

    explicit IERenderEngine(const std::string &t_id, IESettings &settings);

    explicit IERenderEngine(const std::string &t_id, IESettings *t_settings);

    static std::function<bool(IERenderEngine &)> _update;

    bool _openGLUpdate();

    bool _vulkanUpdate();


    static std::function<void(IERenderEngine &)> _destroy;

    void _openGLDestroy();

    void _vulkanDestroy();

    void destroy();


    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height);

    void createRenderPass();

    void handleResolutionChange();

    static void windowPositionCallback(GLFWwindow *pWindow, int x, int y);

public:
    static void APIENTRY glDebugOutput(
      GLenum       source,
      GLenum       type,
      unsigned int id,
      GLenum       severity,
      GLsizei      length,
      const char  *message,
      const void  *userParam
    );

    static void setAPI(const IEAPI &API);

    template<typename T>
    auto createAspect(const std::string &t_id, IE::Core::File *t_resource) -> std::shared_ptr<T> {
        IE::Core::getLogger().log(
          "Cannot create Aspect with ID '" + t_id + "' due to invalid type.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
        return nullptr;
    }

    template<>
    std::shared_ptr<IERenderable> createAspect<IERenderable>(const std::string &t_id, IE::Core::File *t_resource) {
        return createRenderable(t_id, t_resource);
    }

    std::shared_ptr<IERenderable> createRenderable(const std::string &t_id, IE::Core::File *t_resource);
    std::shared_ptr<IERenderable> getRenderable(const std::string &t_id);
    void                          queueToggleFullscreen();
};