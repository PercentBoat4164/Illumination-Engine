#pragma once

#include "IeImage.hpp"

class IeTexture : public IeImage {
    void create(IeRenderEngineLink *engineLink, CreateInfo * createInfo) override {
        createdWith = *createInfo;
        linkedRenderEngine = engineLink;
        imageProperties.mipLevels = std::min(std::max(static_cast<unsigned int>(std::floor(std::log2(std::max(createdWith.width, createdWith.height)) + 1) * (imageProperties.filter & IE_IMAGE_FILTER_MIPMAP_ENABLED_BIT)), static_cast<unsigned int>(1)), linkedRenderEngine->settings.maxMipLevels);
        createdWith.msaaSamples = std::min(createdWith.msaaSamples, linkedRenderEngine->settings.msaaSamples);
        if (createdWith.specifications.index() == 0) {
            imageProperties.aspect = std::get<IePreDesignedImage>(createdWith.specifications) == DEPTH ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            imageProperties.type = IE_IMAGE_TYPE_2D;
            if (std::get<IePreDesignedImage>(createdWith.specifications) == TEXTURE) {
                if (!createdWith.width) { imageProperties.width = createdWith.width; }
                if (!createdWith.height) { imageProperties.height = createdWith.height; }
            } else {
                imageProperties.width = createdWith.width ? createdWith.width : linkedRenderEngine->swapchain.extent.width;
                imageProperties.height = createdWith.height ? createdWith.height : linkedRenderEngine->swapchain.extent.height;
            }
        } else if (createdWith.specifications.index() == 1) {
            imageProperties.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
            imageProperties.type = std::get<Properties>(createdWith.specifications).type;
            imageProperties.format = std::get<Properties>(createdWith.specifications).format;
        }
    }
};