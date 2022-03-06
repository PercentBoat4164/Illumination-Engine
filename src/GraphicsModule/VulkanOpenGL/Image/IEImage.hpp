#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

class IEBuffer;

/* Include classes used as attributes or function arguments. */
// External dependencies
#include <vk_mem_alloc.h>

#include <vulkan/vulkan.h>

#include <stb_image.h>

// System dependencies
#include <cstdint>
#include <string>
#include <vector>
#include <functional>


class IEImage {
protected:
    [[maybe_unused]] [[nodiscard]] uint8_t getHighestMSAASampleCount(uint8_t requested) const ;

    [[maybe_unused]] [[nodiscard]] float getHighestAnisotropyLevel(float requested) const ;

public:
    struct CreateInfo {
        VkFormat format{VK_FORMAT_R8G8B8A8_SRGB};
        VkImageLayout layout{VK_IMAGE_LAYOUT_UNDEFINED};
        VkImageType type{VK_IMAGE_TYPE_2D};
        VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};
        VkImageCreateFlags flags{};
        VkImageAspectFlags aspect{VK_IMAGE_ASPECT_COLOR_BIT};
        VmaMemoryUsage allocationUsage{};
        uint32_t width{}, height{};
        IEBuffer *dataSource{};
        stbi_uc *data{};
    };

    VkImage image{};
    VkImageView view{};
    VkSampler sampler{};
    VkFormat imageFormat{VK_FORMAT_R8G8B8A8_SRGB};  // Image format as interpreted by the Vulkan API
    VkImageLayout imageLayout{VK_IMAGE_LAYOUT_UNDEFINED};  // How the GPU sees the image
    VkImageType imageType{VK_IMAGE_TYPE_2D};  // Image shape (1D, 2D, 3D)
    VkImageTiling imageMemoryArrangement{VK_IMAGE_TILING_OPTIMAL};  // How the image is stored in GPU memory
    VkImageUsageFlags imageUsage{VK_IMAGE_USAGE_SAMPLED_BIT};  // How the program is going to use the image
    VkImageCreateFlags imageFlags{};  // How should / was the image be created
    VkImageAspectFlags imageAspect{VK_IMAGE_ASPECT_COLOR_BIT};
    VmaMemoryUsage allocationUsage{};  // How is the allocation going to be used between the CPU and GPU
    uint32_t width{}, height{};
    IEBuffer *dataSource{};
    stbi_uc *data{};
    std::string filename{};

    IERenderEngine *linkedRenderEngine{};
    std::vector<std::function<void()>> deletionQueue{};
    VmaAllocation allocation{};

    IEImage();

    void destroy();

    virtual void create(IEImage::CreateInfo *createInfo);

    virtual void copyCreateInfo(IEImage::CreateInfo *createInfo);

    virtual void create(IERenderEngine *engineLink, IEImage::CreateInfo *createInfo);

    [[maybe_unused]] void toBuffer(const IEBuffer &buffer) const;

    void transitionLayout(VkImageLayout newLayout);

    ~IEImage();
};
