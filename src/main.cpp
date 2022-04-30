#include <iostream>
#include "Core/LogModule/IELogger.hpp"
#include "Core/FileSystemModule/IETempFile.hpp"

int main() {
    IETempFile myFile("C:\\Users\\ethan\\CLionProjects\\Illumination-Engine\\src\\testFile.txt");
    std::cout << ("\n" + myFile.data);
}