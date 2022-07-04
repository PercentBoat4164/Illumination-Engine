#include <iostream>
#include "Core/LogModule/IELogger.hpp"
#include "Core/FileSystemModule/IEImporter.hpp"

int main() {
    //IEFileSystem{"C:/Users/ethan/CLionProjects/Illumination-Engine/src"};
    IEFile testFile("C:/Users/ethan/CLionProjects/Illumination-Engine/src/testFile.txt");

    IEImporter fileImporter = IEImporter();
    std::cout << "Imported data: \n" << fileImporter.import(testFile, 0) << "\n";
    return 0;
}