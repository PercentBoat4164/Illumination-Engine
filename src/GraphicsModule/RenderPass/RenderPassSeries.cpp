#include "RenderPassSeries.hpp"

#include "RenderEngine.hpp"

IE::Graphics::RenderPassSeries::RenderPassSeries(IE::Graphics::RenderEngine *t_engineLink) :
        m_linkedRenderEngine(t_engineLink) {
}

auto IE::Graphics::RenderPassSeries::build() -> decltype(*this) {
    // For each subpass in each render pass in the render pass series:
    //
    return *this;
}

auto IE::Graphics::RenderPassSeries::addRenderPass(IE::Graphics::RenderPass &t_pass) -> decltype(*this) {
    m_renderPasses.push_back(t_pass);
    m_renderPasses[m_renderPasses.size() - 1].m_linkedRenderEngine = m_linkedRenderEngine;
    return *this;
}
