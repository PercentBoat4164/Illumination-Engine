#include "ImageVulkan.hpp"

VkImageLayout IE::Graphics::detail::ImageVulkan::layoutFromPreset(IE::Graphics::Image::Preset t_type) {
    return VK_IMAGE_LAYOUT_GENERAL;
}