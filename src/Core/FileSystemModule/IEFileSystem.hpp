#include "IEDirectory.hpp"
#include "IEFile.hpp"

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#define ILLUMINATION_ENGINE_ASSET_FILE_EXTENSION "iea"

enum IEPathName {
    IE_FILE_BIT          = 0x1,
    IE_DIRECTORY_BIT     = 0x10,
    IE_HIDDEN_BIT        = 0x100,
    IE_VISIBLE_FILE      = IE_FILE_BIT,
    IE_VISIBLE_DIRECTORY = IE_DIRECTORY_BIT,
    IE_HIDDEN_FILE       = IE_FILE_BIT | IE_HIDDEN_BIT,
    IE_HIDDEN_DIRECTORY  = IE_DIRECTORY_BIT | IE_HIDDEN_BIT
};

class IEFileSystem {
private:
    std::unordered_map<std::string, IEFile>        files{};
    std::unordered_map<std::string, IEDirectory>   directories{};
    std::vector<std::variant<IEFile, IEDirectory>> trash{};
    IEDirectory                                    baseDirectory{};

public:
    explicit IEFileSystem(const std::string &initialAssetDirectory);

    explicit IEFileSystem(const IEDirectory &initialBaseDirectory);

    template<typename... Args>
    static std::string composePath(IEPathName format, const Args &...args);

    IEFile *getAssetFile(const std::string &assetName) {
        IEFile *file = &files[assetName];
        if (file->path.empty()) {
            *file = IEFile{composePath(
              IE_VISIBLE_FILE,
              baseDirectory.path,
              assetName,
              assetName,
              ILLUMINATION_ENGINE_ASSET_FILE_EXTENSION
            )};
        }
        return file;
    }

    IEFile *getFile(const std::string &filePath, bool isCompletePath = true) {
        IEFile *file = &files[filePath];
        if (file->path.empty()) {

            // Do this if the path is complete
            if (!isCompletePath) {
                *file = IEFile{composePath(
                  IE_VISIBLE_FILE,
                  baseDirectory.path,
                  filePath,
                  filePath,
                  ILLUMINATION_ENGINE_ASSET_FILE_EXTENSION
                )};
            } else {
                *file = IEFile{filePath};
            }
        }
        return file;
    }

    template<typename... Args>
    auto getAssetDirectory(Args &&...args) -> decltype(getDirectory(std::forward<Args>(args)...)) {
        return getDirectory(std::forward<Args>(args)...);
    }

    IEDirectory *getDirectory(const std::string &assetName) {
        IEDirectory *directory = &directories[assetName];
        if (directory->path.empty())
            *directory = IEDirectory{composePath(IE_DIRECTORY_BIT, baseDirectory.path, assetName)};
        return directory;
    }
};