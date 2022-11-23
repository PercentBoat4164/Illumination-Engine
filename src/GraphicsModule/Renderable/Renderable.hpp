#pragma once

#include "CommandBuffer/CommandBuffer.hpp"
#include "Core/AssetModule/Aspect.hpp"
#include "DescriptorSet/DescriptorSet.hpp"
#include "Instance.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

namespace IE::Core {
class File;
}  // namespace IE::Core

namespace IE::Graphics {
class Renderable : public IE::Core::Aspect {
public:
    IE::Graphics::RenderEngine *m_linkedRenderEngine;
    DescriptorSet               m_descriptorSet{DescriptorSet::IE_DESCRIPTOR_SET_TYPE_PER_OBJECT};
    std::shared_ptr<Material>   m_material;
    Mesh                        m_mesh;
    Instance                    m_instance;  // This can be extended to a vector to support instancing.
    CommandBuffer               m_commandBuffer;
    IE::Core::File             *m_file;

    Renderable(IE::Graphics::RenderEngine *t_engineLink);

    void createRenderable() {
    }
};
}  // namespace IE::Graphics