#include "ImageVulkan.hpp"

VkImageLayout IE::Graphics::detail::ImageVulkan::layoutFromPreset(IE::Graphics::Image::Preset t_preset) {
    return VK_IMAGE_LAYOUT_GENERAL;
}

VkFormat IE::Graphics::detail::ImageVulkan::formatFromPreset(IE::Graphics::Image::Preset t_preset) {
    return VK_FORMAT_UNDEFINED;
}
