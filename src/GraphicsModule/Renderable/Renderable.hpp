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
public:
    IE::Graphics::RenderEngine                 *m_linkedRenderEngine;
    DescriptorSet                               m_descriptorSet{DescriptorSet::IE_DESCRIPTOR_SET_TYPE_PER_OBJECT};
    Mesh                                        m_mesh;
    std::vector<std::shared_ptr<Material>>      m_materials;
    std::vector<Instance>                       m_instances;
    std::vector<std::shared_ptr<CommandBuffer>> m_commandBuffers;

    Renderable(IE::Core::Engine *t_engineLink, IE::Core::File *t_resource);

    std::vector<VkCommandBuffer> getCommands();

    void load();

    ~Renderable() = default;
};
}  // namespace IE::Graphics