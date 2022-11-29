#include "Renderable.hpp"

#include "RenderEngine.hpp"

IE::Graphics::Renderable::Renderable(IE::Core::Engine *t_engineLink, IE::Core::File *t_resource) :
        m_linkedRenderEngine(dynamic_cast<RenderEngine *>(t_engineLink)),
        m_commandBuffer(
          dynamic_cast<RenderEngine *>(t_engineLink),
          dynamic_cast<RenderEngine *>(t_engineLink)->getCommandPool()
        ),
        IE::Core::Aspect(t_engineLink, t_resource) {
}
