#pragma once

#include "Attachment.hpp"

#include "ImageVulkan.hpp"

#include <vulkan/vulkan_core.h>

namespace IE::Graphics::detail {
	class AttachmentVulkan : public IE::Graphics::Attachment, public IE::Graphics::detail::ImageVulkan {
	public:
	
	};
}