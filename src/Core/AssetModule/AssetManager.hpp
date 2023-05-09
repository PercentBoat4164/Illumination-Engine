#pragma once

#include "Aspect.hpp"
#include "Asset.hpp"
#include "Core/FileSystemModule/FileSystem.hpp"

#include <unordered_map>

namespace IE::Core {
class File;

class AssetManager {
private:
    IE::Core::FileSystem                               &m_filesystem;
    std::unordered_map<std::string, IE::Core::Asset *>  m_assets;
    std::unordered_map<std::string, IE::Core::Aspect *> m_aspects;

public:
    AssetManager(IE::Core::FileSystem &t_filesystem) : m_filesystem(t_filesystem) {
    }

    template<typename T>
    T &createAspect(const std::string &t_id, const std::filesystem::path &t_path) {
        return createAspect<T>(t_id, m_filesystem.addFile(t_path));
    }

    template<typename T>
    T &createAspect(const std::string &t_id, IE::Core::File *t_file) {
        return *dynamic_cast<T *>(m_aspects.insert({t_id, static_cast<Aspect *>(new T(t_file))}).first->second);
    }

    template<typename... Args>
    IE::Core::Asset &createAsset(const std::string &t_id, const std::filesystem::path &t_path, Args... args) {
        return createAsset(t_id, m_filesystem.addFile(t_path), args...);
    }

    template<typename... Args>
    IE::Core::Asset &createAsset(const std::string &t_id, IE::Core::File *t_resourceFile, Args... args) {
        return *m_assets.insert({t_id, new Asset(t_resourceFile, args...)}).first->second;
    }

    IE::Core::Aspect *getAspect(const std::string &t_id) {
        auto iterator = m_aspects.find(t_id);
        if (iterator != m_aspects.end()) return iterator->second;
        return nullptr;
    }

    IE::Core::Asset *getAsset(const std::string &t_id) {
        auto iterator = m_assets.find(t_id);
        if (iterator != m_assets.end()) return iterator->second;
        return nullptr;
    }
};
}  // namespace IE::Core