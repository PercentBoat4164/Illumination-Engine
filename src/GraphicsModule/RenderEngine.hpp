#pragma once

#include "API/API.hpp"
#include "API/Version.hpp"
#include "CommandBuffer/CommandPool.hpp"
#include "Core/Core.hpp"
#include "Core/EngineModule/Engine.hpp"
#include "Core/LogModule/Logger.hpp"
#include "RenderPass/RenderPassSeries.hpp"
#include "Settings.hpp"
#include "SyncObjects/Fence.hpp"
#include "SyncObjects/Semaphore.hpp"


#define GLEW_IMPLEMENTATION
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Renderable/Renderable.hpp"
#include "VkBootstrap.h"

#include <memory>
#include <utility>
#include <vk_mem_alloc.h>

/* Define macros used throughout the file. */
#define ILLUMINATION_ENGINE_VERSION_MAJOR 0
#define ILLUMINATION_ENGINE_VERSION_MINOR 0
#define ILLUMINATION_ENGINE_VERSION_PATCH 0
#define ILLUMINATION_ENGINE_NAME          "Illumination Engine"

#define ILLUMINATION_ENGINE_ICON_PATH            "res/logos/IlluminationEngineLogo.png"
#define ILLUMINATION_ENGINE_GRAPHICS_LOGGER_NAME "Render Engine"

namespace IE::Graphics {
class RenderEngine : public IE::Core::Engine {
public:
    using AspectType = IE::Graphics::Renderable;

private:
    std::string m_applicationName{ILLUMINATION_ENGINE_NAME};
    Version     m_applicationVersion{
      ILLUMINATION_ENGINE_VERSION_MAJOR,
      ILLUMINATION_ENGINE_VERSION_MINOR,
      ILLUMINATION_ENGINE_VERSION_PATCH};
    IE::Graphics::Version    m_minimumVulkanVersion{1, 0, 0};
    IE::Graphics::Version    m_desiredVulkanVersion{1, 3, 0};
    IE::Graphics::API        m_api;
    IE::Graphics::Settings   m_settings;
    vkb::Instance            m_instance;
    vkb::Swapchain           m_swapchain;
    VkSurfaceKHR             m_surface{};
    VmaAllocator             m_allocator{};
    IE::Core::Logger         m_graphicsAPICallbackLog{ILLUMINATION_ENGINE_GRAPHICS_LOGGER_NAME};
    GLFWwindow              *m_window{};
    std::array<size_t, 2>    m_defaultResolution{800, 600};
    std::array<size_t, 2>    m_defaultPosition{10, 10};
    uint64_t                 m_frameNumber{};
    bool                     m_useVsync{false};
    std::vector<VkImageView> m_swapchainImageViews;
    std::vector<std::shared_ptr<IE::Graphics::Semaphore>>   m_imageAvailableSemaphores{};
    std::vector<std::shared_ptr<IE::Graphics::Semaphore>>   m_renderFinishedSemaphores{};
    std::vector<std::shared_ptr<IE::Graphics::Fence>>       m_inFlightFences{};
    std::vector<std::shared_ptr<IE::Graphics::Fence>>       m_imagesInFlight{};
    std::vector<std::shared_ptr<IE::Graphics::CommandPool>> m_commandPools{};
    IE::Graphics::RenderPassSeries                          m_renderPassSeries{this};
    IE::Graphics::DescriptorSet m_engineDescriptor{IE::Graphics::DescriptorSet::IE_DESCRIPTOR_SET_TYPE_PER_FRAME};
    std::shared_ptr<IE::Graphics::CommandPool>                m_primaryCommandPool{};
    std::vector<std::shared_ptr<IE::Graphics::CommandBuffer>> m_primaryCommandBuffers{};

    static std::remove_pointer_t<PFN_vkDebugUtilsMessengerCallbackEXT> APIDebugMessenger;

    IE::Core::Threading::Task<GLFWwindow *> createWindow();

    vkb::Instance createInstance();

    VkSurfaceKHR createSurface();

    vkb::Device createDevice();

    VmaAllocator createAllocator();

    vkb::Swapchain createSwapchain();

    void createSyncObjects();

    void createCommandPools();

    static void framebufferResizeCallback(GLFWwindow *pWindow, int x, int y);

public:
    std::array<size_t, 2> m_currentResolution{m_defaultResolution};
    vkb::Device           m_device;

    explicit RenderEngine(const std::string &t_ID);

    Core::Threading::Task<bool> update();

    GLFWwindow *getWindow();

    VmaAllocator getAllocator();

    IE::Core::Logger getLogger();

    IE::Graphics::API getAPI();

    Core::Threading::Task<void> create();

    static std::string translateVkResultCodes(VkResult t_result);

    static std::string makeErrorMessage(
      const std::string &t_error,
      const std::string &t_function,
      const std::string &t_file,
      int                t_line,
      const std::string &t_moreInfo = ""
    );

    template<typename T>
    static std::function<std::string(T &)> makeErrorMessageReporter(
      std::initializer_list<T>           t_errors,
      std::initializer_list<std::string> t_errorNames,
      const std::string                 &t_function,
      const std::string                 &t_file,
      int                                t_line,
      const std::string                 &t_moreInfo = ""
    ) {
        return [&](T &t_error) {
            for (size_t i = 0; i < t_errors.size(); ++i)
                if (t_error == *(t_errors.begin() + i))
                    return IE::Graphics::RenderEngine::makeErrorMessage(
                      *(t_errorNames.begin() + i),
                      t_function,
                      t_file,
                      t_line,
                      t_moreInfo
                    );
            return IE::Graphics::RenderEngine::makeErrorMessage(
              "UNSPECIFIED_ERROR",
              t_function,
              t_file,
              t_line,
              t_moreInfo
            );
        };
    }

    ~RenderEngine() override;

    std::shared_ptr<AspectType> createAspect(const std::string &t_id, IE::Core::File *t_resource);

    std::shared_ptr<AspectType> getAspect(const std::string &t_id);

    void         createRenderPasses();
    VkFormat     getColorFormat();
    Settings    &getSettings();
    void         createDescriptorSets();
    CommandPool *getCommandPool();
    void         createPrimaryCommandObjects();
};
}  // namespace IE::Graphics