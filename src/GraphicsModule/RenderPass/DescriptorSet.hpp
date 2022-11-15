#pragma once

#include "Shader.hpp"

namespace IE::Graphics {
class Subpass;

class DescriptorSet {
public:
    IE::Graphics::Subpass *m_subpass;

    DescriptorSet() = default;

    void build(IE::Graphics::Subpass *t_subpass, std::vector<IE::Graphics::Shader> &t_shaders);
};
}  // namespace IE::Graphics