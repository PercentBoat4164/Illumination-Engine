#include "IEFileSystem.hpp"

IEFileSystem::IEFileSystem(const std::string &initialAssetDirectory) {
    baseDirectory = IEDirectory{initialAssetDirectory};
}

IEFileSystem::IEFileSystem(const IEDirectory &initialBaseDirectory) {
    baseDirectory = initialBaseDirectory;
}

template<typename... Args>
std::string IEFileSystem::composePath(IEPathName format, const Args &...args) {
    std::vector<std::string> arguments = {args...};
    std::string              pathName{};
    for (const std::string &argument : arguments) pathName += "/" + argument;
    if (format & IE_FILE_BIT) {
        size_t beginFileName = pathName.find_last_of('/');
        pathName             = pathName.substr(0, beginFileName) + '.' + pathName.substr(beginFileName + 1);
    }
    if (format & IE_HIDDEN_BIT) {
        size_t beginFileName = pathName.find_last_of('/');
        pathName             = pathName.substr(0, beginFileName + 1) + '.' + pathName.substr(beginFileName + 1);
    }
    return pathName.substr(1);
}