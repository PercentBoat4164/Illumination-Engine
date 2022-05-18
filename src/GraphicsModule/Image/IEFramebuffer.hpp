#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IEImage.hpp"

// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <string>
#include <vector>


class IEFramebuffer : public IEImage {
public:
	struct CreateInfo {
		VkImageView swapchainImageView{};
		VkRenderPass renderPass{};
		std::vector<float> defaultColor{0.0F, 0.0F, 0.0F, 1.0F};
	};

	VkFramebuffer framebuffer{};
	std::vector<VkClearValue> clearValues{3};
	std::vector<float> defaultColor{0.0F, 0.0F, 0.0F, 1.0F};
	VkImageView swapchainImageView{};
	VkRenderPass renderPass{};
	std::shared_ptr<IEImage> colorImage{};  /**@todo Eliminate this and use the inbuilt image.*/
	std::shared_ptr<IEImage> depthImage{};

	IEFramebuffer();

	void copyCreateInfo(IEFramebuffer::CreateInfo *createInfo);

	void create(IERenderEngine *engineLink, CreateInfo *createInfo);
};