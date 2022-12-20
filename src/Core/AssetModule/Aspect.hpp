#pragma once

#include <memory>
#include <string>
#include <vector>

namespace IE::Core {
class Engine;
class File;
class Asset;

/**
 * The Aspect class is an interface that is designed to allow the addition of new aspect types.
 * Inherit from this class to be eligible to be added as an aspect to an asset.
 * The pre-designed aspect classes are:
 *  - Renderable
 *  - Keyboard
 *  - More to be added
 * The user is free to add aspects as they see fit.
 * @brief An interface class used to create aspects of an Asset.
 */

class Aspect {
public:
    Aspect(IE::Core::Engine *t_engine, IE::Core::File *t_resource) {}

    virtual ~Aspect() = default;

    IE::Core::File *m_resourceFile;

    // A vector of assets that this aspect belongs to
    std::vector<std::weak_ptr<IE::Core::Asset>> associatedAssets{};
};
}  // namespace IE::Core