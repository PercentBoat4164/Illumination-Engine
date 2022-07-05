#include <iostream>
#include "Core/LogModule/IELogger.hpp"
#include "Core/FileSystemModule/IEFileSystem.hpp"

int main() {
    IEFileSystem fileSystem{"C:/Users/ethan/CLionProjects/Illumination-Engine/src"};
    //IEFile testFile("C:/Users/ethan/CLionProjects/Illumination-Engine/src/testFile.txt");

    fileSystem.addFile("newTestFile.txt");
    fileSystem.exportData("newTestFile.txt", "this was exported!");
    std::cout << fileSystem.importFile("newTestFile.txt");
    fileSystem.deleteFile("newTestFile.txt");
    return 0;
}