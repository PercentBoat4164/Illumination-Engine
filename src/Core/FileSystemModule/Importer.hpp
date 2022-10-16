#pragma once

#include "File.hpp"

#include <assimp/Importer.hpp>

class IETexture;

/*
 * A class used to import files into a program. One per filesystem.
 */
namespace IE::Core {
class Importer {
public:
    Assimp::Importer importer{};

    void import(const aiScene **, File &, uint32_t);

    static void import(std::string *, File &, uint32_t);

    /**@todo Find a way around needing to define each variant of this function for user defined or IE types.*/
    friend void import(IETexture *, File &, uint32_t);  // Needs to be implemented in IETexture.
};
}  // namespace IE::Core