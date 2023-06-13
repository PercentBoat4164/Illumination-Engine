#pragma once

#include "File.hpp"
#include "Importer.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace IE::Core {
class FileSystem {
public:
    FileSystem();

    // Create a new File with the given relative path
    File *addFile(const std::filesystem::path &filePath);

    File *getFile(const std::filesystem::path &filePath);

    void createFolder(const std::filesystem::path &folderPath) const;

    // Export data to a File
    void exportData(const std::filesystem::path &filePath, const std::vector<char> &data);

    // Delete a file
    void deleteFile(const std::filesystem::path &filePath);

    // Delete a directory
    void deleteDirectory(const std::filesystem::path &filePath) const;

    // Delete a directory that has other files in it
    void deleteUsedDirectory(const std::filesystem::path &filePath);

    void setBaseDirectory(const std::filesystem::path &t_path);

    std::filesystem::path getBaseDirectory(const std::filesystem::path &t_path);

    std::filesystem::path &makePathAbsolute(std::filesystem::path &filePath);

    template<class T>
    void importFile(T *data, File *file, unsigned int flags = 0) {
        m_importer.import(data, file, flags);
    }

    template<class T>
    void importFile(T *data, std::filesystem::path filePath, unsigned int flags = 0) {
        m_importer.import(data, getFile(filePath), flags);
    };

private:
    std::filesystem::path                 m_path;
    Importer                              m_importer{};
    std::unordered_map<std::string, File> m_files;
};
}  // namespace IE::Core