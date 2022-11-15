#include "FileSystem.hpp"

#include "File.hpp"

#include <filesystem>

IE::Core::File *IE::Core::FileSystem::addFile(const std::filesystem::path &filePath) {
    std::filesystem::path newPath(filePath);
    makePathAbsolute(newPath);
    createFolder(newPath.parent_path().string());
    auto file = getFile(newPath);
    if (file == nullptr) return &(*m_files.insert(std::make_pair(filePath.string(), File(newPath))).first).second;
    return file;
}

std::filesystem::path &IE::Core::FileSystem::makePathAbsolute(std::filesystem::path &filePath) {
    if (!filePath.string().starts_with(m_path.string())) filePath = m_path / filePath;
    return filePath;
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
    std::filesystem::path newPath(filePath);
    makePathAbsolute(newPath);
    auto iterator = m_files.find(newPath.string());
    if (iterator == m_files.end()) return nullptr;
    return &iterator->second;
}

std::filesystem::path IE::Core::FileSystem::getBaseDirectory(const std::filesystem::path &t_path) {
    return t_path;
}

void IE::Core::FileSystem::setBaseDirectory(const std::filesystem::path &t_path) {
    m_path = t_path;
    m_files.clear();
    std::filesystem::recursive_directory_iterator directory{t_path};
    for (auto entry : directory)
        if (!entry.is_directory()) addFile(entry.path());
}

IE::Core::FileSystem::FileSystem() = default;