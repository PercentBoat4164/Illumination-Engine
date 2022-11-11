#include "FileSystem.hpp"

#include "File.hpp"

#include <filesystem>

IE::Core::FileSystem::FileSystem(const std::filesystem::path &fileSystemPath) {
    path = fileSystemPath.string();
}

IE::Core::File* IE::Core::FileSystem::addFile(const std::filesystem::path &filePath) {
    std::filesystem::path newPath(path / filePath);
    createFolder(newPath.parent_path().string());
    return &(* files.insert(std::make_pair(filePath.string(), File(newPath))).first).second;
}

void IE::Core::FileSystem::createFolder(const std::filesystem::path &folderPath) const {
    std::filesystem::create_directories(path / folderPath);
}

void IE::Core::FileSystem::exportData(const std::filesystem::path &filePath, const std::vector<char> &data) {
    files.find(filePath.string())->second.write(data);
}

void IE::Core::FileSystem::deleteFile(const std::filesystem::path &filePath) {
    files.erase(files.find(filePath.string()));
    std::filesystem::remove(path / filePath);
}

void IE::Core::FileSystem::deleteDirectory(const std::filesystem::path &filePath) const {
    if (std::filesystem::is_empty(path / filePath) && std::filesystem::is_directory(std::filesystem::path(path.string() + "/" + filePath.string())))
        std::filesystem::remove(path / filePath);
}

void IE::Core::FileSystem::deleteUsedDirectory(const std::filesystem::path &filePath) {
    std::string testPath;
    for (const auto &x : files) {
        testPath = x.second.path.string().substr(0, filePath.string().size());
        if (testPath == filePath) files.erase(x.first);
    }
    std::filesystem::remove_all(path / filePath);
}

IE::Core::File *IE::Core::FileSystem::getFile(const std::filesystem::path &filePath) {
    return &files.find(filePath.string())->second;
}

// template<class T>
// void FileSystem::importFile(T* data, File &file, unsigned int flags) {
//     importer.import(data, file, flags);
// }