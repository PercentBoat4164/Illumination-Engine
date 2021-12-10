#pragma once

#include "IEImage.hpp"

class IETexture : public IEImage {
public:
    void create(IERenderEngineLink *engineLink, CreateInfo * createInfo) override {
        createdWith = *createInfo;
        linkedRenderEngine = engineLink;
        imageProperties.mipLevels = std::min(std::max(static_cast<uint32_t>(std::floor(std::log2(std::max(createdWith.width, createdWith.height)) + 1) * (imageProperties.filter & IE_IMAGE_FILTER_MIPMAP_ENABLED_BIT)), static_cast<uint32_t>(1)), linkedRenderEngine->settings.maxMipLevels);
        createdWith.msaaSamples = std::min(createdWith.msaaSamples, linkedRenderEngine->settings.msaaSamples);
        if (createdWith.properties.index() == 0) {
            preDesignedImage = true;
            imageProperties.type = std::get<IePreDesignedImage>(createdWith.properties) == IE_PRE_DESIGNED_TEXTURE_IMAGE ? IE_IMAGE_TYPE_2D : createdWith.msaaSamples > 1 ? IE_IMAGE_TYPE_2D_MULTISAMPLE : IE_IMAGE_TYPE_2D;
            if (std::get<IePreDesignedImage>(createdWith.properties) != IE_PRE_DESIGNED_TEXTURE_IMAGE) {
                imageProperties.width = createdWith.width ? createdWith.width : linkedRenderEngine->swapchain.extent.width;
                imageProperties.height = createdWith.height ? createdWith.height : linkedRenderEngine->swapchain.extent.height;
            }
        } else if (createdWith.properties.index() == 1) {
            preDesignedImage = true;
            imageProperties = std::get<Properties>(createdWith.properties);
        }
    }
};