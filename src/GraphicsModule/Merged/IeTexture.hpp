#pragma once

#include "IeImage.hpp"

class IeTexture : public IeImage {
    void create(IeRenderEngineLink *engineLink, CreateInfo * createInfo) override {
        createdWith = *createInfo;
        linkedRenderEngine = engineLink;
        imageProperties.mipLevels = std::min(std::max(static_cast<unsigned int>(std::floor(std::log2(std::max(createdWith.width, createdWith.height)) + 1) * (imageProperties.filter & IE_IMAGE_FILTER_MIPMAP_ENABLED_BIT)), static_cast<unsigned int>(1)), linkedRenderEngine->settings.maxMipLevels);
        createdWith.msaaSamples = std::min(createdWith.msaaSamples, linkedRenderEngine->settings.msaaSamples);
        if (createdWith.specifications.index() == 0) {
            preDesignedImage = true;
            imageProperties.type = std::get<IePreDesignedImage>(createdWith.specifications) == TEXTURE ? IE_IMAGE_TYPE_2D : createdWith.msaaSamples > 1 ? IE_IMAGE_TYPE_2D_MULTISAMPLE : IE_IMAGE_TYPE_2D;
            if (std::get<IePreDesignedImage>(createdWith.specifications) != TEXTURE) {
                imageProperties.width = createdWith.width ? createdWith.width : linkedRenderEngine->swapchain.extent.width;
                imageProperties.height = createdWith.height ? createdWith.height : linkedRenderEngine->swapchain.extent.height;
            }
        } else if (createdWith.specifications.index() == 1) {
            preDesignedImage = true;
            imageProperties = std::get<Properties>(createdWith.specifications);
        }
    }
};