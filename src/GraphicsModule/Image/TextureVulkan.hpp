#pragma once
#include "Texture.hpp"
#include "ImageVulkan.hpp"

#include <vulkan/vulkan_core.h>
#include <array>
#include <unordered_map>

namespace IE::Graphics::detail {
	class TextureVulkan : public IE::Graphics::Texture, public IE::Graphics::detail::ImageVulkan {
	public:
		VkSampler m_sampler;
		VkCompareOp m_compareOp;
		VkBorderColor m_borderColor;
		
		bool _createSampler();
		
		void _destroyVkSampler();
		
		~TextureVulkan() override;
		
		static std::unordered_map<Filter, VkFilter> filter;
		
		static std::unordered_map<AddressMode, VkSamplerAddressMode> addressMode;
	};
}