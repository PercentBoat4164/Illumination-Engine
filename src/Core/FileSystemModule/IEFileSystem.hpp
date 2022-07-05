#pragma once

#include "IEFile.hpp"
#include "IETempFile.hpp"
#include "IEImporter.hpp"

#include <vector>
#include <string>
#include <unordered_map>
#include <variant>
#include <iostream>

class IEFileSystem {
public:
    std::filesystem::path path;
    IEImporter importer;
    std::unordered_map<std::string, IEFile> files;
    std::unordered_map<std::string, IETempFile> tempFiles;

    //constructor
    IEFileSystem(std::filesystem::path fileSystemPath) {
        path = fileSystemPath.string();
    }

    //create a new IEFile with the given relative path
    void addFile(std::string filePath) {
        files.insert(std::make_pair(filePath, IEFile(path / filePath)));
    }

    //export data to an IEFile
    void exportData(std::string filePath, std::string data) {
        files.find(filePath)->second.write(data);
    }

    //delete a file
    void deleteFile(std::string filePath) {
        files.erase(files.find(filePath));
        std::filesystem::remove(path / filePath);
    }

    //import a file
    std::string importFile(std::string filePath) {
        return importer.import(files.find(filePath)->second);
    }
};

/* old version

#define ILLUMINATION_ENGINE_ASSET_FILE_EXTENSION "iea"
enum IEPathName {
    IE_FILE_BIT = 0x1,
    IE_DIRECTORY_BIT = 0x10,
    IE_HIDDEN_BIT = 0x100,
    IE_VISIBLE_FILE = IE_FILE_BIT,
    IE_VISIBLE_DIRECTORY = IE_DIRECTORY_BIT,
    IE_HIDDEN_FILE = IE_FILE_BIT | IE_HIDDEN_BIT,
    IE_HIDDEN_DIRECTORY = IE_DIRECTORY_BIT | IE_HIDDEN_BIT
};

class IEFileSystem {
private:
    std::unordered_map<std::string, IEFile> files{};

public:
    explicit IEFileSystem(const std::string& initialAssetDirectory) {
        baseDirectory = IEDirectory{initialAssetDirectory};
    }

    explicit IEFileSystem(const IEDirectory& initialBaseDirectory) {
        baseDirectory = initialBaseDirectory;
    }

    template<typename... Args> static std::string composePath(IEPathName format, const Args&... args) {
        std::vector<std::string> arguments = {args...};
        std::string pathName{};
        for (const std::string& argument : arguments) {
            pathName += "/" + argument;
        }
        if (format & IE_FILE_BIT) {
            size_t beginFileName = pathName.find_last_of('/');
            pathName = pathName.substr(0, beginFileName) + '.' + pathName.substr(beginFileName + 1);
        }
        if (format & IE_HIDDEN_BIT) {
            size_t beginFileName = pathName.find_last_of('/');
            pathName = pathName.substr(0, beginFileName + 1) + '.' + pathName.substr(beginFileName + 1);
        }
        return pathName.substr(1);
    }

    IEFile* getAssetFile(const std::string& assetName) {
        IEFile* file = &files[assetName];
        if (file->path.empty()) {
            *file = IEFile{composePath(IE_VISIBLE_FILE, baseDirectory.path, assetName, assetName, ILLUMINATION_ENGINE_ASSET_FILE_EXTENSION)};
        }
        return file;
    }

    IEFile* getFile(const std::string& filePath, bool isCompletePath=true) {
        IEFile* file = &files[filePath];
        if (file->path.empty()) {

            // Do this if the path is complete
            if (!isCompletePath) {
                *file = IEFile{composePath(IE_VISIBLE_FILE, baseDirectory.path, filePath, filePath, ILLUMINATION_ENGINE_ASSET_FILE_EXTENSION)};
            }
            else  {
                *file = IEFile{filePath};
            }
        }
        return file;
    }

    template <typename... Args> auto getAssetDirectory(Args&&... args) -> decltype(getDirectory(std::forward<Args>(args)...)) {
        return getDirectory(std::forward<Args>(args)...);
    }

    IEDirectory* getDirectory(const std::string& assetName) {
        IEDirectory* directory = &directories[assetName];
        if (directory->path.empty()) {
            *directory = IEDirectory{composePath(IE_DIRECTORY_BIT, baseDirectory.path, assetName)};
        }
        return directory;
    }
};*/