#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or _function arguments. */
// Internal dependencies
#include "GraphicsModule/Image/IEFramebuffer.hpp"
#include "CommandBuffer/IECommandBuffer.hpp"

// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <cstdint>
#include <vector>
#include <functional>


class IERenderEngine;

class IERenderPass : public IEDependency {
public:
	struct CreateInfo {
		uint8_t msaaSamples{1};
	} createdWith;

	VkRenderPass renderPass{};
	std::vector<IEFramebuffer> framebuffers{};

	void create(IERenderEngine *engineLink, CreateInfo *createInfo);

	IERenderPassBeginInfo beginRenderPass(uint32_t framebufferIndex);

	void destroy();

	~IERenderPass() override;

private:
	std::vector<std::function<void()>> deletionQueue{};
	IERenderEngine *linkedRenderEngine{};
};
