#include "Event.hpp"

size_t std::hash<IE::Core::Event>::operator()(const IE::Core::Event &t_event) const {
    return std::hash<std::string>()(t_event.m_engineID + t_event.m_eventID);
}
