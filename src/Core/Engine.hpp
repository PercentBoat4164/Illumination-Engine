#pragma once

#include <memory>

namespace IE::Core { class Core; }

namespace IE::Core {
class Engine : private std::enable_shared_from_this<Engine> {
public:
    virtual auto weak_from_this() -> std::weak_ptr<std::remove_pointer<decltype(this)>::type>;

    std::shared_ptr<IE::Core::Core> m_core;
};
}  // namespace IE::Core