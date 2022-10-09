/* Include this file's header. */
#include "IETexture.hpp"

/* Include dependencies within this module. */
#include "Buffer/IEBuffer.hpp"
#include "IERenderEngine.hpp"

/* Include dependencies from Core. */
#include "assimp/texture.h"
#include "Core/LogModule/IELogger.hpp"

IETexture::IETexture(IERenderEngine *engineLink, IETexture::CreateInfo *createInfo) {
    create(engineLink, createInfo);
}

void IETexture::setAPI(const IEAPI &API) {
    if (API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
        _update_texture       = &IETexture::_openglUpdate_aiTexture;
        _uploadToRAM_texture  = &IETexture::_openglUploadToRAM_texture;
        _uploadToVRAM         = &IETexture::_openglUploadToVRAM;
        _uploadToVRAM_texture = &IETexture::_openglUploadToVRAM_texture;
    } else if (API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
        _update_texture       = &IETexture::_vulkanUpdate_aiTexture;
        _uploadToRAM_texture  = &IETexture::_vulkanUploadToRAM_texture;
        _uploadToVRAM         = &IETexture::_vulkanUploadToVRAM;
        _uploadToVRAM_texture = &IETexture::_vulkanUploadToVRAM_texture;
    }
}

void IETexture::create(IERenderEngine *engineLink, IETexture::CreateInfo *createInfo) {
    linkedRenderEngine = engineLink;
    layout             = createInfo->layout;
    format             = createInfo->format;
    type               = createInfo->type;
    usage              = createInfo->usage | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    status             = static_cast<IEImageStatus>(
      IE_IMAGE_STATUS_UNLOADED | IE_IMAGE_STATUS_QUEUED_RAM | IE_IMAGE_STATUS_QUEUED_VRAM
    );

    /**@todo Implement mip-mapping again.*/
    // Determine image data based on settings and input data
    //        auto maxMipLevel = static_cast<uint8_t>(std::floor(std::log2(std::max(width, height))) + 1);
    //        mipLevels = mipMapping && linkedRenderEngine->settings.mipMapping ? std::min(std::max(maxMipLevel,
    //        static_cast<uint8_t>(1)), static_cast<uint8_t>(linkedRenderEngine->settings.mipMapLevel)) : 1;
}

std::function<void(IETexture &)> IETexture::_uploadToVRAM{nullptr};

void IETexture::uploadToVRAM() {
    if (width * height * channels == 0) {
        linkedRenderEngine->settings->logger.log(
          "Attempt to load image with size of zero into VRAM",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_DEBUG
        );
    }
    if (status & IE_IMAGE_STATUS_QUEUED_VRAM) _uploadToVRAM(*this);

    // Update status
    status = static_cast<IEImageStatus>(status & ~IE_IMAGE_STATUS_QUEUED_VRAM | IE_IMAGE_STATUS_IN_VRAM);
}

void IETexture::_openglUploadToVRAM() {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_SRGB8_ALPHA8,
      (GLsizei) width,
      (GLsizei) height,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      data.data()
    );
    glBindTexture(GL_TEXTURE_2D, 0);
    update(data);
}

void IETexture::_vulkanUploadToVRAM() {
    VkImageLayout desiredLayout = layout;
    layout                      = VK_IMAGE_LAYOUT_UNDEFINED;

    _vulkanCreateImage();
    _vulkanCreateImageView();
    _vulkanCreateImageSampler();

    update(data);

    // Set transition to requested layout from undefined or dst_optimal.
    if (layout != desiredLayout) transitionLayout(desiredLayout);
}

std::function<void(IETexture &, aiTexture *)> IETexture::_update_texture{nullptr};

void IETexture::update(aiTexture *texture) {
    _update_texture(*this, texture);
}

