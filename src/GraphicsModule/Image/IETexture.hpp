#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IEBuffer;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IEImage.hpp"

// External dependencies
#include <assimp/material.h>
#include <contrib/stb/stb_image.h>
#include <vulkan/vulkan.h>

// System dependencies
#include <cstdint>
#include <string>

struct aiTexture;

class IETexture : public IEImage {
public:
    struct CreateInfo {
        VkFormat           format{VK_FORMAT_R8G8B8A8_SRGB};
        VkImageLayout      layout{VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
        VkImageType        type{VK_IMAGE_TYPE_2D};
        VkImageUsageFlags  usage{VK_IMAGE_USAGE_SAMPLED_BIT};
        VkImageCreateFlags flags{};
        VmaMemoryUsage     allocationUsage{};
    };

    std::string filename{};

    IETexture() = default;

    IETexture(IERenderEngine *, IETexture::CreateInfo *);

    static void setAPI(const IEAPI &);


    void create(IERenderEngine *, IETexture::CreateInfo *);

private:
    static std::function<void(IETexture &)> _uploadToVRAM;

    static std::function<void(IETexture &, aiTexture *)> _uploadToRAM_texture;

    static std::function<void(IETexture &, aiTexture *)> _uploadToVRAM_texture;

    static std::function<void(IETexture &, aiTexture *)> _update_texture;

protected:
    void _openglUploadToVRAM() override;

    void _vulkanUploadToVRAM() override;


    virtual void _openglUpdate_aiTexture(aiTexture *);

    virtual void _vulkanUpdate_aiTexture(aiTexture *);


    virtual void _openglUploadToRAM_texture(aiTexture *);

    virtual void _vulkanUploadToRAM_texture(aiTexture *);


    virtual void _openglUploadToVRAM_texture(aiTexture *);

    virtual void _vulkanUploadToVRAM_texture(aiTexture *);


    void _vulkanCreateImageSampler();

public:
    using IEImage::uploadToRAM;

    void uploadToRAM(aiTexture *);

    using IEImage::uploadToVRAM;

    void uploadToVRAM(aiTexture *);


    void uploadToVRAM() override;

    using IEImage::update;


    void update(aiTexture *);


    using IEImage::unloadFromVRAM;

    using IEImage::destroy;
};