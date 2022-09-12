#pragma once

#include "Image.hpp"

namespace IE::Graphics {
	class Texture : virtual public IE::Graphics::Image {
	public:
		enum Filter {
			IE_TEXTURE_FILTER_NEAREST = 0x0,
			IE_TEXTURE_FILTER_LINEAR = 0x1,
			IE_TEXTURE_FILTER_CUBIC = 0x2,
		};
		
		enum AddressMode {
			IE_TEXTURE_ADDRESS_MODE_REPEAT = 0x0,
			IE_TEXTURE_ADDRESS_MODE_MIRRORED_REPEAT = 0x1,
			IE_TEXTURE_ADDRESS_MODE_CLAMP_TO_EDGE = 0x2,
			IE_TEXTURE_ADDRESS_MODE_CLAMP_TO_BORDER = 0x3,
			IE_TEXTURE_ADDRESS_MODE_MIRRORED_CLAMP_TO_EDGE = 0x4,
		};
		
		float m_mipLodBias;
		float m_anisotropyLevel;
		Filter m_magnificationFilter;
		Filter m_minificationFilter;
		AddressMode u;
		AddressMode v;
		AddressMode w;
	};
}
