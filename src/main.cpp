#include <iostream>

#include <assimp/scene.h>

#include "Core/LogModule/IELogger.hpp"
#include "Core/FileSystemModule/IEFileSystem.hpp"

int main() {
	IEFileSystem fileSystem{"C:/Users/ethan/Documents"};

    std::string testString = "";

	/**@todo The IEFile should never be created by the user. This should only be accessible from files registered with the filesystem.*/
	fileSystem.addFile("testFile.txt");
    fileSystem.importFile(&testString, "testFile.txt", 0);
	std::cout << testString;
    return 0;
}