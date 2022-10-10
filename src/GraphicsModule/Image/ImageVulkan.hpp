#pragma once

class IERenderEngine;

#include "Image.hpp"

#include <functional>
#include <vulkan/vulkan.h>

namespace IE::Graphics::detail {
class ImageVulkan : virtual public IE::Graphics::Image {
public:
    VkFormat              m_format;    // Pixel format of the image. Includes channel count, location, and depth.
    VkImageLayout         m_layout;    // How the image is to be laid out in memory.
    VkImageType           m_type;      // Defines the image's dimensionality.
    VkImageViewType       m_viewType;  // Defines the usage of the image's dimensionality. (e.g. 3D, Cube, Array)
    VkImageTiling         m_tiling;    // Boilerplate. Leave at VK_IMAGE_TILING_OPTIMAL.
    VkImageUsageFlags     m_usage;     // How the program will be using the image.
    VkImageCreateFlags    m_flags;     // A record of the flags used to create the image.
    VkImageAspectFlags    m_aspect;    // What aspects should the image have? (e.g. Depth, Color)
    VkSampleCountFlagBits m_samples;   // The number of MSAA samples to use.
    uint8_t               m_mipLevel;  // The number of mip maps to generate / that exist for this image.
    VmaMemoryUsage        m_allocationUsage;  // A record of what this allocation is optimized for.
    VmaAllocation         m_allocation;       // The allocation used to create this image.
    VmaAllocationInfo     m_allocationInfo;   // Information about the allocation filled during image creation.
    VkImage               m_id;               // The VkImage itself.
    VkImageView           m_view;             // The perspective of the view. (e.g. A 2D slice of a 3D texture)

    ImageVulkan();

    template<typename... Args>
    explicit ImageVulkan(const std::weak_ptr<IERenderEngine> &t_engineLink, Args... t_dimensions);

    ImageVulkan &operator=(ImageVulkan &&t_other) noexcept;

    ImageVulkan &operator=(const ImageVulkan &t_other);

    ~ImageVulkan() override;

    [[nodiscard]] uint8_t getBytesInFormat() const override;

    void transitionLayout(VkImageLayout);

    void setLocation(Location) override;

    void setData(const IE::Core::MultiDimensionalVector<unsigned char> &) override;

protected:
    bool _createImage(const IE::Core::MultiDimensionalVector<unsigned char> &t_data) override;

    void _setImageData(const Core::MultiDimensionalVector<unsigned char> &t_data) override;

    void _getImageData(Core::MultiDimensionalVector<unsigned char> *pVector) const override;

    void _destroyImage() override;

private:
    // Used to call the _destroyImage function of this class in the destructor without worrying about virtuality.
    static void (IE::Graphics::detail::ImageVulkan::*const destroyImage)();
};
}  // namespace IE::Graphics::detail