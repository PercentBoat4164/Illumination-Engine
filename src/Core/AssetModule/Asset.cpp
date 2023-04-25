#include "Asset.hpp"

#include "Aspect.hpp"

void IE::Core::Asset::addAspect(Aspect *aspect) {
    m_aspects.emplace_back(aspect);
    aspect->associatedAssets.push_back(weak_from_this());
}
