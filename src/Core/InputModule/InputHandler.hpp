#pragma once

#include "Core/AssetModule/Aspect.hpp"
#include "Core/EngineModule/Engine.hpp"
#include "Keyboard.hpp"

struct GLFWwindow;

namespace IE::Core {
class InputHandler {
public:
    Keyboard m_keyboard;
};
}  // namespace IE::Core