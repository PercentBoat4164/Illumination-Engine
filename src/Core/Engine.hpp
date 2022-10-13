#pragma once

#include "InheritableSharedFromThis.hpp"

#include <memory>

namespace IE::Core {
class Core;
}  // namespace IE::Core

namespace IE::Core {
class Engine : public inheritable_enable_shared_from_this<Engine> {
public:
    const uint64_t m_id;  // The ID of this Engine.

    virtual ~Engine() = default;

protected:
    Engine() : m_id(m_nextId++) {
    }

private:
    static uint64_t m_nextId;
};
}  // namespace IE::Core