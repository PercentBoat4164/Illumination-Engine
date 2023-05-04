#pragma once

#include "Instance.hpp"

#include <memory>
#include <string>
#include <vector>

namespace IE::Core {
class Asset;
class File;

/**
 * The Aspect class is an interface that is designed to allow the addition of new aspect types.
 * Inherit from this class to be eligible to be added as an aspect to an asset.
 * The user is free to add m_instances as they see fit.
 * @brief An interface class used to create m_instances of an Asset.
 */
class Aspect {
public:
    virtual ~Aspect() = default;

    std::vector<IE::Core::Instance *> m_instances;
    IE::Core::File                   *m_resourceFile;

    Aspect(IE::Core::File *t_resourceFile);
};
}  // namespace IE::Core
