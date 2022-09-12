#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

class IERenderPassBeginInfo;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "IEFramebuffer.hpp"
#include "CommandBuffer/IECommandBuffer.hpp"

// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <cstdint>
#include <vector>
#include <functional>


class IERenderEngine;

class IERenderPass : public std::enable_shared_from_this<IERenderPass> {
public:
	struct CreateInfo {
		uint8_t msaaSamples{1};
	} createdWith;

	VkRenderPass renderPass{};
	std::shared_ptr<IEFramebuffer> framebuffer{};

	void create(IERenderEngine *engineLink, CreateInfo *createInfo);

	IERenderPassBeginInfo beginRenderPass(uint8_t index);

	void destroy();

	~IERenderPass();

private:
	std::vector<std::function<void()>> deletionQueue{};
	IERenderEngine *linkedRenderEngine{};
};
