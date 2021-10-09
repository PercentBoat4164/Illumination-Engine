#pragma once

#include "Image.hpp"

class Texture : public Image {
    void create(RenderEngineLink *engineLink, CreateInfo * createInfo) override {
        createdWith = *createInfo;
        linkedRenderEngine = engineLink;
        imageProperties.mipLevels = std::min(std::max(static_cast<unsigned int>(std::floor(std::log2(std::max(createdWith.width, createdWith.height)) + 1) * createdWith.mipMapping), static_cast<unsigned int>(1)), linkedRenderEngine->settings.maxMipLevels);
        createdWith.msaaSamples = std::min(createdWith.msaaSamples, linkedRenderEngine->settings.msaaSamples);
        if (createdWith.specifications.index() == 0) {
            imageProperties.aspect = std::get<ImageType>(createdWith.specifications) == DEPTH ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            imageProperties.type = VK_IMAGE_TYPE_2D;
            if (std::get<ImageType>(createdWith.specifications) == TEXTURE) {
                if (!createdWith.width) { imageProperties.width = createdWith.width; }
                if (!createdWith.height) { imageProperties.height = createdWith.height; }
            } else {
                imageProperties.width = createdWith.width ? createdWith.width : linkedRenderEngine->swapchain.extent.width;
                imageProperties.height = createdWith.height ? createdWith.height : linkedRenderEngine->swapchain.extent.height;
            }
        } else if (createdWith.specifications.index() == 1) {
            imageProperties.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
            imageProperties.type = std::get<Specifications>(createdWith.specifications).type;
            imageProperties.format = std::get<Specifications>(createdWith.specifications).format;
        }
    }
};