#pragma once

#include "Core/AssetModule/Aspect.hpp"

#include <memory>
#include <unordered_map>

namespace IE::Core {
class Engine {
    using AspectType = Aspect;

protected:
    std::unordered_map<std::string, std::shared_ptr<Aspect>> m_aspects;

public:
    Engine() = default;

    Engine(const IE::Core::Engine &t_other) = default;

    Engine(IE::Core::Engine &&t_other) = default;

    Engine &operator=(const IE::Core::Engine &t_other);

    Engine &operator=(IE::Core::Engine &&t_other) noexcept;

    virtual ~Engine() = default;
};
}  // namespace IE::Core