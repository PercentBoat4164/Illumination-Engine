#include "Asset.hpp"

void IE::Core::Asset::addAspect(std::shared_ptr<IE::Core::Aspect> t_aspect) {
    m_aspects.emplace_back(t_aspect);
    t_aspect->associatedAssets.push_back(weak_from_this());
}

IE::Core::Asset::Asset(IE::Core::File *t_file) : m_file(t_file) {
}
