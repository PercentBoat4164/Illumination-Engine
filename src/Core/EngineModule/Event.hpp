#pragma once

#include <string>
#include <utility>
#include <vector>

namespace IE::Core {
class Event {
public:
    Event(Engine *t_engine, std::string t_eventID) :
            m_engineID(t_engine->getID()),
            m_eventID(std::move(t_eventID)) {
    }

    Event(std::string t_engineID, std::string t_eventID) :
            m_engineID(std::move(t_engineID)),
            m_eventID(std::move(t_eventID)) {
    }

    bool operator==(const Event &t_other) const = default;

private:
    std::string m_engineID;
    std::string m_eventID;
};
}  // namespace IE::Core

template<>
struct [[maybe_unused]] std::hash<IE::Core::Event> {
    size_t operator()(const IE::Core::Event &t_event) const;
};