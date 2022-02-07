#include <iostream>
#include "Core/LogModule/IELogger.hpp"
#include "Core/FileSystemModule/IEFileSystem.hpp"

int main() {
    IEFileSystem fileSystem{"res/Assets"};
    IEFile* cubeFile = fileSystem.getAssetFile("cube");
    cubeFile->open();
    cubeFile->erase(cubeFile->length, 0);
    cubeFile->overwrite("Pork.", 0);
    cubeFile->insert("cupine", 3);
    cubeFile->erase(1);
    std::cout << cubeFile->read(cubeFile->length, 0);
}