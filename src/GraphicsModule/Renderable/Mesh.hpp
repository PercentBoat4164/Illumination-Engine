#pragma once

#include "Buffer/Buffer.hpp"
#include "Vertex.hpp"

#include <vector>

struct aiScene;

namespace IE::Graphics {
class RenderEngine;

class Mesh {
public:
    std::vector<Vertex>         m_vertices;
    std::vector<uint32_t>       m_indices;
    std::shared_ptr<Buffer>     m_vertexBuffer;
    std::shared_ptr<Buffer>     m_indexBuffer;
    IE::Graphics::RenderEngine *m_linkedRenderEngine{};
    uint32_t                    m_triangleCount{};

    explicit Mesh(IE::Graphics::RenderEngine *t_renderEngine);

    void load(const aiScene *t_scene);
};
}  // namespace IE::Graphics