#include "FileSystem.hpp"

#include "File.hpp"

#include <filesystem>

IE::Core::FileSystem::FileSystem(const std::filesystem::path &fileSystemPath) {
	path = fileSystemPath.string();
}

void IE::Core::FileSystem::addFile(const std::string &filePath) {
	std::filesystem::path newPath(path / filePath);
	createFolder(newPath.parent_path().string());
	files.insert(std::make_pair(filePath, File(newPath)));
}

void IE::Core::FileSystem::createFolder(const std::string &folderPath) const {
	std::filesystem::create_directories(path / folderPath);
}

void IE::Core::FileSystem::exportData(const std::string &filePath, const std::vector<char> &data) {
	files.find(filePath)->second.write(data);
}

void IE::Core::FileSystem::deleteFile(const std::string &filePath) {
	files.erase(files.find(filePath));
	std::filesystem::remove(path / filePath);
}

void IE::Core::FileSystem::deleteDirectory(const std::string &filePath) const {
	if(std::filesystem::is_empty(path / filePath) && std::filesystem::is_directory(std::filesystem::path(path.string() + "/"  + filePath))) {
		std::filesystem::remove(path / filePath);
	}
}

void IE::Core::FileSystem::deleteUsedDirectory(const std::string &filePath) {
	std::string testPath;
	for(auto const& x : files) {
		testPath = x.second.path.string().substr(0, filePath.size());
		if(testPath == filePath) {
			files.erase(x.first);
		}
	}
	std::filesystem::remove_all(path / filePath);
}

IE::Core::File &IE::Core::FileSystem::getFile(const std::string &filePath) {
    return files.find(filePath)->second;
}



//template<class T>
//void FileSystem::importFile(T* data, File &file, unsigned int flags) {
//    importer.import(data, file, flags);
//}
