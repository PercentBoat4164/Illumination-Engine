#include "Asset.hpp"

void IE::Core::Asset::addAspect(std::shared_ptr<IE::Core::Aspect> t_aspect) {
    m_aspects.emplace_back(t_aspect);
    t_aspect->m_associatedAssets.push_back(this);
}

IE::Core::Asset::Asset(IE::Core::File *t_file) : m_file(t_file) {
}

IE::Core::Asset::~Asset() {
    for (const std::shared_ptr<IE::Core::Aspect> &aspect : m_aspects) {
        erase_if(aspect->m_associatedAssets, [&](Asset *a){return a==this;});
    }
}