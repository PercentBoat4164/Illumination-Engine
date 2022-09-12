#pragma once

#include "IERenderEngine.hpp"
#include "Core/AssetModule/IEAspect.hpp"
#include "Image/Image.hpp"
#include "RenderPass/IEFramebuffer.hpp"

class IELight : public IEAspect {
public:
	IEFramebuffer framebuffer{};
	IE::Graphics::Image depthImage{};
	IERenderEngine *linkedRenderEngine{};
	
	IELight(IERenderEngine *engineLink) : linkedRenderEngine(engineLink){
		IEImage::CreateInfo depthImageCreateInfo {
			.format=VK_FORMAT_D16_UNORM,  // 16 bit depth image
			.aspect=VK_IMAGE_ASPECT_DEPTH_BIT,
			.width=1024,
			.height=1024,
			.channels=1
		};
		depthImage.create(linkedRenderEngine, &depthImageCreateInfo);
		framebuffer.create(linkedRenderEngine, {
				.renderPass=linkedRenderEngine->shadowPass,
				.attachments={
						.image=depthImage,
				}
		});
	}
};