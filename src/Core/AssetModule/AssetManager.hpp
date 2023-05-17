#pragma once

#include "Aspect.hpp"
#include "Asset.hpp"
#include "Core/FileSystemModule/FileSystem.hpp"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace IE::Core {
class File;

class AssetManager {
private:
    IE::Core::FileSystem                                              &m_filesystem;
    std::unordered_map<std::string, std::shared_ptr<IE::Core::Asset>>  m_assets;
    std::mutex                                                         m_assetsMutex;
    std::unordered_map<std::string, std::shared_ptr<IE::Core::Aspect>> m_aspects;
    std::mutex                                                         m_aspectsMutex;

public:
    explicit AssetManager(IE::Core::FileSystem &t_filesystem);

    template<typename T>
    std::shared_ptr<T> createAspect(const std::string &t_id, const std::filesystem::path &t_path) {
        return createAspect<T>(t_id, m_filesystem.addFile(t_path));
    }

    template<typename T>
    std::shared_ptr<T> createAspect(const std::string &t_id, IE::Core::File *t_file) {
        std::lock_guard<std::mutex> lock(m_aspectsMutex);
        return std::dynamic_pointer_cast<T>(
          m_aspects.insert({t_id, std::make_shared<T>(t_id, t_file)}).first->second
        );
    }

    template<typename... Args>
    std::shared_ptr<IE::Core::Asset>
    createAsset(const std::string &t_id, const std::filesystem::path &t_path, Args... args) {
        return createAsset(t_id, m_filesystem.addFile(t_path), args...);
    }

    template<typename... Args>
    std::shared_ptr<IE::Core::Asset>
    createAsset(const std::string &t_id, IE::Core::File *t_resourceFile, Args... args) {
        std::lock_guard<std::mutex> lock(m_assetsMutex);
        return m_assets.insert({t_id, Asset::Factory(t_id, t_resourceFile, args...)}).first->second;
    }

    void removeAsset(const std::string &t_id);

    void removeAsset(IE::Core::Asset &t_asset);

    void removeAspect(const std::string &t_id);

    void removeAspect(IE::Core::Aspect &t_aspect);

    std::shared_ptr<IE::Core::Aspect> getAspect(const std::string &t_id);

    std::shared_ptr<IE::Core::Asset> getAsset(const std::string &t_id);
};
}  // namespace IE::Core