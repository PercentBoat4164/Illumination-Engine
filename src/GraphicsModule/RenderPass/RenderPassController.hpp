#pragma once

#include "RenderPass.hpp"

#include <unordered_map>
#include <memory>
#include <vector>

class IERenderEngine;

namespace IE::Graphics {
	class Attachment;
	
	class RenderPassController {
	public:
		std::weak_ptr<IERenderEngine> m_renderEngineLink;
		std::unordered_map<std::string, IE::Graphics::RenderPass *> m_renderPasses;
		IE::Graphics::RenderPass *m_currentRenderPass;
		
		RenderPassController();
		
		template<typename RenderPassType>
		void addRenderPass(const RenderPassType &t_renderPass, const std::string &t_name);
		
		void build();
	};
}