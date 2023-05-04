#include "Instance.hpp"

IE::Core::Instance::Instance(IE::Core::Asset &t_asset, IE::Core::Aspect &t_aspect) :
        m_asset(&t_asset),
        m_aspect(&t_aspect) {
}

IE::Core::Instance::Instance(IE::Core::Asset *t_asset, IE::Core::Aspect *t_aspect) :
        m_asset(t_asset),
        m_aspect(t_aspect) {
}
