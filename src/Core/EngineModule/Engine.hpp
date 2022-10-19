#pragma once

#include "Core/AssetModule/IEAspect.hpp"

#include <memory>
#include <unordered_map>

namespace IE::Core {
class Engine {
    using AspectType = IEAspect;

protected:
    std::unordered_map<std::string, std::shared_ptr<IEAspect>> m_aspects;

public:
    Engine() = default;

    Engine(const IE::Core::Engine &t_other) = default;

    Engine(IE::Core::Engine &&t_other) = default;

    Engine &operator=(const IE::Core::Engine &t_other);

    Engine &operator=(IE::Core::Engine &&t_other) noexcept;

    virtual ~Engine() = default;


    virtual Engine *create() = 0;

    virtual IEAspect *createAspect(std::weak_ptr<IEAsset> t_asset, const std::string &t_id) = 0;

    virtual IEAspect *getAspect(const std::string &t_id);
};
}  // namespace IE::Core