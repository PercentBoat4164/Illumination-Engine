#include "IEFile.hpp"
#include "IEDirectory.hpp"

#include <vector>
#include <string>
#include <unordered_map>

#define ILLUMINATION_ENGINE_ASSET_FILE_EXTENSION "iea"

class IEFileSystem {
private:
    std::unordered_map<std::string, IEFile> files{};
    std::string assetDirectory{};

    static std::string stringFilename(const std::string& thisPartOfPath) {
        return "." + thisPartOfPath;
    }

    template<typename... Args> static std::string stringFilename(const std::string& thisPartOfPath, Args... args) {
        return thisPartOfPath + "/" + stringFilename(args...);
    }

    static std::string stringPath(const std::string& thisPartOfPath) {
        return thisPartOfPath;
    }

    template<typename... Args> static std::string stringPath(const std::string& thisPartOfPath, Args... args) {
        return thisPartOfPath + "/" + stringPath(args...);
    }

public:
    explicit IEFileSystem(const std::string& initialAssetDirectory) {
        assetDirectory = initialAssetDirectory;
    }

    template<typename... Args> static std::string composeFilename(Args... args) {
        std::string filename{stringFilename(args...)};
        size_t position{filename.find_last_of('/')};
        return filename.substr(0, position) + filename.substr(position + 1);
    }

    template<typename... Args> static std::string composePath(Args... args) {
        return stringPath(args...);
    }

    IEFile* getAssetFile(const std::string& assetName) {
        IEFile* file = &files[assetName];
        if (!file->exists()) {
            file->path = composeFilename(assetDirectory, assetName, assetName, ILLUMINATION_ENGINE_ASSET_FILE_EXTENSION);
        }
        return file;
    }

    std::string getAssetDirectory(const std::string& assetName) {
        return composePath(assetDirectory, assetName);
    }
};