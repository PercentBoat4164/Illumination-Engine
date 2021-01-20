#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <VkBootstrap.h>
#include <vector>


class VulkanEngine {
public:
    bool _isInitialized{false};
    int _frameNumber{0};
    VkExtent2D _windowExtent{1700, 900};
    GLFWwindow *_window{nullptr};
    VkInstance _instance{};
    VkDebugUtilsMessengerEXT _debug_messenger{};
    VkPhysicalDevice _chosenGPU{};
    VkDevice _device{};
    VkSurfaceKHR _surface{};
    VkSwapchainKHR _swapchain{};
    VkFormat _swapchainImageFormat{};
    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;
    bool framebufferResized{false};

    VulkanEngine() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _window = glfwCreateWindow(_windowExtent.width, _windowExtent.height, "Vulkan Tutorial", nullptr, nullptr);
        glfwSetWindowUserPointer(_window, this);
        glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);
        vkb::InstanceBuilder builder;
        auto inst_ret = builder.set_app_name("Example Vulkan Application").request_validation_layers(true).require_api_version(1, 2, 0).use_default_debug_messenger().build();
        vkb::Instance vkb_inst = inst_ret.value();
        _instance = vkb_inst.instance;
        _debug_messenger = vkb_inst.debug_messenger;
        if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS) {throw std::runtime_error("failed to create window surface: 38");}
        vkb::PhysicalDeviceSelector selector{vkb_inst};
        vkb::PhysicalDevice physicalDevice = selector.set_surface(_surface).select().value();
        vkb::DeviceBuilder deviceBuilder{physicalDevice};
        vkb::Device vkbDevice = deviceBuilder.build().value();
        _device = vkbDevice.device;
        _chosenGPU = physicalDevice.physical_device;
        vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};
        vkb::Swapchain vkbSwapchain = swapchainBuilder.use_default_format_selection().set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).set_desired_extent(_windowExtent.width, _windowExtent.height).build().value();
        _swapchain = vkbSwapchain.swapchain;
        _swapchainImages = vkbSwapchain.get_images().value();
        _swapchainImageViews = vkbSwapchain.get_image_views().value();
        _swapchainImageFormat = vkbSwapchain.image_format;
        _isInitialized = true;
    }

    void cleanUp() {
        if (_isInitialized) {
            vkDestroySwapchainKHR(_device, _swapchain, nullptr);
            for (VkImageView & _swapchainImageView : _swapchainImageViews) {vkDestroyImageView(_device, _swapchainImageView, nullptr);}
            vkDestroyDevice(_device, nullptr);
            vkDestroySurfaceKHR(_instance, _surface, nullptr);
            vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
            vkDestroyInstance(_instance, nullptr);
            glfwDestroyWindow(_window);
        }
    }

    void update() {

    }

private:
    static void FramebufferResizeCallback(GLFWwindow *window, int width, int height) {
        reinterpret_cast<VulkanEngine *>(glfwGetWindowUserPointer(window))->framebufferResized = true;
    }
};

int main() {
    VulkanEngine renderEngine;
    renderEngine.cleanUp();
}