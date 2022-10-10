#pragma once

#include "InheritableSharedFromThis.hpp"

#include <memory>

namespace IE::Core {
class Core;
}  // namespace IE::Core

namespace IE::Core {
class Engine : public inheritable_enable_shared_from_this<Engine> {
public:
    uint64_t m_id;  // The ID of this Engine.

private:
    static uint64_t m_nextId;
};
}  // namespace IE::Core