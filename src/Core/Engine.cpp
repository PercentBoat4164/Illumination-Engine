#include "Engine.hpp"

auto IE::Core::Engine::weak_from_this() -> std::weak_ptr<std::remove_pointer<decltype(this)>::type> {
    return {std::dynamic_pointer_cast<std::remove_pointer<decltype(this)>::type>(shared_from_this())};
}
