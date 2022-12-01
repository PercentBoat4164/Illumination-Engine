#pragma once

#include "Buffer/BufferVulkan.hpp"
#include "CommandBuffer/CommandBuffer.hpp"
#include "Core/AssetModule/Aspect.hpp"
#include "DescriptorSet/DescriptorSet.hpp"
#include "Instance.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

namespace IE::Core {
class File;
class Engine;
}  // namespace IE::Core

namespace IE::Graphics {
class Renderable : public IE::Core::Aspect {
private:
    std::shared_ptr<IE::Graphics::Buffer> buildInstanceBuffer() {
        std::shared_ptr<IE::Graphics::Buffer> instanceBuffer{IE::Graphics::Buffer::create(m_linkedRenderEngine)};
        instanceBuffer->createBuffer(IE::Graphics::Buffer::IE_BUFFER_TYPE_INSTANCE_BUFFER, 0x0, nullptr, 0x0);
        return instanceBuffer;
    }

public:
    IE::Graphics::RenderEngine *m_linkedRenderEngine;
    DescriptorSet               m_descriptorSet{DescriptorSet::IE_DESCRIPTOR_SET_TYPE_PER_OBJECT};
    std::shared_ptr<Material>   m_material;
    Mesh                        m_mesh;
    std::vector<Instance>       m_instances;
    CommandBuffer               m_commandBuffer;
    IE::Core::File             *m_file;

    Renderable(IE::Core::Engine *t_engineLink, IE::Core::File *t_resource);

    void update() {
        auto instanceBuffer = buildInstanceBuffer();

        vkCmdDrawIndexedIndirect(
          m_commandBuffer.m_commandBuffer,
          instanceBuffer->m_id,
          0,
          m_instances.size(),
          sizeof(decltype(m_instances)::value_type)
        );
    }
};
}  // namespace IE::Graphics