#pragma once

#include "Aspect.hpp"
#include "Asset.hpp"
#include "Core/FileSystemModule/FileSystem.hpp"

#include <memory>
#include <mutex>
#include <unordered_map>

namespace IE::Core {
class File;


/**
 * # AssetManager
 * ---
 *
 * The AssetManager's job is to record the usage of Assets and handle the construction and destruction of the
 *  Aspects that they control. An example of its usage would be to aid in the construction of a character Asset.
 *  The character has one or more 3D models, textures, animations, custom shaders, sound effects, scripts, and
 *  physics objects associated with it. All of these things can be loaded and managed independently
 *  from one another by their controlling engines using the AssetManager. An identifier of the Asset is passed to
 *  the AssetManager which then parses the Asset file and creates Aspects and associations between those Aspects
 *  and the owning Asset.
 *
 * Each Aspect can be owned by multiple Assets. In this case the Aspect is not removed until
 *  all of its owning Assets have been removed. Furthermore, shared Aspects only exist in memory once. The data at
 *  that location is shared among all Assets that use it and should therefore be modified with caution.
 */
class AssetManager {
private:
    IE::Core::FileSystem                                              &m_filesystem;
    std::unordered_map<std::string, std::shared_ptr<IE::Core::Asset>>  m_assets;
    std::mutex                                                         m_assetsMutex;
    std::unordered_map<std::string, std::shared_ptr<IE::Core::Aspect>> m_aspects;
    std::mutex                                                         m_aspectsMutex;

public:
    /**
     * The Asset Manager requires a FileSystem to use for referencing and accessing all of the files that are used
     *  by the Aspects that it controls. The default FileSystem object is the main FileSystem that is included in Core.
     */
    AssetManager();

    /**
     * The Asset Manager requires a FileSystem to use for referencing and accessing all of the files that are used
     *  by the Aspects that it controls. The default FileSystem object is the main FileSystem that is included in Core.
     *
     * @param t_filesystem
     */
    AssetManager(IE::Core::FileSystem &t_filesystem);


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

    void removeAspect(const std::string &t_id);

    void removeAspect(IE::Core::Aspect &t_aspect);

    void removeAsset(const std::string &t_id);

    void removeAsset(IE::Core::Asset &t_asset);

    std::shared_ptr<IE::Core::Aspect> getAspect(const std::string &t_id);

    std::shared_ptr<IE::Core::Asset> getAsset(const std::string &t_id);
};
}  // namespace IE::Core