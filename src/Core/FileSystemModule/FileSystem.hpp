#pragma once

#include "File.hpp"
#include "Importer.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace IE::Core {
class FileSystem {
public:
    std::filesystem::path path;
    Importer              importer{};

    explicit FileSystem(const std::filesystem::path& fileSystemPath);

    // Create a new File with the given relative path
    void addFile(const std::string& filePath);

    File & getFile(const std::string& filePath);

    void createFolder(const std::string& folderPath) const;

    // Export data to an File
    void exportData(const std::string& filePath, const std::vector<char> &data);

    // Delete a file
    void deleteFile(const std::string& filePath);

    // Delete a directory
    void deleteDirectory(const std::string& filePath) const;

    // Delete a directory that has other files in it
    void deleteUsedDirectory(const std::string& filePath);

    template<class T>
    void importFile(T* data, File &file, unsigned int flags = 0) {
        importer.import(data, file, flags);
    }

    template<class T>
    void importFile(T* data, std::string filePath, unsigned int flags = 0) {
        importer.import(data, getFile(filePath), flags);
    };

private:
    std::unordered_map<std::string, File> files;
};
}  // namespace IE::Core