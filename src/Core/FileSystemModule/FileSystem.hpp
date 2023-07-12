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

    File *getFile(const std::filesystem::path &filePath);

    void createFolder(const std::filesystem::path &folderPath) const;

    // Export data to a File
    void exportData(const std::filesystem::path &filePath, const std::vector<char> &data);

    std::filesystem::path getBaseDirectory(const std::filesystem::path &t_path);

private:
    std::filesystem::path                 m_path;
    std::unordered_map<std::string, File> m_files;
};
}  // namespace IE::Core