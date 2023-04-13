#include "EventActionMapping.hpp"

void IE::Core::EventActionMapping::registerEvent(
  const IE::Core::Event                      &event,
  const IE::Core::EventActionMapping::Action &action
) {
    m_eventHashTable.insert(std::pair(event, action));
}

IE::Core::EventActionMapping::Action IE::Core::EventActionMapping::getMappedAction(const IE::Core::Event &event) {
    return m_eventHashTable.find(event)->second;
}
