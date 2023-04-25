#pragma once

#include "Aspect.hpp"
#include "Asset.hpp"
#include "Core/Core.hpp"
#include "Core/FileSystemModule/File.hpp"

#include <unordered_map>

namespace IE::Core {
class AssetManager {
    std::unordered_map<std::string, IE::Core::Asset> m_assets;
    std::unordered_map<std::string, IE::Core::Aspect> m_aspects;

    template <typename T>
    auto createAspect(const std::string &t_id, const std::filesystem::path t_path) {
        return createAspect<T>(t_id, IE::Core::Core::getFileSystem()->getFile(t_path));
    }

    template <typename T>
    auto createAspect(const std::string &t_id, IE::Core::File *t_file) {
        // Load the file and read the data for the Aspect
    }

    template <typename... Args>
    auto createAsset(const std::string &t_id, Args && ... args) {
        m_assets[t_id] = Asset(args...);
    }

    auto createAsset(const std::string &t_id, std::vector<IE::Core::Aspect *> t_aspects) {
        m_assets[t_id] = Asset(t_aspects);
    }
};
}