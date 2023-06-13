#pragma once

#include "Core/Core.hpp"
#include "Engine.hpp"
#include "Event.hpp"

#include <concepts>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace IE::Core {
class EventActionMapping {
    using Action = std::function<void()>;
    std::unordered_multimap<IE::Core::Event, Action> m_eventHashTable{};

    template<typename E>
        requires std::derived_from<E, IE::Core::Engine>
    void registerEvent(
      const IE::Core::Event          &event,
      const std::function<void(E &)> &action,
      const std::string              &engineID
    ) {
        std::shared_ptr<E> engine = IE::Core::getEngine<E>(engineID);
        m_eventHashTable.insert(std::pair(event, [engine, action] { action(*engine); }));
    }

    template<typename E>
        requires std::derived_from<E, IE::Core::Engine>
    void registerEvent(
      const IE::Core::Event          &event,
      const std::function<void(E *)> &action,
      const std::string              &engineID
    ) {
        m_eventHashTable.insert(std::pair(event, IE::Core::getEngine<Engine>(engineID)->bindFunction<E>(action)));
    }

    template<typename E>
        requires std::derived_from<E, IE::Core::Engine>
    void registerEvent(
      const IE::Core::Event          &event,
      const std::function<void(E *)> &action,
      const std::shared_ptr<E>       &engine
    ) {
        m_eventHashTable.insert(std::pair(event, engine->template bindFunction<E>(action)));
    }

    void registerEvent(const IE::Core::Event &event, const Action &action);

    Action getMappedAction(const IE::Core::Event &event);
};
}  // namespace IE::Core