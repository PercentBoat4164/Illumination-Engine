#include "Asset.hpp"

void Asset::addAspect(Aspect *aspect) {
    aspects.emplace_back(aspect);
    aspect->associatedAssets.push_back(weak_from_this());
}
