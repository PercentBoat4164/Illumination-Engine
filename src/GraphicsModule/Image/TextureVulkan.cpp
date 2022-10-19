#include "TextureVulkan.hpp"

#include "RenderEngine.hpp"

bool IE::Graphics::detail::TextureVulkan::_createSampler() {
    std::unique_lock<std::mutex> lock(*m_mutex);
    VkSamplerCreateInfo          samplerCreateInfo{
               .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
               .magFilter               = filter.at(m_magnificationFilter),
               .minFilter               = filter.at(m_minificationFilter),
               .addressModeU            = addressMode.at(u),
               .addressModeV            = addressMode.at(v),
               .addressModeW            = addressMode.at(w),
               .mipLodBias              = m_mipLodBias,
               .anisotropyEnable        = m_anisotropyLevel > 0,
               .maxAnisotropy           = m_anisotropyLevel,
               .compareEnable           = m_compareOp != VK_COMPARE_OP_NEVER,
               .compareOp               = m_compareOp,
               .minLod                  = 0.0,
               .maxLod                  = static_cast<float>(m_mipLevel),
               .borderColor             = m_borderColor,
               .unnormalizedCoordinates = VK_FALSE};

    VkResult result{
      vkCreateSampler(m_linkedRenderEngine->m_device.device, &samplerCreateInfo, nullptr, &m_sampler)};
    if (result != VK_SUCCESS) {
        IE::Core::Core::getInst().getLogger()->log(
          "failed to create image sampler with error: " +
            IE::Graphics::RenderEngine::translateVkResultCodes(result) +
            ". Use Vulkan's validation layers for more"
            "information.",
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }
    return result == VK_SUCCESS;
}

IE::Graphics::detail::TextureVulkan::~TextureVulkan() {
    (this->*IE::Graphics::detail::TextureVulkan::destroyTexture)();
}

const std::unordered_map<IE::Graphics::Texture::Filter, VkFilter> IE::Graphics::detail::TextureVulkan::filter{
  {IE::Graphics::Texture::IE_TEXTURE_FILTER_NEAREST, VK_FILTER_NEAREST  },
  {IE::Graphics::Texture::IE_TEXTURE_FILTER_LINEAR,  VK_FILTER_LINEAR   },
  {IE::Graphics::Texture::IE_TEXTURE_FILTER_CUBIC,   VK_FILTER_CUBIC_IMG}
};

const std::unordered_map<IE::Graphics::Texture::AddressMode, VkSamplerAddressMode>
  IE::Graphics::detail::TextureVulkan::addressMode{
    {IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_REPEAT,               VK_SAMPLER_ADDRESS_MODE_REPEAT         },
    {IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_MIRRORED_REPEAT,      VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT},
    {IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE,        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE  },
    {IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_CLAMP_TO_BORDER,      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER},
    {IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE,
     VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE                                                                }
};

bool IE::Graphics::detail::TextureVulkan::_createImage(
  const IE::Core::MultiDimensionalVector<unsigned char> &t_data
) {
    return ImageVulkan::_createImage(t_data) && _createSampler();
}

void IE::Graphics::detail::TextureVulkan::_destroyImage() {
    vkDestroySampler(m_linkedRenderEngine->m_device.device, m_sampler, nullptr);
    ImageVulkan::_destroyImage();
}

void (IE::Graphics::detail::TextureVulkan::*const IE::Graphics::detail::TextureVulkan::destroyTexture
)(){&IE::Graphics::detail::TextureVulkan::_destroyImage};

IE::Graphics::detail::TextureVulkan::TextureVulkan() noexcept :
        m_sampler{VK_NULL_HANDLE},
        m_compareOp{VK_COMPARE_OP_NEVER},
        m_borderColor{VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK} {
}

template<typename... Args>
IE::Graphics::detail::TextureVulkan::TextureVulkan(
  const std::weak_ptr<IE::Graphics::RenderEngine> &t_engineLink,
  Args... t_dimensions
) :
        Texture(t_engineLink, t_dimensions...),
        m_sampler{VK_NULL_HANDLE},
        m_compareOp{VK_COMPARE_OP_NEVER},
        m_borderColor{VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK} {
}

IE::Graphics::detail::TextureVulkan &
IE::Graphics::detail::TextureVulkan::operator=(IE::Graphics::detail::TextureVulkan &&t_other) noexcept {
    if (&t_other != this) {
        m_sampler     = std::exchange(t_other.m_sampler, VK_NULL_HANDLE);
        m_compareOp   = std::exchange(t_other.m_compareOp, VK_COMPARE_OP_NEVER);
        m_borderColor = std::exchange(t_other.m_borderColor, VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK);
    }
    return *this;
}

IE::Graphics::detail::TextureVulkan &
IE::Graphics::detail::TextureVulkan::operator=(const IE::Graphics::detail::TextureVulkan &t_other) {
    if (&t_other != this) {
        m_sampler     = t_other.m_sampler;
        m_compareOp   = t_other.m_compareOp;
        m_borderColor = t_other.m_borderColor;
    }
    return *this;
}
