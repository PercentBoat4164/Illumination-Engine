#include "IERenderPassBeginInfo.hpp"

#include "GraphicsModule/RenderPass/IERenderPass.hpp"

std::vector<std::shared_ptr<IEFramebuffer>> IERenderPassBeginInfo::getFramebuffers() const {
	return {framebuffer};
}

std::vector<std::shared_ptr<IERenderPass>> IERenderPassBeginInfo::getRenderPasses() const {
	return {renderPass};
}

std::vector<std::shared_ptr<IEDependency>> IERenderPassBeginInfo::getDependencies() const {
	return {framebuffer, renderPass};
}

IERenderPassBeginInfo::operator VkRenderPassBeginInfo() const {
	return {
			.sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext=pNext,
			.renderPass=renderPass->renderPass,
			.framebuffer=framebuffer->getNextFramebuffer(),
			.renderArea=renderArea,
			.clearValueCount=clearValueCount,
			.pClearValues=pClearValues
	};
}
