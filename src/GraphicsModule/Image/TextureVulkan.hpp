#pragma once
#include "ImageVulkan.hpp"
#include "Texture.hpp"

#include <array>
#include <unordered_map>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics::detail {
class TextureVulkan : public IE::Graphics::Texture, public IE::Graphics::detail::ImageVulkan {
public:
    VkSampler     m_sampler;
    VkCompareOp   m_compareOp;
    VkBorderColor m_borderColor;

    TextureVulkan() noexcept;

    template<typename... Args>
    explicit TextureVulkan(const std::weak_ptr<IERenderEngine> &t_engineLink, Args... t_dimensions);

    TextureVulkan &operator=(TextureVulkan &&t_other) noexcept;

    TextureVulkan &operator=(const TextureVulkan &t_other);

    ~TextureVulkan() override;

    static const std::unordered_map<Filter, VkFilter> filter;

    static const std::unordered_map<AddressMode, VkSamplerAddressMode> addressMode;

protected:
    bool _createSampler() override;

    bool _createImage(const IE::Core::MultiDimensionalVector<unsigned char> &t_data) override;

    void _destroyImage() override;

private:
    // Used to call the _destroyImage function of this class in the destructor without worrying about virtuality.
    static void (IE::Graphics::detail::TextureVulkan::*const destroyTexture)();
};
}  // namespace IE::Graphics::detail