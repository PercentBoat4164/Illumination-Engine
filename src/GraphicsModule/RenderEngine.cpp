#include "RenderEngine.hpp"

#include "Image/Image.hpp"

#include <iostream>
#include <utility>

VkBool32 IE::Graphics::RenderEngine::APIDebugMessenger(
  VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
  VkDebugUtilsMessageTypeFlagsEXT             messageType,
  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
  void                                       *pUserData
) {
    ((IE::Core::Logger *) pUserData)
      ->log(
        messageSeverity,
        std::string(pCallbackData->pMessageIdName) + " ID: " + std::to_string(pCallbackData->messageIdNumber) +
          " | " + vkb::to_string_message_type(messageType) + " " + pCallbackData->pMessage
      );
    return VK_TRUE;
}

vkb::Instance IE::Graphics::RenderEngine::createVulkanInstance() {
    vkb::InstanceBuilder builder{};

    // Set engine properties
    builder.set_engine_name(m_applicationName.c_str())
      .set_engine_version(m_applicationVersion.major, m_applicationVersion.minor, m_applicationVersion.patch);

    // Set application properties
    builder.set_app_name(m_applicationName.c_str()).set_app_version(m_applicationVersion.number);

    // Set vulkan version requirements
    builder.set_minimum_instance_version(m_desiredVulkanVersion.number)
      .require_api_version(m_minimumVulkanVersion.number);

// If debugging and components are available, add validation layers and a debug messenger
#ifndef NDEBUG
    builder
      //      .enable_layer("VK_LAYER_LUNARG_standard_validation")
      //      .enable_layer("VK_LAYER_LUNARG_parameter_validation")
      //      .enable_layer("VK_LAYER_LUNARG_core_validation")
      .enable_validation_layers()
      //    if (vkb::) {
      //       Is mutually exclusive with VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT
      //        builder.add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT)
      //          .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT)
      //          .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT)
      //          //       Is mutually exclusive with VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT
      //          //      .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT)
      //          .add_validation_feature_enable(VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT)
      //    }

      .set_debug_callback(APIDebugMessenger)
      .set_debug_callback_user_data_pointer(&m_graphicsAPICallbackLog);
    m_graphicsAPICallbackLog.setLogLevel(IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_1);
#endif

    // Build the instance and check for errors.
    auto instanceBuilder = builder.build();
    if (!instanceBuilder) {
        m_graphicsAPICallbackLog.log(
          "Failed to create Vulkan instance. Error: " + instanceBuilder.error().message(),
          IE::Core::Logger::Level::ILLUMINATION_ENGINE_LOG_LEVEL_CRITICAL
        );
    }
    m_instance = instanceBuilder.value();
    return m_instance;
}

std::shared_ptr<IE::Graphics::RenderEngine>
IE::Graphics::RenderEngine::create(const std::shared_ptr<IE::Core::Core> &t_core) {

    return std::make_shared<IE::Graphics::RenderEngine>(t_core);
}

