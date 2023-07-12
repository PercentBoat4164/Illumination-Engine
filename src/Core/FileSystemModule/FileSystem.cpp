#include "FileSystem.hpp"

#include "File.hpp"

#include <filesystem>

void IE::Core::FileSystem::createFolder(const std::filesystem::path &folderPath) const {
    std::filesystem::create_directories(m_path / folderPath);
}

void IE::Core::FileSystem::exportData(const std::filesystem::path &filePath, const std::vector<char> &data) {
    m_files.find(filePath.string())->second.write(data);
}

IE::Core::File *IE::Core::FileSystem::getFile(const std::filesystem::path &filePath) {
    std::filesystem::path newPath(std::filesystem::canonical(filePath));
    createFolder(newPath.parent_path().string());
    auto iterator = m_files.find(newPath.string());
    if (iterator != m_files.end()) return &iterator->second;
    return &m_files.emplace(filePath.string(), newPath).first->second;
}

std::filesystem::path IE::Core::FileSystem::getBaseDirectory(const std::filesystem::path &t_path) {
    return t_path;
}

IE::Core::FileSystem::FileSystem() {
    m_path = std::filesystem::canonical(".");
}