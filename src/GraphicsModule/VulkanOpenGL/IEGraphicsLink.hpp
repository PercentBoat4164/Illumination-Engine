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
        IEVersion version = getHighestSupportedVersion(); // The version of the API to use

        /**
         * @brief Finds the highest supported API version.
         * @return An IEVersion populated with all the data about the highest supported API version
         */
        IEVersion getHighestSupportedVersion() const {
            IEVersion temporaryVersion;
            #ifdef ILLUMINATION_ENGINE_VULKAN
            if (name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
                auto vkEnumerateDeviceInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
                if (vkEnumerateDeviceInstanceVersion == nullptr) { temporaryVersion = IEVersion{"1.0.0"}; } else {
                    uint32_t instanceVersion;
                    vkEnumerateDeviceInstanceVersion(&instanceVersion);
                    temporaryVersion = IEVersion{VK_VERSION_MAJOR(instanceVersion), VK_VERSION_MINOR(instanceVersion), VK_VERSION_PATCH(instanceVersion)};
                    temporaryVersion.number = instanceVersion;
                }
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
                // Alternative solution if version is still not found
                else {
                    glfwInit();
                    GLFWwindow *temporaryWindow = glfwCreateWindow(1, 1, "Gathering OpenGL Data...", nullptr, nullptr);
                    glfwMakeContextCurrent(temporaryWindow);
                    std::string versionName = reinterpret_cast<const char *>(glGetString(GL_VERSION));
                    versionName = versionName.substr(0, versionName.find_first_of(' '));
                    if (std::count(versionName.begin(), versionName.end(), '.') < 2) {
                        versionName += ".0";
                    }
                    temporaryVersion = IEVersion{versionName};
                    glfwDestroyWindow(temporaryWindow);
                    glfwTerminate();
                }
            }
            #endif
            return temporaryVersion;
        }
    };

    IESettings settings{};
    IEAPI api{};
    vkb::Device device{};
    vkb::Swapchain swapchain{};
    vkb::Instance instance{};
    vkb::detail::Result<vkb::SystemInfo> systemInfo = vkb::SystemInfo::get_system_info();
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
    PFN_vkGetPhysicalDeviceProperties2KHR vkGetPhysicalDeviceProperties2KHR{};

    void build() {
        vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device.device, "vkGetBufferDeviceAddressKHR"));
        vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device.device, "vkCmdBuildAccelerationStructuresKHR"));
        vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device.device, "vkCreateAccelerationStructureKHR"));
        vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device.device, "vkDestroyAccelerationStructureKHR"));
        vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device.device, "vkGetAccelerationStructureBuildSizesKHR"));
        vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device.device, "vkGetAccelerationStructureDeviceAddressKHR"));
        vkAcquireNextImageKhr = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(device.device, "vkAcquireNextImageKHR"));
        vkGetPhysicalDeviceProperties2KHR = reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2KHR>(vkGetDeviceProcAddr(device.device, "vkGetPhysicalDeviceProperties2KHR"));

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
};