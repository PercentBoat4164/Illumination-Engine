#include <iostream>
#include "Core/LogModule/IELogger.hpp"
#include "Core/FileSystemModule/IEFileSystem.hpp"

int main() {
	IEFileSystem fileSystem{"C:/Users/ethan/CLionProjects/Illumination-Engine/src"};
	//IEFile testFile("C:/Users/ethan/CLionProjects/Illumination-Engine/src/testFile.txt");
	
	fileSystem.addFile("f1/f2/f3/f4/f1/f6/file.obj");
	fileSystem.importFile("f1/f2/f3/f4/f1/f6/file.obj");
	return 0;
}