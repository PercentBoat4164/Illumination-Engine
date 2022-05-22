#include "IERenderPassBeginInfo.hpp"

#include "IERenderPass.hpp"

std::vector<std::shared_ptr<IEImage>> IERenderPassBeginInfo::getFramebuffers() const {
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
			.framebuffer=framebuffer->framebuffer,
			.renderArea=renderArea,
			.clearValueCount=clearValueCount,
			.pClearValues=pClearValues
	};
}
