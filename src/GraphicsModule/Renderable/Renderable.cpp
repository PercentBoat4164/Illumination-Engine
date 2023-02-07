#include "Renderable.hpp"

#include "RenderEngine.hpp"

#include <assimp/Importer.hpp>

IE::Graphics::Renderable::Renderable(IE::Core::Engine *t_engineLink, IE::Core::File *t_resource) :
        IE::Core::Aspect(t_engineLink, t_resource),
        m_linkedRenderEngine(dynamic_cast<RenderEngine *>(t_engineLink)) {
    m_mesh.m_linkedRenderEngine = m_linkedRenderEngine;
    load();
}

std::vector<VkCommandBuffer> IE::Graphics::Renderable::getCommands() {
    std::vector<VkCommandBuffer> commands(m_commandBuffers.size());
    for (size_t i{}; i < m_commandBuffers.size(); ++i) commands.push_back(m_commandBuffers[i]->m_commandBuffer);
    return commands;
}

void IE::Graphics::Renderable::load() {
    const aiScene *scene;
    IE::Core::Core::getFileSystem()->importFile(&scene, m_resourceFile);
    // load all meshes in file into one mesh.
    m_mesh.load(scene);
    // load materials
    size_t i{};
    m_materials.resize(scene->mNumMaterials);
    for (std::shared_ptr<Material> &material : m_materials) {
        material = std::make_shared<Material>();
        material->load(scene, scene->mMaterials[i++], m_resourceFile);
    }
}
