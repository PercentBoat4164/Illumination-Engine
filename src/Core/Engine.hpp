#pragma once

#include <memory>

namespace IE::Core {
class Core;
}  // namespace IE::Core

namespace IE::Core {
class Engine {
public:
    virtual ~Engine() = default;

    Engine(const IE::Core::Engine &t_other) = default;

    Engine(IE::Core::Engine &&t_other) = default;

    Engine &operator=(const IE::Core::Engine &t_other) {
        if (this == &t_other) m_id = t_other.m_id;
        return *this;
    }

    Engine &operator=(IE::Core::Engine &&t_other) noexcept {
        if (this == &t_other) m_id = std::exchange(t_other.m_id, 0);
        return *this;
    }

protected:
    uint64_t m_id;  // The ID of this Engine.

    Engine() : m_id(m_nextId++) {
    }

private:
    static uint64_t m_nextId;
};
}  // namespace IE::Core