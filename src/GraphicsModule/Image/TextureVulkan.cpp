#include "TextureVulkan.hpp"

#include "IERenderEngine.hpp"

bool IE::Graphics::detail::TextureVulkan::_createSampler() {
	VkSamplerCreateInfo samplerCreateInfo{
			.sType=VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter=filter[m_magnificationFilter],
			.minFilter=filter[m_minificationFilter],
			.addressModeU=addressMode[u],
			.addressModeV=addressMode[v],
			.addressModeW=addressMode[w],
			.mipLodBias=m_mipLodBias,
			.anisotropyEnable=m_anisotropyLevel > 0,
			.maxAnisotropy=m_anisotropyLevel,
			.compareEnable=m_compareOp != VK_COMPARE_OP_NEVER,
			.compareOp=m_compareOp,
			.minLod=0.0,
			.maxLod=static_cast<float>(m_mipLevel),
			.borderColor=m_borderColor,
			.unnormalizedCoordinates=VK_FALSE
	};
	
	VkResult result{vkCreateSampler(m_linkedRenderEngine.lock()->device.device, &samplerCreateInfo, nullptr, &m_sampler)};
	if (result != VK_SUCCESS) {
		m_linkedRenderEngine.lock()->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "failed to create image sampler with error: " +
																							   IERenderEngine::translateVkResultCodes(result) +
																							   ". Use Vulkan's validation layers for more"
																							   "information.");
	}
	return result == VK_SUCCESS;
}

IE::Graphics::detail::TextureVulkan::~TextureVulkan() {
	_destroyVkSampler();
}

void IE::Graphics::detail::TextureVulkan::_destroyVkSampler() {
	vkDestroySampler(m_linkedRenderEngine.lock()->device.device, m_sampler, nullptr);
}

std::unordered_map<IE::Graphics::Texture::Filter, VkFilter> IE::Graphics::detail::TextureVulkan::filter{
		{IE::Graphics::Texture::IE_TEXTURE_FILTER_NEAREST, VK_FILTER_NEAREST},
		{IE::Graphics::Texture::IE_TEXTURE_FILTER_LINEAR,  VK_FILTER_LINEAR},
		{IE::Graphics::Texture::IE_TEXTURE_FILTER_CUBIC,   VK_FILTER_CUBIC_IMG}
};

std::unordered_map<IE::Graphics::Texture::AddressMode, VkSamplerAddressMode> IE::Graphics::detail::TextureVulkan::addressMode{
		{IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_REPEAT,                 VK_SAMPLER_ADDRESS_MODE_REPEAT},
		{IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_MIRRORED_REPEAT,        VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT},
		{IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE,          VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE},
		{IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_CLAMP_TO_BORDER,        VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER},
		{IE::Graphics::Texture::IE_TEXTURE_ADDRESS_MODE_MIRRORED_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE}
};
