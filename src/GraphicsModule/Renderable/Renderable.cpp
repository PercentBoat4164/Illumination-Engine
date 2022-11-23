#include "Renderable.hpp"

#include "RenderEngine.hpp"

IE::Graphics::Renderable::Renderable(IE::Graphics::RenderEngine *t_engineLink) :
        m_linkedRenderEngine(t_engineLink),
        m_commandBuffer(t_engineLink, t_engineLink->getCommandPool()) {
}
