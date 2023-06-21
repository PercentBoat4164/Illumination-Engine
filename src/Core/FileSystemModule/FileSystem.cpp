#include "FileSystem.hpp"

#include "File.hpp"

#include <filesystem>

IE::Core::File *IE::Core::FileSystem::addFile(const std::filesystem::path &filePath) {
    std::filesystem::path newPath(std::filesystem::absolute(filePath));
    createFolder(newPath.parent_path().string());
    auto file = getFile(newPath);
    if (file == nullptr) return &(*m_files.insert(std::make_pair(filePath.string(), File(newPath))).first).second;
    return file;
}

void IE::Core::FileSystem::createFolder(const std::filesystem::path &folderPath) const {
    std::filesystem::create_directories(m_path / folderPath);
}

void IE::Core::FileSystem::exportData(const std::filesystem::path &filePath, const std::vector<char> &data) {
    m_files.find(filePath.string())->second.write(data);
}

void IE::Core::FileSystem::deleteFile(const std::filesystem::path &filePath) {
    m_files.erase(m_files.find(filePath.string()));
    std::filesystem::remove(m_path / filePath);
}

void IE::Core::FileSystem::deleteDirectory(const std::filesystem::path &filePath) const {
    if (std::filesystem::is_empty(m_path / filePath) && std::filesystem::is_directory(std::filesystem::path(m_path.string() + "/" + filePath.string())))
        std::filesystem::remove(m_path / filePath);
}

void IE::Core::FileSystem::deleteUsedDirectory(const std::filesystem::path &filePath) {
    std::string testPath;
    for (const auto &x : m_files) {
        testPath = x.second.path.string().substr(0, filePath.string().size());
        if (testPath == filePath) m_files.erase(x.first);
    }
    std::filesystem::remove_all(m_path / filePath);
}

IE::Core::File *IE::Core::FileSystem::getFile(const std::filesystem::path &filePath) {
    std::filesystem::path newPath(std::filesystem::absolute(filePath));
    auto iterator = m_files.find(newPath.string());
    if (iterator == m_files.end()) return nullptr;
    return &iterator->second;
}

std::filesystem::path IE::Core::FileSystem::getBaseDirectory() {
    return m_path;
}

void IE::Core::FileSystem::setBaseDirectory(const std::filesystem::path &t_path) {
    m_path = std::filesystem::absolute(t_path);
//    if constexpr (IE_OS == IE_MACOS) {
//        m_internalResourcesPath = m_path / ".." / "Resources";
//    }
}

void IE::Core::FileSystem::setInternalResourcesDirectory(const std::filesystem::path &t_path) {
    m_internalResourcesPath = t_path;
}

std::filesystem::path IE::Core::FileSystem::getInternalResourcesDirectory() {
    return m_internalResourcesPath;
}

IE::Core::FileSystem::FileSystem() = default;