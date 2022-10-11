#pragma once

#include "API/API.hpp"
#include "API/Version.hpp"
#include "Core/Core.hpp"
#include "Core/LogModule/IELogger.hpp"
#include "SyncObjects/Fence.hpp"
#include "SyncObjects/Semaphore.hpp"

#include <vulkan/vulkan.h>

#define GLEW_IMPLEMENTATION
#include "GL/glew.h"

#define GLFW_IMPLEMENTATION
#include "GLFW/glfw3.h"
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
private:
    std::string m_applicationName{ILLUMINATION_ENGINE_NAME};
    Version     m_applicationVersion{
      ILLUMINATION_ENGINE_VERSION_MAJOR,
      ILLUMINATION_ENGINE_VERSION_MINOR,
      ILLUMINATION_ENGINE_VERSION_PATCH};
    IE::Graphics::Version m_minimumVulkanVersion{1, 0, 0};
    IE::Graphics::Version m_desiredVulkanVersion{1, 3, 0};
    IE::Graphics::API     m_api;
    vkb::Instance         m_instance;
    vkb::Swapchain        m_swapchain;
    vkb::Device           m_device;
    VkSurfaceKHR          m_surface{};
    VmaAllocator          m_allocator{};
    IE::Core::Logger      m_graphicsAPICallbackLog{
      ILLUMINATION_ENGINE_GRAPHICS_LOGGER_NAME,
      ILLUMINATION_ENGINE_GRAPHICS_LOGGER_FILENAME};
    GLFWwindow                          *m_window{};
    std::array<size_t, 2>                m_defaultResolution{800, 600};
    std::array<size_t, 2>                m_currentResolution = m_defaultResolution;
    std::array<size_t, 2>                m_defaultPosition{10, 10};
    bool                                 m_useVsync{false};
    std::vector<VkImageView>             m_swapchainImageViews;
    std::vector<IE::Graphics::Semaphore> m_imageAvailableSemaphores{};
    std::vector<IE::Graphics::Semaphore> m_renderFinishedSemaphores{};
    std::vector<IE::Graphics::Fence>     m_inFlightFences{};
    std::vector<IE::Graphics::Fence>     m_imagesInFlight{};

    static VkBool32 APIDebugMessenger(
      VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT             messageType,
      const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
      void                                       *pUserData
    );

    explicit RenderEngine() = default;

    GLFWwindow *createWindow();

    vkb::Instance createInstance();

    VkSurfaceKHR createSurface();

    vkb::Device createDevice();

    VmaAllocator createAllocator();

    vkb::Swapchain createSwapchain();

    static void framebufferResizeCallback(GLFWwindow *pWindow, int x, int y);

public:
    GLFWwindow *getWindow();

    VmaAllocator getAllocator();

    IE::Core::Logger getLogger();

    VkDevice getDevice() const {
        return m_device.device;
    }

    VkPhysicalDevice getPhysicalDevice() const {
        return m_device.physical_device;
    }

    IE::Graphics::API getAPI();

    static std::shared_ptr<RenderEngine> create();

    static std::string translateVkResultCodes(VkResult t_result);

    ~RenderEngine();

    void destroy();

    static void destroy(IE::Graphics::RenderEngine *t_engineLink);
};
}  // namespace IE::Graphics