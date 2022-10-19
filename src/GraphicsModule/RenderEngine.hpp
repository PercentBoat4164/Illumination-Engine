#pragma once

#include "API/API.hpp"
#include "API/Version.hpp"
#include "CommandBuffer/CommandPool.hpp"
#include "Core/Core.hpp"
#include "Core/LogModule/IELogger.hpp"
#include "Settings.hpp"
#include "SyncObjects/Fence.hpp"
#include "SyncObjects/Semaphore.hpp"
#include "RenderPass/RenderPass.hpp"


#include <vulkan/vulkan.h>

#define GLEW_IMPLEMENTATION
#include "GL/glew.h"

#define GLFW_IMPLEMENTATION
#include "GLFW/glfw3.h"
#include "RenderPass/RenderPassSeries.hpp"
#include "VkBootstrap.h"

#include <memory>
#include <utility>
#include <vk_mem_alloc.h>

/* Define macros used throughout the file. */
#define ILLUMINATION_ENGINE_VERSION_MAJOR 0
#define ILLUMINATION_ENGINE_VERSION_MINOR 0
#define ILLUMINATION_ENGINE_VERSION_PATCH 0
#define ILLUMINATION_ENGINE_NAME          "Illumination Engine"

#define ILLUMINATION_ENGINE_ICON_PATH                "res/logos/IlluminationEngineLogo.png"
#define ILLUMINATION_ENGINE_GRAPHICS_LOGGER_NAME     "Graphics API"
#define ILLUMINATION_ENGINE_GRAPHICS_LOGGER_FILENAME "logs/GraphicsAPI.log"

namespace IE::Graphics {
class RenderEngine : public IE::Core::Engine {
public:
    using AspectType = IEAspect;

private:
    std::string m_applicationName{ILLUMINATION_ENGINE_NAME};
    Version     m_applicationVersion{
      ILLUMINATION_ENGINE_VERSION_MAJOR,
      ILLUMINATION_ENGINE_VERSION_MINOR,
      ILLUMINATION_ENGINE_VERSION_PATCH};
    IE::Graphics::Version  m_minimumVulkanVersion{1, 0, 0};
    IE::Graphics::Version  m_desiredVulkanVersion{1, 3, 0};
    IE::Graphics::API      m_api;
    IE::Graphics::Settings m_settings;
    vkb::Instance          m_instance;
    vkb::Swapchain         m_swapchain;
    VkSurfaceKHR           m_surface{};
    VmaAllocator           m_allocator{};
    IE::Core::Logger       m_graphicsAPICallbackLog{
      ILLUMINATION_ENGINE_GRAPHICS_LOGGER_NAME,
      ILLUMINATION_ENGINE_GRAPHICS_LOGGER_FILENAME};
    GLFWwindow                                             *m_window{};
    std::array<size_t, 2>                                   m_defaultResolution{800, 600};
    std::array<size_t, 2>                                   m_currentResolution = m_defaultResolution;
    std::array<size_t, 2>                                   m_defaultPosition{10, 10};
    bool                                                    m_useVsync{false};
    std::vector<VkImageView>                                m_swapchainImageViews;
    std::vector<std::shared_ptr<IE::Graphics::Semaphore>>   m_imageAvailableSemaphores{};
    std::vector<std::shared_ptr<IE::Graphics::Semaphore>>   m_renderFinishedSemaphores{};
    std::vector<std::shared_ptr<IE::Graphics::Fence>>       m_inFlightFences{};
    std::vector<std::shared_ptr<IE::Graphics::Fence>>       m_imagesInFlight{};
    std::vector<std::shared_ptr<IE::Graphics::CommandPool>> m_commandPools{};
    IE::Graphics::RenderPassSeries m_renderPassSeries{this};


    static VkBool32 APIDebugMessenger(
      VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT             messageType,
      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
      void                                       *pUserData
    );

    GLFWwindow *createWindow();

    vkb::Instance createInstance();

    VkSurfaceKHR createSurface();

    vkb::Device createDevice();

    VmaAllocator createAllocator();

    vkb::Swapchain createSwapchain();

    void createSyncObjects();

    void createCommandPools();

    static void framebufferResizeCallback(GLFWwindow *pWindow, int x, int y);

public:
    vkb::Device m_device;

    explicit RenderEngine() = default;

    GLFWwindow *getWindow();

    VmaAllocator getAllocator();

    IE::Core::Logger getLogger();

    IE::Graphics::CommandPool *tryGetCommandPool();

    IE::Graphics::CommandPool *getCommandPool();
    IEAspect                  *createAspect(std::weak_ptr<IEAsset> t_asset, const std::string &t_id) override;

    IE::Graphics::API getAPI();

    IE::Core::Engine *create() override;

    static std::string translateVkResultCodes(VkResult t_result);

    ~RenderEngine() override;

    AspectType *getAspect(const std::string &t_id) override;

    void createRenderPasses();
};
}  // namespace IE::Graphics