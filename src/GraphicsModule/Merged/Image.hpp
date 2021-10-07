#pragma once

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

#include "Buffer.hpp"

#ifndef ILLUMINATION_ENGINE_VULKAN
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkImage)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkFormat)
#endif

enum ImageType {
    DEPTH = 0x00000000,
    COLOR = 0x00000001,
    TEXTURE = 0x00000002
};

class Image {
public:
    struct Specifications{
    public:
        unsigned int mipLevels{};
        unsigned int imageAspect{};
        unsigned int imageFormat{};
        unsigned int imageType{};
        unsigned int width{};
        unsigned int height{};
    };

    struct CreateInfo {
    public:
        //Required
        std::variant<ImageType, Specifications> format{};

        //Only required if format != TEXTURE
        int width{}, height{};
        uint8_t msaaSamples{1};
        #ifdef ILLUMINATION_ENGINE_VULKAN
        VkImageLayout imageLayout{VK_IMAGE_LAYOUT_UNDEFINED};
        #endif

        //Only required if format == TEXTURE
        std::string filename{};

        //Optional
        bool mipMapping{true};
        /**@todo: Double check that this can be stored as a string and still behave as expected.*/
        std::string *data{};

        //Optional - Only has an effect when using the Vulkan API.
        Buffer *dataSource{};
    };

    struct Created {
        bool image{};
        bool loaded{};
    };

    /**@todo: Separate the fields that can be made private so that they can be made so.*/
    #ifdef ILLUMINATION_ENGINE_VULKAN
    VkImageView view{};
    VkSampler sampler{};
    VkImageLayout imageLayout{VK_IMAGE_LAYOUT_UNDEFINED};
    VmaAllocation allocation{};
    #endif
    #ifdef ILLUMINATION_ENGINE_OPENGL
    GLenum format{};
    #endif
    CreateInfo createdWith{};
    RenderEngineLink *renderEngineLink{};
    std::variant<VkImage, unsigned int> image{};
    Created created{};
    Specifications imageProperties{};

    virtual void create(RenderEngineLink *engineLink, CreateInfo *createInfo) {
        createdWith = *createInfo;
        renderEngineLink = engineLink;
        imageProperties.mipLevels = std::min(std::max(static_cast<unsigned int>(std::floor(std::log2(std::max(createdWith.width, createdWith.height)) + 1) * createdWith.mipMapping), static_cast<unsigned int>(1)), renderEngineLink->settings.maxMipLevels);
        createdWith.msaaSamples = std::min(createdWith.msaaSamples, renderEngineLink->settings.msaaSamples);
        if (createdWith.format.index() == 0) {
            imageProperties.imageAspect = std::get<ImageType>(createdWith.format) == DEPTH ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
            imageProperties.imageType = VK_IMAGE_TYPE_2D;
            if (std::get<ImageType>(createdWith.format) == TEXTURE) {
                if (!createdWith.width) { imageProperties.width = createdWith.width; }
                if (!createdWith.height) { imageProperties.height = createdWith.height; }
            } else {
                imageProperties.width = createdWith.width ? createdWith.width : renderEngineLink->swapchain.extent.width;
                imageProperties.height = createdWith.height ? createdWith.height : renderEngineLink->swapchain.extent.height;
            }
        } else if (createdWith.format.index() == 1) {
            imageProperties.imageAspect = VK_IMAGE_ASPECT_COLOR_BIT;
            imageProperties.imageType = std::get<Specifications>(createdWith.format).imageType;
            imageProperties.imageFormat = std::get<Specifications>(createdWith.format).imageFormat;
        }
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink->api.name == "Vulkan") {
            VkImageCreateInfo imageCreateInfo{.sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
            VmaAllocationCreateInfo allocationCreateInfo{};
            if (vmaCreateImage(renderEngineLink->allocator, &imageCreateInfo, &allocationCreateInfo, &std::get<VkImage>(image), &allocation, nullptr) != VK_SUCCESS) { renderEngineLink->log->log("Failed to create image!", log4cplus::WARN_LOG_LEVEL, "Graphics Module"); }
            created.image = true;
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (renderEngineLink->api.name == "OpenGL") {
            glGenTextures(1, &std::get<unsigned int>(image));
            created.image = true;
            format = createdWith.msaaSamples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        }
        #endif
    }

    virtual void unload() {
        if (created.loaded) { stbi_image_free(createdWith.data); }
        created.loaded = false;
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink->api.name == "Vulkan") { renderEngineLink->log->log("Freed image bufferData for image at " + std::to_string(reinterpret_cast<unsigned long long int>(&image)), log4cplus::INFO_LOG_LEVEL, "Graphics Module"); }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (renderEngineLink->api.name == "OpenGL") { renderEngineLink->log->log("Freed image bufferData for image " + std::to_string(std::get<unsigned int>(image)), log4cplus::INFO_LOG_LEVEL, "Graphics Module"); }
        #endif
    }

    virtual void upload() {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink->api.name == "Vulkan") {
            VkImageSubresourceRange subresourceRange{.aspectMask=imageProperties.imageAspect, .baseMipLevel=0, .levelCount=1, .baseArrayLayer=0, .layerCount=1};
            VkImageViewCreateInfo imageViewCreateInfo{.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .viewType=VK_IMAGE_VIEW_TYPE_2D, .format=static_cast<VkFormat>(imageProperties.imageFormat), .subresourceRange=subresourceRange};
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (renderEngineLink->api.name == "OpenGL") {
            glBindTexture(GL_TEXTURE_2D, std::get<unsigned int>(image));
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, createdWith.mipMapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, createdWith.width, createdWith.height, 0, GL_RGB, GL_UNSIGNED_BYTE, createdWith.data);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, createdWith.width, createdWith.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, createdWith.data);
            if (createdWith.mipMapping) { glGenerateMipmap(GL_TEXTURE_2D); }
        }
        #endif
        created.loaded = true;
    }

    virtual void destroy() {
        unload();
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (created.image) { vmaDestroyImage(renderEngineLink->allocator, std::get<VkImage>(image), allocation); }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (created.image) { glDeleteTextures(1, &std::get<unsigned int>(image)); }
        #endif
        created.image = false;
    }
};

void Buffer::toImage(Image &image, unsigned int width, unsigned int height, VkCommandBuffer commandBuffer) {
    if (!created) { linkedRenderEngine->log->log("Called Buffer::toImage() on a Buffer that does not exist!", log4cplus::WARN_LOG_LEVEL, "Graphics Module"); }
    /**@todo: Properly handle an uncreated Buffer. i.e. Make one.*/
    /**@todo: Finish making this function.*/
}