#include "ImageVulkan.hpp"

#include "RenderEngine.hpp"

#include <mutex>
#include <utility>

IE::Graphics::detail::ImageVulkan::ImageVulkan() :
        m_format{VK_FORMAT_UNDEFINED},
        m_layout{VK_IMAGE_LAYOUT_UNDEFINED},
        m_type{VK_IMAGE_TYPE_1D},
        m_viewType{VK_IMAGE_VIEW_TYPE_1D},
        m_tiling{VK_IMAGE_TILING_OPTIMAL},
        m_usage{VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
        m_flags{0},
        m_aspect{VK_IMAGE_ASPECT_NONE},
        m_samples{VK_SAMPLE_COUNT_1_BIT},
        m_mipLevel{1},
        m_allocationUsage{VMA_MEMORY_USAGE_AUTO},
        m_allocation(),
        m_allocationInfo(),
        m_id(nullptr),
        m_view(nullptr) {
}

IE::Graphics::detail::ImageVulkan &
IE::Graphics::detail::ImageVulkan::operator=(const IE::Graphics::detail::ImageVulkan &t_other) {
    if (&t_other != this) {
        m_format          = t_other.m_format;
        m_layout          = t_other.m_layout;
        m_type            = t_other.m_type;
        m_viewType        = t_other.m_viewType;
        m_tiling          = t_other.m_tiling;
        m_usage           = t_other.m_usage;
        m_flags           = t_other.m_flags;
        m_aspect          = t_other.m_aspect;
        m_samples         = t_other.m_samples;
        m_mipLevel        = t_other.m_mipLevel;
        m_allocationUsage = t_other.m_allocationUsage;
        m_allocation      = t_other.m_allocation;
        m_allocationInfo  = t_other.m_allocationInfo;
        m_id              = t_other.m_id;
        m_view            = t_other.m_view;
    }
    return *this;
}

IE::Graphics::detail::ImageVulkan &
IE::Graphics::detail::ImageVulkan::operator=(IE::Graphics::detail::ImageVulkan &&t_other) noexcept {
    if (&t_other != this) {
        m_format          = std::exchange(t_other.m_format, VK_FORMAT_UNDEFINED);
        m_layout          = std::exchange(t_other.m_layout, VK_IMAGE_LAYOUT_UNDEFINED);
        m_type            = std::exchange(t_other.m_type, VK_IMAGE_TYPE_1D);
        m_viewType        = std::exchange(t_other.m_viewType, VK_IMAGE_VIEW_TYPE_1D);
        m_tiling          = std::exchange(t_other.m_tiling, VK_IMAGE_TILING_OPTIMAL);
        m_usage           = std::exchange(t_other.m_usage, VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        m_flags           = std::exchange(t_other.m_flags, 0);
        m_samples         = std::exchange(t_other.m_samples, VK_SAMPLE_COUNT_1_BIT);
        m_mipLevel        = std::exchange(t_other.m_mipLevel, 1);
        m_aspect          = std::exchange(t_other.m_aspect, VK_IMAGE_ASPECT_NONE);
        m_allocationUsage = std::exchange(t_other.m_allocationUsage, VMA_MEMORY_USAGE_AUTO);
        m_allocation      = std::exchange(t_other.m_allocation, {});
        m_allocationInfo  = std::exchange(t_other.m_allocationInfo, {});
        m_id              = std::exchange(t_other.m_id, nullptr);
        m_view            = std::exchange(t_other.m_view, nullptr);
    }
    return *this;
}

template<typename... Args>
IE::Graphics::detail::ImageVulkan::ImageVulkan(
  const std::weak_ptr<IERenderEngine> &t_engineLink,
  Args... t_dimensions
) :
        Image(t_engineLink, t_dimensions...),
        m_format{VK_FORMAT_UNDEFINED},
        m_layout{VK_IMAGE_LAYOUT_UNDEFINED},
        m_type{VK_IMAGE_TYPE_1D},
        m_viewType{VK_IMAGE_VIEW_TYPE_1D},
        m_tiling{VK_IMAGE_TILING_OPTIMAL},
        m_usage{VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
        m_flags{0},
        m_aspect{VK_IMAGE_ASPECT_NONE},
        m_samples{VK_SAMPLE_COUNT_1_BIT},
        m_mipLevel{1},
        m_allocationUsage{VMA_MEMORY_USAGE_AUTO},
        m_allocation(),
        m_allocationInfo(),
        m_id(nullptr),
        m_view(nullptr) {
}

uint8_t IE::Graphics::detail::ImageVulkan::getBytesInFormat() const {
    switch (m_format) {
        case VK_FORMAT_UNDEFINED:
            IE::Core::Core::getInst().logger.log(
              "Attempt to get number of bytes in pixel of format VK_FORMAT_UNDEFINED!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
            );
            return 0x0;
        case VK_FORMAT_MAX_ENUM:
            IE::Core::Core::getInst().logger.log(
              "Attempt to get number of bytes in pixel of format VK_FORMAT_MAX_ENUM!",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
            );
            return 0x0;
        default:
            IE::Core::Core::getInst().logger.log(
              "Attempt to get number of bytes in pixel of unknown format",
              IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
            );
            return 0x0;
        case VK_FORMAT_R8_UNORM:
        case VK_FORMAT_R8_SNORM:
        case VK_FORMAT_R8_USCALED:
        case VK_FORMAT_R8_SSCALED:
        case VK_FORMAT_R8_UINT:
        case VK_FORMAT_R8_SINT:
        case VK_FORMAT_R8_SRGB:
        case VK_FORMAT_R4G4_UNORM_PACK8: return 0x1;
        case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
        case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
        case VK_FORMAT_B5G6R5_UNORM_PACK16:
        case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
        case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
        case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
        case VK_FORMAT_R8G8_UNORM:
        case VK_FORMAT_R8G8_SNORM:
        case VK_FORMAT_R8G8_USCALED:
        case VK_FORMAT_R8G8_SSCALED:
        case VK_FORMAT_R8G8_UINT:
        case VK_FORMAT_R8G8_SINT:
        case VK_FORMAT_R8G8_SRGB:
        case VK_FORMAT_R16_UNORM:
        case VK_FORMAT_R16_SNORM:
        case VK_FORMAT_R16_USCALED:
        case VK_FORMAT_R16_SSCALED:
        case VK_FORMAT_R16_UINT:
        case VK_FORMAT_R16_SINT:
        case VK_FORMAT_R16_SFLOAT:
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_R10X6_UNORM_PACK16:
        case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
        case VK_FORMAT_A4B4G4R4_UNORM_PACK16: return 0x2;
        case VK_FORMAT_R8G8B8_UNORM:
        case VK_FORMAT_R8G8B8_SNORM:
        case VK_FORMAT_R8G8B8_USCALED:
        case VK_FORMAT_R8G8B8_SSCALED:
        case VK_FORMAT_R8G8B8_UINT:
        case VK_FORMAT_R8G8B8_SINT:
        case VK_FORMAT_R8G8B8_SRGB:
        case VK_FORMAT_B8G8R8_UNORM:
        case VK_FORMAT_B8G8R8_SNORM:
        case VK_FORMAT_B8G8R8_USCALED:
        case VK_FORMAT_B8G8R8_SSCALED:
        case VK_FORMAT_B8G8R8_UINT:
        case VK_FORMAT_B8G8R8_SINT:
        case VK_FORMAT_B8G8R8_SRGB:
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
        case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
        case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM: return 0x3;
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SNORM:
        case VK_FORMAT_R8G8B8A8_USCALED:
        case VK_FORMAT_R8G8B8A8_SSCALED:
        case VK_FORMAT_R8G8B8A8_UINT:
        case VK_FORMAT_R8G8B8A8_SINT:
        case VK_FORMAT_R8G8B8A8_SRGB:
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SNORM:
        case VK_FORMAT_B8G8R8A8_USCALED:
        case VK_FORMAT_B8G8R8A8_SSCALED:
        case VK_FORMAT_B8G8R8A8_UINT:
        case VK_FORMAT_B8G8R8A8_SINT:
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
        case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
        case VK_FORMAT_A8B8G8R8_UINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SINT_PACK32:
        case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
        case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
        case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
        case VK_FORMAT_A2R10G10B10_UINT_PACK32:
        case VK_FORMAT_A2R10G10B10_SINT_PACK32:
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
        case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
        case VK_FORMAT_A2B10G10R10_UINT_PACK32:
        case VK_FORMAT_A2B10G10R10_SINT_PACK32:
        case VK_FORMAT_R16G16_UNORM:
        case VK_FORMAT_R16G16_SNORM:
        case VK_FORMAT_R16G16_USCALED:
        case VK_FORMAT_R16G16_SSCALED:
        case VK_FORMAT_R16G16_UINT:
        case VK_FORMAT_R16G16_SINT:
        case VK_FORMAT_R16G16_SFLOAT:
        case VK_FORMAT_R32_UINT:
        case VK_FORMAT_R32_SINT:
        case VK_FORMAT_R32_SFLOAT:
        case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
        case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
        case VK_FORMAT_G8B8G8R8_422_UNORM:
        case VK_FORMAT_B8G8R8G8_422_UNORM:
        case VK_FORMAT_R10X6G10X6_UNORM_2PACK16: return 0x4;
        case VK_FORMAT_D32_SFLOAT_S8_UINT: return 0x5;
        case VK_FORMAT_R16G16B16_UNORM:
        case VK_FORMAT_R16G16B16_SNORM:
        case VK_FORMAT_R16G16B16_USCALED:
        case VK_FORMAT_R16G16B16_SSCALED:
        case VK_FORMAT_R16G16B16_UINT:
        case VK_FORMAT_R16G16B16_SINT:
        case VK_FORMAT_R16G16B16_SFLOAT:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
        case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
        case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
        case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
        case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM: return 0x6;
        case VK_FORMAT_R16G16B16A16_UNORM:
        case VK_FORMAT_R16G16B16A16_SNORM:
        case VK_FORMAT_R16G16B16A16_USCALED:
        case VK_FORMAT_R16G16B16A16_SSCALED:
        case VK_FORMAT_R16G16B16A16_UINT:
        case VK_FORMAT_R16G16B16A16_SINT:
        case VK_FORMAT_R16G16B16A16_SFLOAT:
        case VK_FORMAT_R32G32_UINT:
        case VK_FORMAT_R32G32_SINT:
        case VK_FORMAT_R32G32_SFLOAT:
        case VK_FORMAT_R64_UINT:
        case VK_FORMAT_R64_SINT:
        case VK_FORMAT_R64_SFLOAT:
        case VK_FORMAT_S8_UINT:
        case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
        case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
        case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
        case VK_FORMAT_BC4_UNORM_BLOCK:
        case VK_FORMAT_BC4_SNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
        case VK_FORMAT_EAC_R11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11_SNORM_BLOCK:
        case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
        case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
        case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
        case VK_FORMAT_R12X4_UNORM_PACK16:
        case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
        case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
        case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
        case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
        case VK_FORMAT_G16B16G16R16_422_UNORM:
        case VK_FORMAT_B16G16R16G16_422_UNORM:
        case VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG:
        case VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG: return 0x8;
        case VK_FORMAT_R32G32B32_UINT:
        case VK_FORMAT_R32G32B32_SINT:
        case VK_FORMAT_R32G32B32_SFLOAT: return 0x12;
        case VK_FORMAT_R32G32B32A32_UINT:
        case VK_FORMAT_R32G32B32A32_SINT:
        case VK_FORMAT_R32G32B32A32_SFLOAT:
        case VK_FORMAT_R64G64_UINT:
        case VK_FORMAT_R64G64_SINT:
        case VK_FORMAT_R64G64_SFLOAT:
        case VK_FORMAT_BC2_UNORM_BLOCK:
        case VK_FORMAT_BC2_SRGB_BLOCK:
        case VK_FORMAT_BC3_UNORM_BLOCK:
        case VK_FORMAT_BC3_SRGB_BLOCK:
        case VK_FORMAT_BC5_UNORM_BLOCK:
        case VK_FORMAT_BC5_SNORM_BLOCK:
        case VK_FORMAT_BC6H_UFLOAT_BLOCK:
        case VK_FORMAT_BC6H_SFLOAT_BLOCK:
        case VK_FORMAT_BC7_UNORM_BLOCK:
        case VK_FORMAT_BC7_SRGB_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
        case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
        case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
        case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
        case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
        case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
        case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
        case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
        case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
        case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK: return 0x16;
        case VK_FORMAT_R64G64B64_UINT:
        case VK_FORMAT_R64G64B64_SINT:
        case VK_FORMAT_R64G64B64_SFLOAT: return 0x24;
        case VK_FORMAT_R64G64B64A64_UINT:
        case VK_FORMAT_R64G64B64A64_SINT:
        case VK_FORMAT_R64G64B64A64_SFLOAT: return 0x32;
    }
}

void IE::Graphics::detail::ImageVulkan::transitionLayout(VkImageLayout) {
    /**@todo Implement me! (ImageVulkan::transitionLayout)*/
}

void IE::Graphics::detail::ImageVulkan::setLocation(IE::Graphics::Image::Location t_location) {
    std::unique_lock<std::mutex> lock(*m_mutex);
    if (!m_data.empty() || m_id != nullptr && t_location != m_location) {  // If data already exists somewhere and
                                                                           // the new location is not the same as
                                                                           // the old
        if (m_location & IE_IMAGE_LOCATION_SYSTEM && t_location & IE_IMAGE_LOCATION_VIDEO) {  // system -> video
            // Create image with specified intrinsics
            lock.release();
            _createImage(m_data);
            lock.lock();
        } else if (m_location & IE_IMAGE_LOCATION_VIDEO && t_location & IE_IMAGE_LOCATION_SYSTEM) {  // video ->
                                                                                                     // system
            // Copy data to system
            lock.release();
            _getImageData(&m_data);
            lock.lock();
        }
        if (m_location & IE_IMAGE_LOCATION_SYSTEM && ~t_location & IE_IMAGE_LOCATION_SYSTEM) {  // system -> none
            // Clear image from system
            m_data.clear();
        }
        if (m_location & IE_IMAGE_LOCATION_VIDEO && ~t_location & IE_IMAGE_LOCATION_VIDEO) {  // video -> none
            // Destroy video memory copy of image
            lock.release();
            _destroyImage();
            lock.lock();
        }
    }
    m_location = t_location;
}

void IE::Graphics::detail::ImageVulkan::setData(const IE::Core::MultiDimensionalVector<unsigned char> &t_data) {
    std::unique_lock<std::mutex> lock(*m_mutex);
    if (m_location & IE_IMAGE_LOCATION_NONE) {  // Nowhere for image data to go.
        // warn and return
        IE::Core::Core::getInst().logger.log(
          "Attempted to set data to an image stored in IE_IMAGE_LOCATION_NONE!",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
        return;
    }
    if (m_location & IE_IMAGE_LOCATION_SYSTEM) m_data = t_data;
    if (m_location & IE_IMAGE_LOCATION_VIDEO) {
        if (m_dimensions == t_data.getDimensions()) {
            lock.release();
            _setImageData(t_data);
            lock.lock();
        } else {
            lock.release();
            _destroyImage();
            _createImage(t_data);
            lock.lock();
        }
    }
}

bool IE::Graphics::detail::ImageVulkan::_createImage(const IE::Core::MultiDimensionalVector<unsigned char> &t_data
) {
    std::unique_lock<std::mutex> lock(*m_mutex);
    if (t_data.getDimensionality() > 4 || t_data.empty()) {
        IE::Core::Core::getInst().logger.log(
          "Images with more than 4 dimensions or less than 1 cannot be put on the GPU.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }

    // Vulkan Image specifications
    VkImageCreateInfo imageCreateInfo{
      .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      .imageType = m_type,
      .format    = m_format,
      .extent =
        VkExtent3D{
                   .width  = static_cast<uint32_t>(!m_data.empty() ? m_data.getDimensions()[0] : 0),
                   .height = static_cast<uint32_t>(m_data.getDimensionality() > 1 ? m_data.getDimensions()[1] : 1),
                   .depth  = static_cast<uint32_t>(m_data.getDimensionality() > 2 ? m_data.getDimensions()[2] : 1)},
      .mipLevels   = 1,
      .arrayLayers = static_cast<uint32_t>(m_data.getDimensionality() > 3 ? m_data.getDimensions()[3] : 1),
      .samples     = m_samples,
      .tiling      = m_tiling,
      .usage       = m_usage,
      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .initialLayout =
        VK_IMAGE_LAYOUT_UNDEFINED  // Will be changed to the requested layout after being filled with data.
    };

    // VMA allocation specifications
    VmaAllocationCreateInfo allocationCreateInfo{
      .flags          = 0,  // Use VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT for framebuffer attachments
      .usage          = m_allocationUsage,
      .requiredFlags  = 0,
      .preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      .memoryTypeBits = 0,
      .pool           = VK_NULL_HANDLE,
      .pUserData      = nullptr,
      .priority       = 0.0  // Default to the lowest priority
    };

    // Create image and perform error checking
    VkResult result{vmaCreateImage(
      m_linkedRenderEngine.lock()->getAllocator(),
      &imageCreateInfo,
      &allocationCreateInfo,
      &m_id,
      &m_allocation,
      &m_allocationInfo
    )};
    if (!result) {
        IE::Core::Core::getInst().logger.log(
          "failed to create image with error: " + IE::Graphics::RenderEngine::translateVkResultCodes(result) +
            ". Use Vulkan's validation layers for more information.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
        return false;
    }

    // Describe what parts of the image are accessible from this view.
    VkImageViewCreateInfo imageViewCreateInfo{
      .sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .flags    = 0,
      .image    = m_id,
      .viewType = m_viewType,
      .format   = m_format,
      .components =
        VkComponentMapping{
                           .r = VK_COMPONENT_SWIZZLE_B,
                           .g = VK_COMPONENT_SWIZZLE_G,
                           .b = VK_COMPONENT_SWIZZLE_B,
                           .a = VK_COMPONENT_SWIZZLE_A},
      .subresourceRange =
        VkImageSubresourceRange{
                           // View entire image
          .aspectMask     = m_aspect,
                           .baseMipLevel   = 0,
                           .levelCount     = m_mipLevel,
                           .baseArrayLayer = 0,
                           .layerCount     = static_cast<uint32_t>(m_data.getDimensionality() > 3 ? m_data.getDimensions()[3] : 1)},
    };

    result = vkCreateImageView(m_linkedRenderEngine.lock()->getDevice(), &imageViewCreateInfo, nullptr, &m_view);
    if (!result) {
        IE::Core::Core::getInst().logger.log(
          "failed to create image view with error: " + IE::Graphics::RenderEngine::translateVkResultCodes(result) +
            ". Use Vulkan's validation layers for more information.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
        return false;
    }
    m_dimensions = t_data.getDimensions();
    return true;
}

IE::Graphics::detail::ImageVulkan::~ImageVulkan() {
    std::unique_lock<std::mutex> lock(*m_mutex);
    (this->*IE::Graphics::detail::ImageVulkan::destroyImage)();
}

void IE::Graphics::detail::ImageVulkan::_destroyImage() {
    std::unique_lock<std::mutex> lock(*m_mutex);
    vkDestroyImageView(m_linkedRenderEngine.lock()->getDevice(), m_view, nullptr);
    vmaDestroyImage(m_linkedRenderEngine.lock()->getAllocator(), m_id, m_allocation);
}

void IE::Graphics::detail::ImageVulkan::_getImageData(IE::Core::MultiDimensionalVector<unsigned char> *t_pData
) const {
    std::unique_lock<std::mutex> lock(*m_mutex);
    void                        *data{m_allocationInfo.pMappedData};
    if (data == nullptr) vmaMapMemory(m_linkedRenderEngine.lock()->getAllocator(), m_allocation, &data);
    t_pData->assign((unsigned char *) data, (size_t) m_allocationInfo.size);
    vmaUnmapMemory(m_linkedRenderEngine.lock()->getAllocator(), m_allocation);
    lock.release();
}

void IE::Graphics::detail::ImageVulkan::_setImageData(const IE::Core::MultiDimensionalVector<unsigned char> &t_data
) {
    std::unique_lock<std::mutex> lock(*m_mutex);
    if (m_allocationInfo.size != t_data.size()) {
        IE::Core::Core::getInst().logger.log(
          "attempt to fill image with data that does not match its size! This may lead to broken textures or "
          "images.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    }
    void *data{m_allocationInfo.pMappedData};
    if (data == nullptr) vmaMapMemory(m_linkedRenderEngine.lock()->getAllocator(), m_allocation, &data);
    memcpy(data, t_data.data(), t_data.size());
    vmaUnmapMemory(m_linkedRenderEngine.lock()->getAllocator(), m_allocation);
}

void (IE::Graphics::detail::ImageVulkan::*const IE::Graphics::detail::ImageVulkan::destroyImage
)(){&IE::Graphics::detail::ImageVulkan::_destroyImage};