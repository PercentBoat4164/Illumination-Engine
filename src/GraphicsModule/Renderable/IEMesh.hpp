#pragma once

#include <cstdint>
#include <vector>
#include "IEVertex.hpp"
#include "IEMaterial.hpp"

class IEMesh {
    std::vector<IEVertex> vertices{};
    std::vector<uint32_t> indices{};
    IEMaterial material{};
};