#include "Instance.hpp"

#include "Aspect.hpp"
#include "Asset.hpp"

IE::Core::Instance::Instance(IE::Core::Asset &t_asset, IE::Core::Aspect &t_aspect) :
        m_asset(&t_asset),
        m_aspect(&t_aspect) {
    m_asset->m_instances.push_back(this);
    m_aspect->m_instances.push_back(this);
}

IE::Core::Instance::Instance(IE::Core::Asset *t_asset, IE::Core::Aspect *t_aspect) :
        m_asset(t_asset),
        m_aspect(t_aspect) {
    m_asset->m_instances.push_back(this);
    m_aspect->m_instances.push_back(this);
}
