#pragma once

#include "File.hpp"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace IE::Core {
class FileSystem {
public:
    FileSystem();

    // Create a new File with the given relative m_path
    File *addFile(const std::filesystem::path &t_filePath);

    File *getFile(const std::filesystem::path &t_filePath);

    void createDirectory(const std::filesystem::path &t_directoryPath) const;

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

private:
    std::filesystem::path                 m_path;
    std::unordered_map<std::string, File> m_files;
};
}  // namespace IE::Core