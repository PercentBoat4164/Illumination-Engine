#pragma once

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#endif

enum ImageType {
    DEPTH = 0x00000000,
    COLOR = 0x00000001,
    TEXTURE = 0x00000002
};

class Image {
public:
    struct CreateInfo {
    public:
        //Required
        ImageType format{};

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

    CreateInfo createdWith{};
    RenderEngineLink *renderEngineLink{};
    #ifdef ILLUMINATION_ENGINE_VULKAN
    VkImage image{};
    VkImageView view{};
    VkSampler sampler{};
    VkFormat imageFormat{};
    VkImageLayout imageLayout{};
    #endif
    #ifdef ILLUMINATION_ENGINE_OPENGL
    GLuint ID{};
    GLenum format{};
    #endif
    uint32_t mipLevels{};
    Created created{};

    virtual void create(RenderEngineLink *engineLink, CreateInfo *createInfo) {
        createdWith = *createInfo;
        renderEngineLink = engineLink;
        createdWith.msaaSamples = std::min(createdWith.msaaSamples, renderEngineLink->settings.msaaSamples);
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (renderEngineLink->api.name == "OpenGL") {
            glGenTextures(1, &ID);
            format = createdWith.msaaSamples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        }
        #endif
        created.image = true;
    }

    virtual void unload() {
        stbi_image_free(createdWith.data);
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (renderEngineLink->api.name == "Vulkan") { renderEngineLink->log->log("Freed image data for image at " + std::to_string(reinterpret_cast<unsigned long long int>(&image)), log4cplus::INFO_LOG_LEVEL, "Graphics Module"); }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (renderEngineLink->api.name == "OpenGL") { renderEngineLink->log->log("Freed image data for image " + std::to_string(ID), log4cplus::INFO_LOG_LEVEL, "Graphics Module"); }
        #endif
        created.loaded = false;
    }

    virtual void upload() {
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, createdWith.mipMapping ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (createdWith.format == COLOR) { glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, createdWith.width, createdWith.height, 0, GL_RGB, GL_UNSIGNED_BYTE, createdWith.data); }
        if (createdWith.format == DEPTH) { glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, createdWith.width, createdWith.height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, createdWith.data); }
        if (createdWith.mipMapping) { glGenerateMipmap(GL_TEXTURE_2D); }
        created.loaded = true;
    }

    virtual void destroy() {
        unload();
        glDeleteTextures(1, &ID);
        created.image = false;
    }

private:
    VkImageCreateInfo imageCreateInfo{.sType=VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
};