GLFWwindow *IE::Graphics::RenderEngine::createWindow() {
    m_api.name = IE_RENDER_ENGINE_API_NAME_VULKAN;
    // Initialize GLFW
    if (glfwInit() != GLFW_TRUE) {
        const char *description;
        int         code = glfwGetError(&description);
        m_graphicsAPICallbackLog.log(
          "Failed to initialize GLFW. Error: " + std::to_string(code) + " " + description,
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_CRITICAL
        );
    } else m_graphicsAPICallbackLog.log("Initialized GLFW");

    // Set GLFW window hints
    /// IF USING VULKAN
    if (m_api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    /// IF USING OPENGL
    if (m_api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
        glfwWindowHint(GLFW_SAMPLES, 1);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#ifndef NDEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
        // To make macOS happy. Put into macOS only code block.
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    // Create window
    m_window = glfwCreateWindow(
      (int) defaultResolution[0],
      (int) defaultResolution[1],
      m_applicationName.c_str(),
      nullptr,
      nullptr
    );
    if (m_window == nullptr) {
        const char *description;
        int         code = glfwGetError(&description);
        m_graphicsAPICallbackLog.log(
          "Failed to create window! Error: " + std::to_string(code) + " " + description,
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    } else
        m_graphicsAPICallbackLog.log(
          "Created window (" + std::to_string(defaultResolution[0]) + ", " + std::to_string(defaultResolution[1]) +
          ")"
        );

    // Set up window
    glfwMakeContextCurrent(m_window);
    glfwSetWindowSizeLimits(m_window, 1, 1, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetWindowPos(m_window, (int) defaultPosition[0], (int) defaultPosition[1]);
    glfwSetWindowAttrib(m_window, GLFW_AUTO_ICONIFY, 0);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    glfwSetWindowUserPointer(m_window, this);
    glfwSwapInterval(useVsync ? 1 : 0);


    // Set icon
    int                                       iconSizeX, iconSizeY;
    IE::Core::MultiDimensionalVector<stbi_uc> pixels{256u, 256u};
    pixels =
      stbi_load(ILLUMINATION_ENGINE_ICON_PATH, (int *) &iconSizeX, (int *) &iconSizeY, nullptr, STBI_rgb_alpha);
    GLFWimage icon{iconSizeX, iconSizeY, pixels.data()};
    glfwSetWindowIcon(m_window, 1, &icon);
    pixels.clear();

    /// IF USING OPENGL
    if (m_api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
        // Initialize GLEW
        auto result = glewInit();
        if (result != GLEW_OK)
            m_graphicsAPICallbackLog.log(
              "Failed to initialize GLEW. Error: " +
                std::string(reinterpret_cast<const char *>(glewGetErrorString(result))),
              Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_CRITICAL
            );
        else m_graphicsAPICallbackLog.log("Initialized GLEW");
    }

    /// IF USING VULKAN
    if (m_api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        // Create vulkan surface
        VkResult result = glfwCreateWindowSurface(m_instance.instance, m_window, nullptr, &m_surface);
        if (result != VK_SUCCESS) {
            const char *description;
            int         code = glfwGetError(&description);
            m_graphicsAPICallbackLog.log(
              "Failed to create window surface! Error: " + std::to_string(code) + " " + description,
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
            );
        } else m_graphicsAPICallbackLog.log("Created window surface!");
    }

    return m_window;
}

vkb::Device IE::Graphics::RenderEngine::createDevice() {

    return m_device;
}

vkb::Swapchain IE::Graphics::RenderEngine::createSwapchain() {
    // Create Swapchain
    vkb::SwapchainBuilder swapchainBuilder{m_device};
    return swapchainBuilder.build().value();
}

IE::Graphics::RenderEngine::RenderEngine(std::shared_ptr<IE::Core::Core> t_core) : m_core(std::move(t_core)) {
    m_instance = createVulkanInstance();
    m_window   = createWindow();
}

std::shared_ptr<IE::Core::Core> IE::Graphics::RenderEngine::getCore() {
    return m_core;
}

GLFWwindow *IE::Graphics::RenderEngine::getWindow() {
    return m_window;
}

IE::Graphics::API IE::Graphics::RenderEngine::getAPI() {
    return m_api;
}

VmaAllocator IE::Graphics::RenderEngine::getAllocator() {
    return allocator;
}

std::string IE::Graphics::RenderEngine::translateVkResultCodes(VkResult t_result) {
    switch (t_result) {
        case VK_SUCCESS: return "VK_SUCCESS";
        case VK_NOT_READY: return "VK_NOT_READY";
        case VK_TIMEOUT: return "VK_TIMEOUT";
        case VK_EVENT_SET: return "VK_EVENT_SET";
        case VK_EVENT_RESET: return "VK_EVENT_RESET";
        case VK_INCOMPLETE: return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
        case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case VK_PIPELINE_COMPILE_REQUIRED: return "VK_PIPELINE_COMPILE_REQUIRED";
        case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case VK_ERROR_NOT_PERMITTED_KHR: return "VK_ERROR_NOT_PERMITTED_KHR";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
        case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
        case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
        case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
        case VK_RESULT_MAX_ENUM: return "VK_RESULT_MAX_ENUM";
        default: return "VK_ERROR_UNKNOWN";
    }
}

IE::Graphics::RenderEngine::~RenderEngine() {
    vkb::destroy_instance(m_instance);
}

void IE::Graphics::RenderEngine::framebufferResizeCallback(GLFWwindow *pWindow, int x, int y) {
    auto renderEngine = static_cast<IE::Graphics::RenderEngine *>(glfwGetWindowUserPointer(pWindow));
    renderEngine->m_graphicsAPICallbackLog.log(
      "Changing window size to (" + std::to_string(x) + ", " + std::to_string(y) + ")"
    );
    renderEngine->currentResolution = {static_cast<size_t>(x), static_cast<size_t>(y)};
}
