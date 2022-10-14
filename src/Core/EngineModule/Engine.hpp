#pragma once

#include "Core/AssetModule/IEAspect.hpp"

#include <memory>
#include <unordered_map>

namespace IE::Core {
class Engine {
private:
    static uint64_t m_nextId;

protected:
    std::unordered_map<std::string, std::shared_ptr<IEAspect>> m_aspects;
    uint64_t                                                   m_id;

    Engine() : m_id(m_nextId++) {
    }

public:
    virtual ~Engine() = default;

    Engine(const IE::Core::Engine &t_other) = default;

    Engine(IE::Core::Engine &&t_other) = default;

    Engine &operator=(const IE::Core::Engine &t_other) {
        if (this == &t_other) {
            m_aspects = t_other.m_aspects;
            m_id      = t_other.m_id;
        }
        return *this;
    }

    Engine &operator=(IE::Core::Engine &&t_other) noexcept {
        if (this == &t_other) {
            m_aspects = t_other.m_aspects;
            m_id      = t_other.m_id;
        }
        return *this;
    }

    virtual std::weak_ptr<IEAspect> createAspect(std::weak_ptr<IEAsset> asset, const std::string &filename) = 0;

    std::weak_ptr<IEAspect> findAspect(const std::string &filename);
};
}  // namespace IE::Core