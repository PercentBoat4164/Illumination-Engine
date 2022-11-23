#pragma once

#include <memory>
#include <string>
#include <vector>

namespace IE::Core {
class Asset;
}  // namespace IE::Core

/**
 * The Aspect class is an interface that is designed to allow the addition of new aspect types.
 * Inherit from this class to be eligible to be added as an aspect to an asset.
 * The pre-designed aspect classes are:
 *  - Renderable
 *  - More to be added
 * The user is free to add aspects as they see fit.
 * @brief An interface class used to create aspects of an Asset.
 */

namespace IE::Core {
class Aspect {
public:
    virtual ~Aspect() = default;

    std::vector<std::weak_ptr<IE::Core::Asset>>
      associatedAssets{};  // A vector of assets that this aspect belongs to
};
}  // namespace IE::Core