#include <iostream>
#include "Core/LogModule/IELogger.hpp"
#include "Core/FileSystemModule/IEFileSystem.hpp"

int main() {
	IEFileSystem fileSystem{"C:/Users/ethan/CLionProjects/Illumination-Engine/src"};
	//IEFile testFile("C:/Users/ethan/CLionProjects/Illumination-Engine/src/testFile.txt");
	
	const aiScene *scene;
	/**@todo The IEFile should never be created by the user. This should only be accessible from files registered with the filesystem.*/
	IEFile file = IEFile{"res/Models/AncientStatue/ancientStatue.obj"};
	fileSystem.importer.import(&scene, file, 0);
	return 0;
}