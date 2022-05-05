#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IEBuffer;

/* Include classes used as attributes or _function arguments. */
// Internal dependencies
#include "IEImage.hpp"

// External dependencies
#include <vulkan/vulkan.h>

#include <stb_image.h>

// System dependencies
#include <cstdint>
#include <string>


class IETexture : public IEImage {
public:
    struct CreateInfo {
        VkFormat format{VK_FORMAT_R8G8B8A8_SRGB};
        VkImageLayout layout{VK_IMAGE_LAYOUT_UNDEFINED};
        VkImageType type{VK_IMAGE_TYPE_2D};
        VkImageUsageFlags usage{VK_IMAGE_USAGE_SAMPLED_BIT};
        VkImageCreateFlags flags{};
        VmaMemoryUsage allocationUsage{};
        uint32_t width{}, height{};
        IEBuffer *dataSource{};
        stbi_uc *data{};
        std::string filename{};
    };

    void copyCreateInfo(IETexture::CreateInfo *createInfo);

    void create(IERenderEngine *engineLink, IETexture::CreateInfo *createInfo);

    void upload(void *data);

    void upload(IEBuffer *data);

    void generateMipMaps();
};