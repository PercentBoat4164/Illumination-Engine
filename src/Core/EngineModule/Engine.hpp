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

    virtual Engine *create() = 0;

    template<typename T>

        requires std::derived_from<T, IE::Core::Aspect>
    std::shared_ptr<T> createAspect(const std::string &t_id) {
        std::shared_ptr<T> aspect = getAspect(t_id);
        if (aspect == nullptr) aspect = std::make_shared<T>();
        return aspect;
    }

    std::shared_ptr<Aspect> getAspect(const std::string &t_id);
};
}  // namespace IE::Core