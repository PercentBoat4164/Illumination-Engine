#include "IEFileSystem.hpp"

#include <filesystem>

IEFileSystem::IEFileSystem(const std::filesystem::path &fileSystemPath) {
	path = fileSystemPath.string();
}

void IEFileSystem::addFile(const std::string &filePath) {
	std::filesystem::path newPath(path / filePath);
	createFolder(newPath.parent_path().string());
	files.insert(std::make_pair(filePath, IEFile(newPath)));
}

void IEFileSystem::createFolder(const std::string &folderPath) const {
	std::filesystem::create_directories(path / folderPath);
}

void IEFileSystem::exportData(const std::string &filePath, const std::vector<char> &data) {
	files.find(filePath)->second.write(data);
}

void IEFileSystem::deleteFile(const std::string &filePath) {
	files.erase(files.find(filePath));
	std::filesystem::remove(path / filePath);
}

void IEFileSystem::deleteDirectory(const std::string &filePath) const {
	if(std::filesystem::is_empty(path / filePath) && std::filesystem::is_directory(std::filesystem::path(path.string() + "/"  + filePath))) {
		std::filesystem::remove(path / filePath);
	}
}

void IEFileSystem::deleteUsedDirectory(const std::string &filePath) {
	std::string testPath;
	for(auto const& x : files) {
		testPath = x.second.path.string().substr(0, filePath.size());
		if(testPath == filePath) {
			files.erase(x.first);
		}
	}
	std::filesystem::remove_all(path / filePath);
}

IEFile &IEFileSystem::getFile(const std::string &filePath) {
    return files.find(filePath)->second;
}



//template<class T>
//void IEFileSystem::importFile(T* data, IEFile &file, unsigned int flags) {
//    importer.import(data, file, flags);
//}
