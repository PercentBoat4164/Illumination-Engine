#pragma once

#include "Core/AssetModule/Aspect.hpp"
#include "Image/Image.hpp"
#include "RenderPass/Framebuffer.hpp"

class Light : public IE::Core::Aspect {
public:
    IE::Graphics::Image depthImage{};
    RenderEngine       *linkedRenderEngine{};

    Light(IERenderEngine *engineLink) : linkedRenderEngine(engineLink) {
        Image::CreateInfo depthImageCreateInfo{
          .format   = VK_FORMAT_D16_UNORM,  // 16 bit depth image
          .aspect   = VK_IMAGE_ASPECT_DEPTH_BIT,
          .width    = 1024,
          .height   = 1024,
          .channels = 1};
        depthImage.create(linkedRenderEngine, &depthImageCreateInfo);
    }
};