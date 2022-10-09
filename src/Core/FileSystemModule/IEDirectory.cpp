#include "IEDirectory.hpp"

IEDirectory::IEDirectory(const std::string &initialPath) {
    path = initialPath;
}

void IEDirectory::createDirectory() const {
    for (size_t i = 0; i < path.size(); ++i)
        if (path[i] == '/') std::filesystem::create_directory(path.substr(0, i));
    std::filesystem::create_directory(path);
}

std::vector<IEDirectory *> IEDirectory::allDirectories() {
    std::vector<IEDirectory *> allSubDirectories{subDirectories};
    for (IEDirectory *subDirectory : subDirectories) {
        std::vector<IEDirectory *> theseSubDirectories{subDirectory->allDirectories()};
        allSubDirectories.insert(allSubDirectories.end(), theseSubDirectories.begin(), theseSubDirectories.end());
    }
    return allSubDirectories;
}

std::vector<IEFile *> IEDirectory::allFiles() {
    std::vector<IEFile *> allFiles{files};
    for (IEDirectory *subDirectory : subDirectories) {
        std::vector<IEFile *> theseFiles{subDirectory->allFiles()};
        allFiles.insert(allFiles.end(), theseFiles.begin(), theseFiles.end());
    }
    return allFiles;
}

void IEDirectory::create() {
    createDirectory();
    exists = true;
}

void IEDirectory::remove() {
    std::filesystem::remove_all(path);
    exists = false;
}
