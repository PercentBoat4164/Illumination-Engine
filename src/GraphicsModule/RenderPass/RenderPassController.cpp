#include "RenderPassController.hpp"
#include "RenderPassBuilder.hpp"

#include "Image/Attachment.hpp"

#include "IERenderEngine.hpp"

IE::Graphics::RenderPassController::RenderPassController() :
		m_renderEngineLink(),
		m_renderPasses(),
		m_currentRenderPass() {}

template<typename RenderPassType>
void IE::Graphics::RenderPassController::addRenderPass(const RenderPassType &t_renderPass, const std::string &t_name) {
	m_currentRenderPass = t_renderPass;
	m_renderPasses[t_name] = m_currentRenderPass;
}

// Construct and connect all render passes controlled by this controller.
void IE::Graphics::RenderPassController::build() {

}
