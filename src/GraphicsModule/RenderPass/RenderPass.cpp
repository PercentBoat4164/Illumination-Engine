#include "RenderPass.hpp"

#include "RenderPassSeries.hpp"

IE::Graphics::RenderPass::RenderPass(Preset t_preset) : m_preset(t_preset) {
}

auto IE::Graphics::RenderPass::build() -> decltype(*this) {
    return *this;
}
