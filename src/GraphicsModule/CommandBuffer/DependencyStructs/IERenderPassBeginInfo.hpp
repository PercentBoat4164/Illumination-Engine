#pragma once

#include <memory>
#include "GraphicsModule/RenderPass/IEFramebuffer.hpp"

class IERenderPassBeginInfo {
public:
	const void *pNext{};
	std::shared_ptr<IERenderPass> renderPass{};
	std::shared_ptr<IEFramebuffer> framebuffer{};
	uint8_t framebufferIndex{};
	VkRect2D renderArea{};
	uint32_t clearValueCount{};
	const VkClearValue *pClearValues{};

	[[nodiscard]] std::vector<std::shared_ptr<IEFramebuffer>> getFramebuffers() const;

	[[nodiscard]] std::vector<std::shared_ptr<IERenderPass>> getRenderPasses() const;

	explicit operator VkRenderPassBeginInfo() const;
};