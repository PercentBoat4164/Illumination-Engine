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

    //constructor
    IEFileSystem(std::filesystem::path fileSystemPath) {
        path = fileSystemPath.string();
    }

    //create a new IEFile with the given relative path
    void addFile(std::string filePath) {
        std::filesystem::path newPath(path / filePath);
        createFolder(newPath.parent_path().string());
        files.insert(std::make_pair(filePath, IEFile(newPath)));
    }

    void createFolder(std::string folderPath) {
        std::filesystem::create_directories(path / folderPath);
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

    //delete a directory
    void deleteDirectory(std::string filePath) {
        if(std::filesystem::is_empty(path / filePath) && std::filesystem::is_directory(std::filesystem::path(path.string() + "/"  + filePath))) {
            std::filesystem::remove(path / filePath);
        }
    }

    //delete a directory that has other files in it
    void deleteUsedDirectory(std::string filePath) {
        std::string testPath;
        for(auto const& x : files) {
            testPath = x.second.path.string().substr(0, filePath.size());
            if(testPath == filePath) {
                files.erase(x.first);
            }
        }
        std::filesystem::remove_all(path / filePath);
    }

    //import a file
    std::string importFile(std::string filePath) {
        return importer.import(files.find(filePath)->second);
    }

private:
    std::unordered_map<std::string, IEFile> files;
    std::unordered_map<std::string, IETempFile> tempFiles;
};