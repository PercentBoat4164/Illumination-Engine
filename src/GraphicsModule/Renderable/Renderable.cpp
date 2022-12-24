#include "Renderable.hpp"

#include "RenderEngine.hpp"

#include <assimp/Importer.hpp>

IE::Graphics::Renderable::Renderable(IE::Core::Engine *t_engineLink, IE::Core::File *t_resource) :
        IE::Core::Aspect(t_engineLink, t_resource),
        m_linkedRenderEngine(dynamic_cast<RenderEngine *>(t_engineLink)),
        m_commandBuffer(std::make_shared<CommandBuffer>(
          dynamic_cast<RenderEngine *>(t_engineLink),
          dynamic_cast<RenderEngine *>(t_engineLink)->getCommandPool()
        )) {
    load();
}

void IE::Graphics::Renderable::update() {
    std::shared_ptr<IE::Graphics::Buffer> instanceBuffer{IE::Graphics::Buffer::create(m_linkedRenderEngine)};
    instanceBuffer->createBuffer(IE::Graphics::Buffer::IE_BUFFER_TYPE_INSTANCE_BUFFER, 0x0, nullptr, 0x0);

    vkCmdDrawIndexedIndirect(
      m_commandBuffer->m_commandBuffer,
      instanceBuffer->getVkBuffer(),
      0,
      m_instances.size(),
      sizeof(decltype(m_instances)::value_type)
    );
}

VkCommandBuffer IE::Graphics::Renderable::getCommands() {
    return m_commandBuffer->m_commandBuffer;
}

void IE::Graphics::Renderable::load() {
    const aiScene *scene;
    // load all meshes in file into one mesh.
    m_mesh.load(scene);
    // load materials
    size_t i{};
    m_materials.reserve(scene->mNumMeshes);
    for (std::shared_ptr<Material> &material : m_materials) material->load(scene, i++);
}
