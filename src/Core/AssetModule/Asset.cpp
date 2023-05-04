#include "Asset.hpp"

void IE::Core::Asset::addInstance(IE::Core::Instance *t_instance) {
    m_instances.push_back(t_instance);
}

void IE::Core::Asset::addInstance(IE::Core::Aspect *t_aspect) {
    m_instances.push_back(new Instance(this, t_aspect));
}

IE::Core::Asset::Asset(IE::Core::File *t_resourceFile, std::vector<IE::Core::Instance *> t_instances) :
        m_resourceFile(t_resourceFile),
        m_instances(t_instances) {
}

IE::Core::Asset::Asset(IE::Core::File *t_resourceFile, std::vector<IE::Core::Aspect *> t_aspects) :
        m_resourceFile(t_resourceFile) {
    for (IE::Core::Aspect *&aspect : t_aspects) m_instances.push_back(new Instance(this, aspect));
}

IE::Core::Asset::~Asset() {
    for (IE::Core::Instance *&instance : m_instances) delete instance;
}
