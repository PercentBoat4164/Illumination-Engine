#pragma once

#include "Core/AssetModule/IEAspect.hpp"

#include <memory>
#include <unordered_map>

namespace IE::Core {
class Engine {
protected:
    std::unordered_map<std::string, std::shared_ptr<IEAspect>> aspects;

public:
    virtual std::weak_ptr<IEAspect> createAspect(std::weak_ptr<IEAsset> asset, const std::string &filename) = 0;

    std::weak_ptr<IEAspect> findAspect(const std::string &filename);
};
}  // namespace IE::Core