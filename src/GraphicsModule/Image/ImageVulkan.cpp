#include "ImageVulkan.hpp"

/**@todo Implement this function.*/
VkImageLayout IE::Graphics::detail::ImageVulkan::layoutFromPreset(IE::Graphics::Image::Preset t_preset) {
    return VK_IMAGE_LAYOUT_GENERAL;
}

/**@todo Implement this function.*/
VkFormat IE::Graphics::detail::ImageVulkan::formatFromPreset(IE::Graphics::Image::Preset t_preset) {
    return VK_FORMAT_UNDEFINED;
}

/**@todo Implement this function.*/
VkAccessFlags IE::Graphics::detail::ImageVulkan::accessFlagsFromPreset(IE::Graphics::Image::Preset t_preset) {
    return VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
}
