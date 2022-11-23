#pragma once

#include "Image/Image.hpp"

namespace IE::Graphics {
class Material {
    std::shared_ptr<Image> diffuseTexture;
    std::shared_ptr<Image> specularTexture;
};
}  // namespace IE::Graphics
