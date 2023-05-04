#pragma once

#include "Aspect.hpp"
#include "Asset.hpp"
#include "Core/FileSystemModule/FileSystem.hpp"

#include <unordered_map>

namespace IE::Core {
class File;

class AssetManager {
private:
    IE::Core::FileSystem                             &m_filesystem;
    std::unordered_map<std::string, IE::Core::Asset>  m_assets;
    std::unordered_map<std::string, IE::Core::Aspect> m_aspects;

public:
    AssetManager(IE::Core::FileSystem &t_filesystem) : m_filesystem(t_filesystem) {
    }

    template<typename T>
    auto createAspect(const std::string &t_id, const std::filesystem::path t_path) -> T * {
        return createAspect<T>(t_id, m_filesystem.getFile(t_path));
    }

    template<typename T>
    T *createAspect(const std::string &t_id, IE::Core::File *t_file) {
        return m_aspects.insert({t_id, T(t_file)}).first->second;
    }

    IE::Core::Asset &createAsset(const std::string &t_id, IE::Core::File *t_resourceFile) {
        return m_assets.insert({t_id, IE::Core::Asset(t_resourceFile)}).first->second;
    }
};
}  // namespace IE::Core