void IETexture::_openglUpdate_aiTexture(aiTexture *texture) {
    stbi_uc *tempData;
    if (texture->mHeight == 0) {
        tempData = stbi_load_from_memory(
          (unsigned char *) texture->pcData,
          (int) texture->mWidth,
          reinterpret_cast<int *>(&width),
          reinterpret_cast<int *>(&height),
          reinterpret_cast<int *>(&channels),
          4
        );
    } else {
        tempData = stbi_load(
          texture->mFilename.C_Str(),
          reinterpret_cast<int *>(&width),
          reinterpret_cast<int *>(&height),
          reinterpret_cast<int *>(&channels),
          4
        );
    }
    channels = 4; /**@todo Make number of channels imported change the number of channels in VRAM.*/
    std::vector<char> data =
      std::vector<char>{(char *) tempData, (char *) ((uint64_t) tempData + width * height * channels)};
    if (data.empty()) {
        linkedRenderEngine->settings->logger.log(
          std::string{"Failed to load image data from file: '"} + texture->mFilename.C_Str() + "' due to " +
            stbi_failure_reason(),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    }
    _openglUpdate_vector(data);
}

void IETexture::_vulkanUpdate_aiTexture(aiTexture *texture) {
    stbi_uc *tempData;
    if (texture->mHeight == 0) {
        tempData = stbi_load_from_memory(
          (unsigned char *) texture->pcData,
          (int) texture->mWidth,
          reinterpret_cast<int *>(&width),
          reinterpret_cast<int *>(&height),
          reinterpret_cast<int *>(&channels),
          4
        );
    } else {
        tempData = stbi_load(
          texture->mFilename.C_Str(),
          reinterpret_cast<int *>(&width),
          reinterpret_cast<int *>(&height),
          reinterpret_cast<int *>(&channels),
          4
        );
    }
    channels = 4; /**@todo Make number of channels imported change the number of channels in VRAM.*/
    std::vector<char> data =
      std::vector<char>{(char *) tempData, (char *) ((uint64_t) tempData + width * height * channels)};
    if (data.empty()) {
        linkedRenderEngine->settings->logger.log(
          std::string{"Failed to load image data from file: '"} + texture->mFilename.C_Str() + "' due to " +
            stbi_failure_reason(),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    }
    _vulkanUpdate_vector(data);
}

std::function<void(IETexture &, aiTexture *)> IETexture::_uploadToVRAM_texture{nullptr};

void IETexture::uploadToVRAM(aiTexture *texture) {
    _uploadToVRAM_texture(*this, texture);
}

void IETexture::_openglUploadToVRAM_texture(aiTexture *texture) {
    stbi_uc *tempData;
    if (texture->mHeight == 0) {
        tempData = stbi_load_from_memory(
          (unsigned char *) texture->pcData,
          (int) texture->mWidth,
          reinterpret_cast<int *>(&width),
          reinterpret_cast<int *>(&height),
          reinterpret_cast<int *>(&channels),
          4
        );
    } else {
        tempData = stbi_load(
          texture->mFilename.C_Str(),
          reinterpret_cast<int *>(&width),
          reinterpret_cast<int *>(&height),
          reinterpret_cast<int *>(&channels),
          4
        );
    }
    channels = 4; /**@todo Make number of channels imported change the number of channels in VRAM.*/
    std::vector<char> data =
      std::vector<char>{(char *) tempData, (char *) ((uint64_t) tempData + width * height * channels)};
    if (data.empty()) {
        linkedRenderEngine->settings->logger.log(
          std::string{"Failed to load image data from file: '"} + texture->mFilename.C_Str() + "' due to " +
            stbi_failure_reason(),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    }

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_SRGB8_ALPHA8,
      (GLsizei) width,
      (GLsizei) height,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      data.data()
    );
    glBindTexture(GL_TEXTURE_2D, 0);

    status = static_cast<IEImageStatus>(status | IE_IMAGE_STATUS_IN_VRAM);
}

void IETexture::_vulkanUploadToVRAM_texture(aiTexture *texture) {
    stbi_uc *tempData;
    if (texture->mHeight == 0) {
        tempData = stbi_load_from_memory(
          (unsigned char *) texture->pcData,
          (int) texture->mWidth,
          reinterpret_cast<int *>(&width),
          reinterpret_cast<int *>(&height),
          reinterpret_cast<int *>(&channels),
          4
        );
    } else {
        tempData = stbi_load(
          texture->mFilename.C_Str(),
          reinterpret_cast<int *>(&width),
          reinterpret_cast<int *>(&height),
          reinterpret_cast<int *>(&channels),
          4
        );
    }
    channels = 4; /**@todo Make number of channels imported change the number of channels in VRAM.*/
    std::vector<char> data =
      std::vector<char>{(char *) tempData, (char *) ((uint64_t) tempData + width * height * channels)};
    if (data.empty()) {
        linkedRenderEngine->settings->logger.log(
          std::string{"Failed to load image data from file: '"} + texture->mFilename.C_Str() + "' due to " +
            stbi_failure_reason(),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    }

    _vulkanCreateImage();
    _vulkanCreateImageView();
    _vulkanCreateImageSampler();

    status = static_cast<IEImageStatus>(status | IE_IMAGE_STATUS_IN_VRAM);
}

std::function<void(IETexture &, aiTexture *)> IETexture::_uploadToRAM_texture{nullptr};

void IETexture::uploadToRAM(aiTexture *texture) {
    _uploadToRAM_texture(*this, texture);
}

void IETexture::_openglUploadToRAM_texture(aiTexture *texture) {
    stbi_uc *tempData;
    if (texture->mHeight == 0) {
        tempData = stbi_load_from_memory(
          (unsigned char *) texture->pcData,
          (int) texture->mWidth,
          reinterpret_cast<int *>(&width),
          reinterpret_cast<int *>(&height),
          reinterpret_cast<int *>(&channels),
          4
        );
    } else {
        tempData = stbi_load(
          texture->mFilename.C_Str(),
          reinterpret_cast<int *>(&width),
          reinterpret_cast<int *>(&height),
          reinterpret_cast<int *>(&channels),
          4
        );
    }
    channels = 4; /**@todo Make number of channels imported change the number of channels in VRAM.*/
    data     = std::vector<char>{(char *) tempData, (char *) ((uint64_t) tempData + width * height * channels)};
    if (data.empty()) {
        linkedRenderEngine->settings->logger.log(
          std::string{"Failed to load image data from file: '"} + texture->mFilename.C_Str() + "' due to " +
            stbi_failure_reason(),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    }
}

void IETexture::_vulkanUploadToRAM_texture(aiTexture *texture) {
    stbi_uc *tempData;
    if (texture->mHeight == 0) {
        tempData = stbi_load_from_memory(
          (unsigned char *) texture->pcData,
          (int) texture->mWidth,
          reinterpret_cast<int *>(&width),
          reinterpret_cast<int *>(&height),
          reinterpret_cast<int *>(&channels),
          4
        );
    } else {
        tempData = stbi_load(
          texture->mFilename.C_Str(),
          reinterpret_cast<int *>(&width),
          reinterpret_cast<int *>(&height),
          reinterpret_cast<int *>(&channels),
          4
        );
    }
    channels = 4; /**@todo Make number of channels imported change the number of channels in VRAM.*/
    data     = std::vector<char>{(char *) tempData, (char *) ((uint64_t) tempData + width * height * channels)};
    if (data.empty()) {
        linkedRenderEngine->settings->logger.log(
          std::string{"Failed to load image data from file: '"} + texture->mFilename.C_Str() + "' due to " +
            stbi_failure_reason(),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
    }
}

void IETexture::_vulkanCreateImageSampler() {
    // Set up image sampler create info.
    VkSamplerCreateInfo samplerCreateInfo{
      .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
      .magFilter               = VK_FILTER_NEAREST,
      .minFilter               = VK_FILTER_NEAREST,
      .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
      .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
      .mipLodBias              = linkedRenderEngine->settings->mipMapLevel,
      //                .anisotropyEnable=linkedRenderEngine->settings.anisotropicFilterLevel > 0,
      //                .maxAnisotropy=anisotropyLevel,
      .compareEnable           = VK_FALSE,
      .compareOp               = VK_COMPARE_OP_ALWAYS,
      .minLod                  = 0.0F,
      //                .maxLod=static_cast<float>(mipLevels),
      .borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE,
      .unnormalizedCoordinates = VK_FALSE,
    };

    // Create image sampler
    if (vkCreateSampler(linkedRenderEngine->device.device, &samplerCreateInfo, nullptr, &sampler) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture sampler!");
}