#include <iostream>
#include "Core/LogModule/IELogger.hpp"
#include "Core/FileSystemModule/IETempFile.hpp"

int main() {
    IETempFile myFile(R"(C:\Users\ethan\CLionProjects\Illumination-Engine\src\testFile.txt)");
    std::cout << "\n" <<myFile.read(0, 50);
}