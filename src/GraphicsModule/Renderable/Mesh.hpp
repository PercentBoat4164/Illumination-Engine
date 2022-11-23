#pragma once

#include "Buffer/Buffer.hpp"
#include "Vertex.hpp"

#include <vector>

namespace IE::Graphics {
class Mesh {
    std::vector<Vertex>     vertices;
    std::vector<uint32_t>   indices;
    std::shared_ptr<Buffer> vertexBuffer;
    std::shared_ptr<Buffer> indexBuffer;
};
}  // namespace IE::Graphics