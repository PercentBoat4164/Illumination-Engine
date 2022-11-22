#pragma once

#include "Engine.hpp"

namespace IE::Core {
class Window {
public:
    IE::Core::Engine *graphicsEngine{};
    IE::Core::Engine *inputEngine{};
};
}  // namespace IE::Core